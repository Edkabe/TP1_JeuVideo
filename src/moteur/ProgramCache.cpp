#include "moteur/ProgramCache.h"
#include <SDL3/SDL.h>
#include <array>
#include <fstream>
#include <memory>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace moteur {
namespace {
constexpr std::uint64_t seed = 14695981039346656037ull;
constexpr std::uint32_t maxBinarySize = 64 * 1024 * 1024;
constexpr std::uint64_t headerSize = 36;
constexpr std::array<char,8> magic{'T','P','C','A','C','H','E','1'};
std::uint64_t hash(const void* bytes, size_t count, std::uint64_t start = seed) {
    const auto* data = static_cast<const unsigned char*>(bytes);
    for (size_t i = 0; i < count; ++i) { start ^= data[i]; start *= 1099511628211ull; }
    return start;
}
void writeNumber(std::ostream& out, std::uint64_t value, unsigned bytes) {
    for (unsigned i = 0; i < bytes; ++i) out.put(static_cast<char>((value >> (i*8)) & 255));
}
bool readNumber(std::istream& in, std::uint64_t& value, unsigned bytes) {
    value = 0;
    for (unsigned i = 0; i < bytes; ++i) {
        const int byte = in.get();
        if (byte == std::char_traits<char>::eof()) return false;
        value |= static_cast<std::uint64_t>(static_cast<unsigned char>(byte)) << (i*8);
    }
    return true;
}
std::uint64_t addNumber(std::uint64_t key, std::uint64_t number) {
    for (unsigned i = 0; i < 8; ++i) {
        const unsigned char byte = static_cast<unsigned char>(number >> (i*8));
        key = hash(&byte,1,key);
    }
    return key;
}
std::uint64_t addText(std::uint64_t key, const std::string& text) {
    return hash(text.data(),text.size(),addNumber(key,text.size()));
}
bool readCache(const std::filesystem::path& path, std::uint64_t key, std::uint32_t expectedFormat, Program::Binary& result) {
    std::error_code ec;
    const auto size = std::filesystem::file_size(path,ec);
    if (ec || size < headerSize || size > headerSize + maxBinarySize) return false;
    std::ifstream in(path,std::ios::binary);
    std::array<char,8> signature{}; in.read(signature.data(),signature.size());
    std::uint64_t version = 0, format = 0, storedKey = 0, length = 0, checksum = 0;
    if (!in || signature != magic || !readNumber(in,version,4) || version != 1 || !readNumber(in,format,4) ||
        !readNumber(in,storedKey,8) || storedKey != key || !readNumber(in,length,4) ||
        length == 0 || length > maxBinarySize || size != headerSize + length || !readNumber(in,checksum,8) ||
        (expectedFormat != 0 && format != expectedFormat)) return false;
    result.format = static_cast<GLenum>(format);
    result.data.resize(static_cast<size_t>(length));
    in.read(reinterpret_cast<char*>(result.data.data()),static_cast<std::streamsize>(length));
    return in && hash(result.data.data(),result.data.size()) == checksum;
}
bool saveCache(const std::filesystem::path& path, std::uint64_t key, const Program::Binary& binary) {
    if (binary.data.empty() || binary.data.size() > maxBinarySize) return false;
    std::error_code ec;
    if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path(),ec);
    if (ec) return false;
    // Fichier temporaire voisin, puis remplacement atomique du cache existant.
    auto temporary = path;
    temporary += "." + std::to_string(SDL_GetPerformanceCounter()) + ".tmp";
    bool written = false;
    {
        std::ofstream out(temporary,std::ios::binary | std::ios::trunc);
        if (out) {
            out.write(magic.data(),magic.size());
            writeNumber(out,1,4); writeNumber(out,binary.format,4); writeNumber(out,key,8);
            writeNumber(out,binary.data.size(),4); writeNumber(out,hash(binary.data.data(),binary.data.size()),8);
            out.write(reinterpret_cast<const char*>(binary.data.data()),static_cast<std::streamsize>(binary.data.size()));
            out.flush(); out.close(); written = static_cast<bool>(out);
        }
    }
    if (written) {
#ifdef _WIN32
        written = MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
        std::filesystem::rename(temporary,path,ec); written = !ec;
#endif
    }
    if (!written) std::filesystem::remove(temporary,ec);
    return written;
}
}
bool ProgramCache::load(Program& program, const std::vector<Source>& sources,
                        const std::filesystem::path& cacheFile, Settings& settings) {
    cached = false; error.clear(); warning.clear();
    if (!SDL_GL_GetCurrentContext()) { error = "Contexte OpenGL actif necessaire."; return false; }
    if (sources.empty()) { error = "Aucun fichier de nuanceur fourni."; return false; }
    std::vector<std::string> text;
    std::uint64_t key = seed;
    for (GLenum identity : {GL_VENDOR,GL_RENDERER,GL_VERSION}) {
        const auto* value = glGetString(identity);
        if (!value) { error = "Identite du pilote OpenGL indisponible."; return false; }
        key = addText(key,reinterpret_cast<const char*>(value));
    }
    for (const auto& source : sources) {
        std::error_code ec;
        const auto length = std::filesystem::file_size(source.file,ec);
        if (ec || length == 0 || length > 16*1024*1024) { error = "Source absente, vide ou trop grande : " + source.file.string(); return false; }
        std::ifstream in(source.file,std::ios::binary);
        std::string contents(static_cast<size_t>(length),'\0');
        in.read(contents.data(),static_cast<std::streamsize>(length));
        if (!in) { error = "Lecture du nuanceur impossible : " + source.file.string(); return false; }
        key = addText(addNumber(key,source.type),contents);
        text.push_back(std::move(contents));
    }
    const bool supported = Program::supportsBinary();
    Program::Binary binary;
    if (supported && readCache(cacheFile,key,settings.getProgramBinaryFormat(),binary) && program.loadBinary(binary)) {
        cached = true;
    } else {
        // Le candidat est compile et lie avant de remplacer le programme existant.
        std::vector<std::unique_ptr<Shader>> shaders;
        std::vector<const Shader*> pointers;
        for (size_t i = 0; i < sources.size(); ++i) {
            auto shader = std::make_unique<Shader>(sources[i].type);
            if (!shader->compile(text[i])) { error = sources[i].file.string() + ": " + shader->getError(); return false; }
            pointers.push_back(shader.get()); shaders.push_back(std::move(shader));
        }
        if (!program.link(pointers)) { error = program.getError(); return false; }
        if (!supported) { warning = "Pilote sans support binaire : programme compile depuis les sources."; return true; }
        if (!program.getBinary(binary)) { warning = program.getError(); return true; }
        if (!saveCache(cacheFile,key,binary)) warning = "Programme utilisable mais cache non sauvegarde.";
    }
    if (settings.getProgramBinaryFormat() != binary.format) {
        settings.setProgramBinaryFormat(binary.format);
        if (!settings.save()) {
            if (!warning.empty()) warning += " ";
            warning += "Format binaire non sauvegarde : " + settings.getError();
        }
    }
    return true;
}
}

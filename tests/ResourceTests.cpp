#include "moteur/Engine.h"
#include "moteur/Texture.h"
#include "moteur/Font.h"
#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <cstdlib>
#include <string_view>

namespace {
int checks = 0;
void check(bool value, const char* message) { ++checks; if (!value) throw std::runtime_error(message); }
class Fake : public moteur::Resource {
public:
    explicit Fake(int& destroyed) : destroyed(destroyed) {}
    ~Fake() override { ++destroyed; }
    bool load(const std::string&) override { loaded = true; return true; }
    void release() override { loaded = false; }
private:
    int& destroyed;
};
class Other : public Fake { public: using Fake::Fake; };
void managerTests() {
    int destroyed = 0;
    moteur::ResourceManager manager;
    check(manager.size() == 0, "Initialement vide");
    check(!manager.add("null", nullptr), "Pointeur nul refuse");
    auto resource = std::make_shared<Fake>(destroyed);
    check(!manager.add("", resource), "Nom vide refuse");
    check(manager.add("fond", resource), "Ajout");
    check(resource->getTitle()=="fond", "Premier nom conserve comme titre");
    check(manager.contains("fond") && manager.size() == 1, "Presence");
    check(manager.get<Fake>("fond") == resource, "Meme objet partage");
    check(manager.get<>("fond") == resource, "Acces polymorphe");
    check(!manager.get<Other>("fond") && !manager.get<>("absent"), "Types et noms absents");
    auto duplicate = std::make_shared<Other>(destroyed);
    check(!manager.add("fond", duplicate) && manager.get<Fake>("fond") == resource, "Doublon conserve l'original");
    check(duplicate->getTitle().empty(), "Doublon refuse ne change pas le titre");
    duplicate.reset();
    check(destroyed == 1, "Doublon non conserve");
    std::weak_ptr<Fake> weak = resource;
    auto client = manager.get<Fake>("fond"); resource.reset();
    check(manager.remove("fond") && !manager.remove("fond"), "Retrait et absence");
    check(!weak.expired() && destroyed == 1, "Client conserve la ressource retiree");
    client.reset();
    check(weak.expired() && destroyed == 2, "Destruction a la derniere reference");
    auto second = std::make_shared<Fake>(destroyed);
    manager.add("a", second); manager.add("b", second); manager.clear();
    check(manager.size() == 0 && destroyed == 2, "Clear respecte les references clientes");
    second.reset(); check(destroyed == 3, "Destruction unique");
    auto titled=std::make_shared<moteur::Texture>("fond_titre");
    check(manager.add(titled) && manager.get<moteur::Texture>("fond_titre")==titled,"Ajout par titre de la ressource");
    check(!manager.add(titled) && !manager.add(std::shared_ptr<moteur::Resource>{}),"Doublon titre et pointeur nul refuses");
    auto untitled=std::make_shared<moteur::Texture>();
    check(!manager.add(untitled),"Ajout sans nom ni titre refuse");
    manager.clear();
    moteur::Texture noContext;
    check(!noContext.load("absent.png") && !noContext.isLoaded(), "Contexte requis");
    moteur::Font invalid(-1);
    check(!invalid.load("absent.ttf") && !invalid.getError().empty(),"Taille de police invalide refusee");

}
void graphicsTests() {
    auto& engine = moteur::Engine::getInstance();
    check(engine.initialize("Tests ressources", 64, 64, true), "Initialisation graphique");
    const auto path = std::filesystem::path(SDL_GetBasePath()) / "assets/tests/pixels.png";
    {
        moteur::Texture texture;
        GLuint sentinel = 0; glGenTextures(1, &sentinel); glBindTexture(GL_TEXTURE_2D, sentinel);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 8); glPixelStorei(GL_UNPACK_ROW_LENGTH, 12);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 2); glPixelStorei(GL_UNPACK_SKIP_PIXELS, 3);
        check(texture.load(path.string()), "Chargement PNG reel");
        check(texture.isLoaded() && texture.getWidth()==2 && texture.getHeight()==2, "Dimensions");
        GLint value=0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D,&value); check(value==static_cast<GLint>(sentinel), "Binding restaure");
        glGetIntegerv(GL_UNPACK_ALIGNMENT,&value); check(value==8,"Alignement restaure");
        glGetIntegerv(GL_UNPACK_ROW_LENGTH,&value); check(value==12,"Longueur de ligne restauree");
        glGetIntegerv(GL_UNPACK_SKIP_ROWS,&value); check(value==2,"Decalage lignes restaure");
        glGetIntegerv(GL_UNPACK_SKIP_PIXELS,&value); check(value==3,"Decalage pixels restaure");
        const GLuint id=static_cast<GLuint>(texture); texture.bind();
        unsigned char pixels[16]{};
        glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
        const unsigned char expected[16]={255,0,0,255,0,255,0,255,0,0,255,255,255,255,255,255};
        bool correct=true; for(int i=0;i<16;++i) correct=correct && pixels[i]==expected[i];
        check(correct,"Pixels RGBA exacts");
        check(!texture.load("absent.png") && static_cast<GLuint>(texture)==id && texture.isLoaded(), "Echec conserve la texture");
        check(texture.load(path.string()) && !glIsTexture(id), "Rechargement libere l'ancienne texture");
        texture.release(); texture.release();
        check(!texture.isLoaded() && static_cast<GLuint>(texture)==0 && texture.getWidth()==0, "Release idempotent");
        glDeleteTextures(1,&sentinel); check(glGetError()==GL_NO_ERROR,"Pas d'erreur GL");

    }
    const char* windows=SDL_getenv("WINDIR");
    if (!windows) windows=SDL_getenv("windir");
    if (!windows) windows=SDL_getenv("SystemRoot");
    check(windows!=nullptr,"Chemin de police systeme");
    const auto fontPath=std::filesystem::path(windows)/"Fonts/arial.ttf";
    const int initCount=TTF_WasInit();
    {
        auto font=std::make_shared<moteur::Font>(24.0f,"titre_menu");
        check(font->load(fontPath.string()),"Chargement d'une vraie police");
        check(font->getHeight()>0 && font->getPointSize()==24 && static_cast<TTF_Font*>(*font),"Utilitaires de Font");
        check(engine.getResourceManager().add(font),"Police ajoutee par titre");
        check(engine.getResourceManager().get<moteur::Font>("titre_menu")==font,"Police partagee par titre");
        auto* old=static_cast<TTF_Font*>(*font);
        check(!font->load("absent.ttf") && static_cast<TTF_Font*>(*font)==old && font->isLoaded(),"Echec conserve la police");
        moteur::Font other(16);
        check(other.load(fontPath.string()),"Deux polices simultanees");
        check(font->load(fontPath.string()),"Rechargement de police");
        check(engine.getResourceManager().remove("titre_menu"),"Retrait d'une police geree");
        font->release(); font->release();
        check(!font->isLoaded() && !static_cast<TTF_Font*>(*font) && font->getHeight()==0,"Liberation idempotente de Font");
        check(other.getHeight()>0,"L'autre police reste valide");
    }
    check(TTF_WasInit()==initCount,"Initialisations TTF equilibrees");
    std::weak_ptr<moteur::Texture> weak;
    {
        auto managed=std::make_shared<moteur::Texture>(); check(managed->load(path.string()),"Texture geree");
        weak=managed; check(engine.getResourceManager().add("fond",managed),"Gestionnaire dans Engine");
    }
    check(engine.shutdown(),"Fermeture du moteur"); check(weak.expired(),"Gestionnaire vide avant fermeture du contexte");
}
}
int main(int argc,char** argv) {
    try {
        managerTests();
        if(argc>1 && std::string_view(argv[1])=="--graphics") graphicsTests();
        std::cout<<checks<<" verifications reussies.\n"; return 0;
    } catch(const std::exception& error) { std::cerr<<"ECHEC : "<<error.what()<<'\n'; return 1; }
}

#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "moteur/Bibliotheques.h"
#include "moteur/Engine.h"
#include "moteur/VertexShader.h"
#include "moteur/Program.h"
#include "moteur/VAO.h"
#include "moteur/VBO.h"
#include <array>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {
// Vérifie le transfert et la libération sous un contexte OpenGL actif.
bool verifyBufferLifetime() {
  static_assert(!std::is_copy_constructible_v<moteur::VBO>);
  static_assert(!std::is_copy_assignable_v<moteur::VBO>);
  static_assert(!std::is_copy_constructible_v<moteur::VAO>);
  static_assert(!std::is_copy_assignable_v<moteur::VAO>);

  moteur::VBO source;
  if (!source.create()) return false;
  const GLuint sourceId = source.getId();
  source.bind();
  const bool sourceIsBuffer = glIsBuffer(sourceId) == GL_TRUE;
  moteur::VBO::unbind();

  moteur::VBO moved(std::move(source));
  bool valid = sourceIsBuffer && source.getId() == 0 && moved.getId() == sourceId;

  moteur::VBO destination;
  valid = valid && destination.create();
  const GLuint replacedId = destination.getId();
  destination.bind();
  const bool destinationIsBuffer = glIsBuffer(replacedId) == GL_TRUE;
  moteur::VBO::unbind();
  destination = std::move(moved);
  valid = valid && destinationIsBuffer && moved.getId() == 0 &&
      destination.getId() == sourceId && glIsBuffer(replacedId) == GL_FALSE;
  destination.release();
  destination.release();
  valid = valid && destination.getId() == 0 && glIsBuffer(sourceId) == GL_FALSE;

  moteur::VAO vaoSource;
  valid = valid && vaoSource.create();
  const GLuint vaoId = vaoSource.getId();
  vaoSource.bind();
  const bool sourceIsVertexArray = glIsVertexArray(vaoId) == GL_TRUE;
  moteur::VAO::unbind();
  moteur::VAO vaoMoved(std::move(vaoSource));
  valid = valid && sourceIsVertexArray && vaoSource.getId() == 0 && vaoMoved.getId() == vaoId;
  moteur::VAO vaoDestination;
  valid = valid && vaoDestination.create();
  const GLuint replacedVaoId = vaoDestination.getId();
  vaoDestination.bind();
  const bool destinationIsVertexArray = glIsVertexArray(replacedVaoId) == GL_TRUE;
  moteur::VAO::unbind();
  vaoDestination = std::move(vaoMoved);
  valid = valid && destinationIsVertexArray && vaoMoved.getId() == 0 &&
      vaoDestination.getId() == vaoId && glIsVertexArray(replacedVaoId) == GL_FALSE;
  vaoDestination.release();
  vaoDestination.release();
  valid = valid && vaoDestination.getId() == 0 && glIsVertexArray(vaoId) == GL_FALSE;
  return valid && glGetError() == GL_NO_ERROR;
}
}

namespace {
bool demonstrate(moteur::Engine& engine, bool smoke) {
  // Ces objets sont détruits avant Engine::shutdown(), contexte encore actif.
  moteur::VertexShader vertex;
  moteur::Shader fragment(GL_FRAGMENT_SHADER);
  moteur::Program program;
  moteur::VAO vao;
  moteur::VBO vbo;
  const char* basePath = SDL_GetBasePath();
  if (!basePath) { std::cerr << SDL_GetError() << '\n'; return false; }
  const std::filesystem::path shaders = std::filesystem::path(basePath) / "assets" / "shaders";
  if (!vertex.loadShader((shaders / "triangle.vert").string()) ||
      !fragment.loadShader((shaders / "triangle.frag").string())) {
    std::cerr << vertex.getError() << fragment.getError() << '\n'; return false;
  }
  if (!program.link({&vertex, &fragment}) || !vao.create() || !vbo.create()) {
    std::cerr << program.getError() << '\n'; return false;
  }
  // Les trois sommets sont stockés dans le VBO; le VAO mémorise leur attribut.
  constexpr std::array<GLfloat, 6> triangle{
      -0.65f, -0.55f, 0.65f, -0.55f, 0.0f, 0.65f};
  vbo.setData(triangle.data(), static_cast<GLsizeiptr>(sizeof(triangle)), GL_STATIC_DRAW);
  vao.setAttribute(0, 2, GL_FLOAT, GL_FALSE,
                   static_cast<GLsizei>(2 * sizeof(GLfloat)), 0);
  moteur::VAO::unbind();
  moteur::VBO::unbind();
  bool valid = true;
  if (smoke) {
    GLint capturedBuffer = 0;
    vao.bind();
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &capturedBuffer);
    moteur::VAO::unbind();
    valid = capturedBuffer == static_cast<GLint>(vbo.getId()) && verifyBufferLifetime();
    if (!valid) { std::cerr << "Echec des controles VBO/VAO.\n"; return false; }
    const GLuint previousShader = vertex.getId();
    valid = !vertex.compile("#version 330 core\nceci est invalide") &&
        !vertex.getError().empty() && vertex.getId() == previousShader;
    valid = valid && !vertex.loadShader((shaders / "absent.vert").string()) && vertex.getId() == previousShader;
    const GLuint previousProgram = program.getId();
    valid = valid && !program.link({}) && program.getId() == previousProgram;
    moteur::Shader invalid(GL_FRAGMENT_SHADER);
    valid = valid && !program.link({&vertex, &invalid}) && program.getId() == previousProgram;
    // Deux sorties de même nom/type incompatibles : la liaison doit échouer.
    moteur::VertexShader otherVertex;
    moteur::Shader otherFragment(GL_FRAGMENT_SHADER);
    valid = valid && otherVertex.compile("#version 330 core\nout vec3 value; void main(){value=vec3(1); gl_Position=vec4(0,0,0,1);}") &&
        otherFragment.compile("#version 330 core\nin vec4 value; out vec4 color; void main(){color=value;}") &&
        !program.link({&otherVertex, &otherFragment}) && !program.getError().empty() && program.getId() == previousProgram;
    valid = valid && !engine.initialize() && !engine.getWindow().initialize("autre", 10, 10);
    if (!valid) { std::cerr << "Echec des controles d'erreur.\n"; return false; }
  }
  int frames = 0;
  const bool ran = engine.run([&](float dt) {
    if (dt < 0) valid = false;
    if (smoke && frames == 0) {
      if (engine.run([](float){}, []{}) || engine.shutdown()) valid = false;
    }
    if (smoke && frames >= 3) engine.stop();
  }, [&] {
    int width = 0, height = 0;
    if (!engine.getWindow().getPixelSize(width, height)) { valid = false; engine.stop(); return; }
    glViewport(0, 0, width, height);
    glClearColor(0.08f, 0.12f, 0.20f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    program.use(); vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    if (smoke) {
      unsigned char pixel[4]{};
      glReadPixels(width / 2, height / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
      if (pixel[0] < 240 || pixel[1] < 120 || pixel[1] > 160 || pixel[2] > 60) valid = false;
      if (glGetError() != GL_NO_ERROR) valid = false;
    }
    moteur::VAO::unbind(); moteur::Program::unuse();
    ++frames;
  }, [&](const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) engine.stop();
  });
  if (!ran) std::cerr << engine.getError() << '\n';
  return ran && valid && (!smoke || frames == 3);
}
}

/// @brief Fonction principale
/// @param argc Nombre d'arguments
/// @param argv Arguments
/// @return Code de fin de programme
int main(int argc, char* argv[]) {
  const std::string_view mode = argc > 1 ? argv[1] : "";
  if (mode == "--check") return moteur::verifierBibliotheques() ? 0 : 1;
  const bool smoke = mode == "--smoke";
  auto& engine = moteur::Engine::getInstance();
  if (!engine.initialize("TP1 - Moteur : triangle", 960, 540, smoke)) {
    std::cerr << engine.getError() << '\n'; return 1;
  }
  const bool success = demonstrate(engine, smoke);
  engine.shutdown();
  if (smoke && success) std::cout << "Test graphique reussi : rendu, erreurs et boucle verifies.\n";
  return success ? 0 : 1;
}

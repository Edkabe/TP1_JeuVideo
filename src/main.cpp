#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "moteur/Bibliotheques.h"
#include "moteur/CheckBox.h"
#include "moteur/Engine.h"
#include "moteur/VertexShader.h"
#include "moteur/Program.h"
#include "moteur/VAO.h"
#include <filesystem>
#include <iostream>
#include <string_view>

namespace {
bool demonstrate(moteur::Engine& engine, bool smoke) {
  // Ces objets sont détruits avant Engine::shutdown(), contexte encore actif.
  moteur::VertexShader vertex;
  moteur::Shader fragment(GL_FRAGMENT_SHADER);
  moteur::Program program;
  moteur::VAO vao;
  moteur::CheckBox checkBox;
  const char* basePath = SDL_GetBasePath();
  if (!basePath) { std::cerr << SDL_GetError() << '\n'; return false; }
  const std::filesystem::path shaders = std::filesystem::path(basePath) / "assets" / "shaders";
  if (!vertex.loadShader((shaders / "triangle.vert").string()) ||
      !fragment.loadShader((shaders / "triangle.frag").string())) {
    std::cerr << vertex.getError() << fragment.getError() << '\n'; return false;
  }
  if (!program.link({&vertex, &fragment}) || !vao.create()) {
    std::cerr << program.getError() << '\n'; return false;
  }
  checkBox.setPosition(24.0f, 24.0f);
  checkBox.setSize(36.0f);
  checkBox.setBorderColor({0.08f, 0.10f, 0.14f, 1.0f});
  checkBox.setBackgroundColor({0.95f, 0.95f, 0.95f, 1.0f});
  checkBox.setCheckColor({0.10f, 0.70f, 0.28f, 1.0f});
  if (!checkBox.initialize()) {
    std::cerr << checkBox.getError() << '\n'; return false;
  }
  bool valid = true;
  if (smoke) {
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
    int windowWidth = 0, windowHeight = 0;
    if (SDL_GetWindowSize(engine.getWindow().getSDLWindow(), &windowWidth, &windowHeight)) {
      checkBox.render(static_cast<float>(windowWidth), static_cast<float>(windowHeight));
    }
    ++frames;
  }, [&](const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) engine.stop();
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event.button.windowID == engine.getWindow().getId()) checkBox.handleEvent(event);
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

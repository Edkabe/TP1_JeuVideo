#include <glad/gl.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "moteur/Bibliotheques.h"
#include "moteur/CheckBox.h"
#include "moteur/Engine.h"
#include "moteur/EventManager.h"
#include "moteur/Font.h"
#include "moteur/Label.h"
#include "moteur/Slider.h"
#include "moteur/VertexShader.h"
#include "moteur/Program.h"
#include "moteur/VAO.h"
#include "moteur/VBO.h"
#include <array>
#include <filesystem>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
std::string readWindowsDirectory() {
  char* value = nullptr;
  size_t size = 0;
  if (_dupenv_s(&value, &size, "WINDIR") != 0 || !value) return {};
  std::string result(value);
  std::free(value);
  return result;
}

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
bool verifyLabel() {
  auto& engine = moteur::Engine::getInstance();
  if (!engine.initialize("TP1 - verification etiquette", 320, 160, true)) {
    std::cerr << engine.getError() << '\n'; return false;
  }
  bool valid = true;
  const std::string windowsDirectory = readWindowsDirectory();
  const std::filesystem::path fontPath = windowsDirectory.empty()
      ? std::filesystem::path{} : std::filesystem::path(windowsDirectory) / "Fonts" / "arial.ttf";
  auto font = std::make_shared<moteur::Font>(28.0f, "Police de verification");
  valid = valid && !fontPath.empty() && font->load(fontPath.string());
  if (!valid) {
    std::cerr << (font->getError().empty() ? "Police de verification introuvable.\n" : font->getError() + "\n");
    engine.shutdown(); return false;
  }
  valid = engine.getResourceManager().add("police-verification", font);
  std::weak_ptr<moteur::Font> weakFont = font;
  font.reset();

  const auto countRedPixels = [] {
    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    std::vector<unsigned char> pixels(static_cast<size_t>(viewport[2] * viewport[3] * 4));
    glReadPixels(0, 0, viewport[2], viewport[3], GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    size_t count = 0;
    for (size_t i = 0; i + 3 < pixels.size(); i += 4)
      if (pixels[i] > 80 && pixels[i + 1] < 40 && pixels[i + 2] < 40) ++count;
    return count;
  };

  size_t widePixels = 0, narrowPixels = 0;
  {
    auto sharedFont = engine.getResourceManager().get<moteur::Font>("police-verification");
    moteur::Label label(sharedFont, "MMMM");
    sharedFont.reset();
    label.setPosition({12.0f, 12.0f});
    label.setColor({1.0f, 0.0f, 0.0f, 1.0f});
    const moteur::Vector2 wideSize = label.getDimensions();
    valid = valid && wideSize.x > 0 && wideSize.y > 0;

    int width = 0, height = 0;
    valid = valid && engine.getWindow().getPixelSize(width, height);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    valid = valid && label.render();
    glFinish();
    widePixels = countRedPixels();
    valid = valid && widePixels > 0;

    valid = valid && label.setText("i");
    const moteur::Vector2 narrowSize = label.getDimensions();
    valid = valid && narrowSize.x > 0 && narrowSize.y > 0 && wideSize.x > narrowSize.x;
    glClear(GL_COLOR_BUFFER_BIT);
    valid = valid && label.render();
    glFinish();
    narrowPixels = countRedPixels();
    valid = valid && narrowPixels > 0 && narrowPixels != widePixels;

    // Le contrôle conserve la ressource tant qu'il l'utilise, même après retrait du gestionnaire.
    valid = valid && engine.getResourceManager().remove("police-verification") && !weakFont.expired();
  }
  // L'étiquette et ses objets GL sont détruits avant le contexte de la fenêtre.
  valid = valid && weakFont.expired() && glGetError() == GL_NO_ERROR;
  valid = engine.shutdown() && valid;
  if (!valid) std::cerr << "Echec d'une verification du controle Etiquette.\n";
  return valid;
}

bool demonstrate(moteur::Engine& engine, bool smoke) {
  // Ces objets sont détruits avant Engine::shutdown(), contexte encore actif.
  moteur::VertexShader vertex;
  moteur::Shader fragment(GL_FRAGMENT_SHADER);
  moteur::Program program;
  moteur::VAO vao;
  moteur::CheckBox checkBox;
  moteur::VBO vbo;
  auto labelFont = std::make_shared<moteur::Font>(28.0f, "Police de demonstration");
  bool labelAvailable = false;
  if (const std::string windowsDirectory = readWindowsDirectory(); !windowsDirectory.empty()) {
    const auto fontPath = std::filesystem::path(windowsDirectory) / "Fonts" / "arial.ttf";
    labelAvailable = labelFont->load(fontPath.string()) &&
        engine.getResourceManager().add("police-demonstration", labelFont);
  }
  moteur::Label label(labelAvailable
      ? engine.getResourceManager().get<moteur::Font>("police-demonstration")
      : std::shared_ptr<moteur::Font>{}, "TP1 - Controle Etiquette");
  label.setPosition({18.0f, 18.0f});
  label.setColor({1.0f, 1.0f, 1.0f, 1.0f});
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
  checkBox.setPosition(24.0f, 24.0f);
  checkBox.setSize(36.0f);
  checkBox.setBorderColor({0.08f, 0.10f, 0.14f, 1.0f});
  checkBox.setBackgroundColor({0.95f, 0.95f, 0.95f, 1.0f});
  checkBox.setCheckColor({0.10f, 0.70f, 0.28f, 1.0f});
  if (!checkBox.initialize()) {
    std::cerr << checkBox.getError() << '\n'; return false;
  }
  // Le pointeur partagé garde le contrôle en vie pendant ses abonnements.
  auto slider = std::make_shared<moteur::Slider>();
  slider->setPosition({24.0f, 82.0f});
  slider->setDimensions({260.0f, 28.0f});
  if (!slider->setRange(0.0f, 100.0f, 5.0f) || !slider->setValue(50.0f)) {
    std::cerr << slider->getError() << '\n'; return false;
  }
  slider->setBorderColor({0.08f, 0.10f, 0.14f, 1.0f});
  slider->setBackgroundColor({0.82f, 0.85f, 0.90f, 1.0f});
  slider->setProgressColor({0.10f, 0.48f, 0.78f, 1.0f});
  slider->setHoverColor({0.20f, 0.72f, 0.96f, 1.0f});
  moteur::EventManager sliderEvents;
  for (Uint32 type : {SDL_EVENT_MOUSE_MOTION, SDL_EVENT_MOUSE_BUTTON_DOWN,
                      SDL_EVENT_MOUSE_BUTTON_UP, SDL_EVENT_WINDOW_FOCUS_LOST,
                      SDL_EVENT_WINDOW_MOUSE_LEAVE}) {
    if (!sliderEvents.addObserver(slider, type)) {
      std::cerr << "Inscription du glisseur aux evenements impossible.\n";
      return false;
    }
  }
  moteur::Label sliderValueLabel(labelAvailable
      ? engine.getResourceManager().get<moteur::Font>("police-demonstration")
      : std::shared_ptr<moteur::Font>{}, "Valeur du glisseur : 50");
  sliderValueLabel.setPosition({24.0f, 118.0f});
  sliderValueLabel.setColor({1.0f, 1.0f, 1.0f, 1.0f});
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
    if (labelAvailable && !label.render()) {
      std::cerr << label.getError() << '\n'; valid = false; engine.stop(); return;
    }
    if (!slider->render()) {
      std::cerr << slider->getError() << '\n'; valid = false; engine.stop(); return;
    }
    if (labelAvailable && !sliderValueLabel.render()) {
      std::cerr << sliderValueLabel.getError() << '\n'; valid = false; engine.stop(); return;
    }
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
    const moteur::Event wrappedEvent(event);
    sliderEvents.notifyObservers(wrappedEvent);
    sliderValueLabel.setText("Valeur du glisseur : " +
        std::to_string(static_cast<int>(slider->getValue())));
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
  if (mode == "--label-check") return verifyLabel() ? 0 : 1;
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

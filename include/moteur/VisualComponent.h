#pragma once
namespace moteur {
/// Contrat de rendu des controles. Liberer avant de fermer le contexte OpenGL.
class VisualComponent {
public:
    virtual ~VisualComponent() = default;
    virtual bool render() = 0;
    virtual void release() = 0;
};
}

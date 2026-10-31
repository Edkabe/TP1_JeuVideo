#pragma once
namespace moteur {
/// Contrat de rendu des controles. Un rendu qui echoue retourne false et
/// renseigne son diagnostic. Un controle restaure les etats OpenGL globaux
/// qu'il modifie (programme, VAO, melange et textures selon le controle).
/// Liberer avant de fermer le contexte OpenGL.
class VisualComponent {
public:
    virtual ~VisualComponent() = default;
    virtual bool render() = 0;
    virtual void release() = 0;
};
}

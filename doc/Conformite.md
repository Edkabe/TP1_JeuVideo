# Correspondance avec le mandat

Le moteur mono fenêtre est compilé en bibliothèque statique avec le script Windows, sans CMake. Les fonctionnalités des scènes restent inchangées.

## Abstractions

- `VisualComponent` définit un destructeur virtuel, `bool render()` et `void release()`. Les sept contrôles le réalisent. `CheckBox::render(width, height)` reste disponible pour les anciens appels; `render()` utilise le viewport courant.
- `Buffer` définit les opérations d'un tampon GPU. `VBO` réalise ce contrat et conserve ses déplacements et sa propriété exclusive du tampon.
- Les objets graphiques doivent être détruits ou libérés avant la fermeture du contexte. `release()` permet une nouvelle création au rendu suivant.

## Matrice et programme

`Matrix4` fournit une conversion explicite en `const float*`, sans copie. Le pointeur reste valide pendant la vie de la matrice. `Program::getUniformLocation` et `getAttributeLocation` retournent `-1` pour un nom absent, nul ou un programme vide. Un contexte courant est requis pour interroger un programme existant.

## Textures et polices

`Texture::setFiltering` accepte `GL_LINEAR` et `GL_NEAREST` pour les filtres de minification et magnification. Les filtres à mipmaps ne sont pas proposés car le chargement ne génère pas de mipmaps. `setWrapping` accepte `GL_REPEAT`, `GL_MIRRORED_REPEAT` et `GL_CLAMP_TO_EDGE`. Ces réglages sont conservés au chargement et rechargement, et peuvent être modifiés sur une texture existante. La liaison de texture précédente est restaurée. Les paramètres invalides sont refusés sans modifier les réglages.

`Font::createTextTexture` génère une texture RGBA de texte UTF-8 blanc, que `Label` colore au rendu. Le contexte doit être courant, la police chargée, et l'identifiant transmis doit être nul ou appartenir à l'appelant. Le remplacement est effectué après succès; en cas d'échec la texture précédente reste intacte. L'appelant possède la texture retournée et la libère. Une chaîne vide libère la texture et remet ses dimensions à zéro. `Label` délègue cette opération à `Font`.

## Programmes binaires

`CheckBox` et `Slider` passent par `ProgramCache`, comme les autres contrôles. Les nuanceurs de la case sont dans `assets/shaders/checkbox.vert` et `checkbox.frag`. Le glisseur partage les nuanceurs et le cache du bouton. Un binaire valide est chargé en priorité; sinon les sources sont compilées et liées, puis le binaire est sauvegardé si le pilote le permet. Le format est conservé dans `Settings`. Sur un pilote sans support, le rendu reste disponible par compilation des sources.

## Portée de la vérification

Les tests fonctionnels locaux couvrent la génération du texte, les paramètres de texture, les contrats polymorphes et la recréation des contrôles. Ils restent locaux. Les critères individuels de contribution et de comportement du barème nécessitent une évaluation humaine.

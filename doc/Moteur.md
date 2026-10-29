# Moteur et classes adaptées de l'atelier 2

Les en-têtes sont dans `include/moteur/`, les implémentations dans
`src/moteur/`. Ils sont regroupés dans `build/lib/moteur.lib`.

| Classe | Responsabilité |
| --- | --- |
| Engine | Singleton, fenêtre unique, initialisation SDL, boucle avec deltaTime et arrêt. |
| Window | Fenêtre SDL3 redimensionnable (minimum 320 × 540), contexte OpenGL 3.3 Core, chargement GLAD et dimensions en pixels. |
| Shader | Compilation depuis un fichier ou une chaîne et journal d'erreur. |
| VertexShader | Spécialisation pour GL_VERTEX_SHADER. |
| Program | Liaison des nuanceurs et activation du programme. |
| VBO | Propriété du tampon de sommets GPU, envoi et libération de ses données. |
| VAO | Format des attributs de sommets et capture du VBO lié lors de leur configuration. |

Les propriétaires d'identifiants OpenGL ne sont pas copiables. Détruire leurs
instances **avant `Engine::shutdown()`**, sur le thread du contexte actif.
Les scènes sont vidées avant l'arrêt du moteur afin de libérer leurs contrôles
graphiques pendant que le contexte existe encore.

Après compilation depuis VS Code (**Ctrl+Maj+B**), lancer `build/program.exe`
pour ouvrir le menu principal. `--triangle` lance la démonstration OpenGL
historique; Échap ou la croix arrête sa boucle. Les sources GLSL sont copiées
depuis `assets/shaders` à côté de l'application.

`--check` vérifie la liaison des dépendances. `--smoke` crée une fenêtre
masquée et vérifie les erreurs de nuanceur/liaison, la propriété et le
transfert des VBO/VAO, la capture du VBO par le VAO, la protection contre une
boucle réentrante et le rendu du triangle alimenté par un VBO.

Le moteur inclut les paramètres persistants, les ressources titrées, les
scènes et leurs événements, les contrôles visuels ainsi que le cache binaire
des programmes. Voir la [documentation des classes](Classes.md), le
[fonctionnement du menu](MenuPrincipal.md), la [validation d'intégration](Integration.md)
et la [compilation Windows](Bibliotheques.md).

## Diagramme de classes

Le [diagramme à jour](Classes.md) présente les classes implémentées et leurs
relations. L'ancien diagramme UMLet est conservé sous le nom
[Classes-initiale.uxf](Classes-initiale.uxf).

# Premières classes de la bibliothèque moteur

Adaptation des classes de l'atelier 2 fourni en ZIP, sans modifier l'archive.
Les sources se trouvent dans `src/moteur/` et leurs interfaces publiques dans
`include/moteur/`. Elles sont compilées dans `moteur.lib`.

| Classe | Adaptation |
| --- | --- |
| Engine | Singleton, une seule Window, initialisation SDL, boucle avec deltaTime et arrêt; refuse une boucle déjà active. |
| Window | Fenêtre SDL3, contexte OpenGL 3.3 Core, chargement GLAD, dimensions en pixels et destruction du contexte avant la fenêtre. |
| Shader | Compilation depuis un fichier ou une chaîne; erreurs détaillées et conservation de l'ancien objet si le remplacement échoue. |
| VertexShader | Spécialisation de Shader pour GL_VERTEX_SHADER. |
| Program | Liaison de nuanceurs compilés, activation et libération; séparé de Shader. |
| VAO | Création, activation et libération d'un objet de sommets. |

Les objets propriétaires ne sont pas copiables. Leurs destructeurs libèrent
les identifiants OpenGL. **Ils doivent être détruits avant `Engine::shutdown()`**,
sur le thread du contexte actif. L'application de démonstration respecte cet
ordre. Les callbacks de `run` sont exécutés sur ce même thread; ils permettront
de brancher le gestionnaire de scènes dans une étape ultérieure.

```powershell
cmake --preset windows
cmake --build --preset windows-debug
ctest --preset windows-debug
./out/windows/Debug/program.exe
./out/windows/Debug/program.exe --smoke
```

Le lancement normal affiche un triangle orange et se termine avec Échap ou
la croix. Les deux sources GLSL sont dans `assets/shaders/`; le chemin est
configuré par CMake pour cette démonstration locale.

`--check` conserve le test de liaison des dépendances, sans fenêtre.
`--smoke` crée une fenêtre masquée, vérifie une compilation/liaison invalide,
le refus d'une deuxième initialisation et d'une boucle réentrante, dessine
trois images et lit un pixel du triangle pour vérifier le rendu réel.

Le chargement de paramètres, les scènes, les gestionnaires, les contrôles
et les programmes binaires ne font pas partie de cette première étape.

Validation effectuée sous Windows/MSVC x64 : compilation sans avertissement,
test CTest des dépendances réussi et test graphique `--smoke` réussi avec
lecture du pixel, erreurs de nuanceurs/liaison et contrôles de réentrance.

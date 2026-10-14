# Bibliothèque du moteur et compilation incrémentale

Le moteur est une bibliothèque statique (`moteur.lib` sous Windows,
`libmoteur.a` sous macOS). L'application `program` la lie à la compilation.
Les futurs fichiers du moteur vont dans `src/moteur/`, leurs en-têtes publics
dans `include/moteur/`. Ajouter chaque nouveau `.cpp` à la cible `moteur`
dans `CMakeLists.txt`. Le `main.cpp` reste dans la cible de l'application.

SDL3, SDL3_image, SDL3_ttf et GLAD sont déjà compilées dans `build/lib/`.
GLM 1.0.1 est fourni dans `external/glm/`, avec sa licence : cette bibliothèque
d'en-têtes n'a pas de binaire séparé.

## Utilisation depuis VS Code

Installer CMake 3.24 ou plus récent, les extensions recommandées et le
compilateur C++. Sous Windows, ce preset utilise les Build Tools
**Visual Studio 2026 x64**, comme ceux installés sur le poste d'Edkabe.
L'éditeur reste VS Code. Le générateur Visual Studio 2026 nécessite CMake 4.2
ou plus récent. Pour Visual Studio 2022, utiliser CMake 3.24 ou plus récent et
le générateur `Visual Studio 17 2022` dans un `CMakeUserPresets.json` local.
Ne pas partager un dossier de sortie entre deux générateurs différents.

Avec CMake dans le PATH, **Ctrl+Maj+B** configure puis compile ce qui a changé.
**F5** compile de la même façon puis lance l'application.
La configuration CMake répétée ne force pas la recompilation des sources.

Sur le poste d'Edkabe, CMake est déjà livré avec Visual Studio. Si `cmake`
n'est pas reconnu dans le terminal VS Code, ajouter ce dossier au PATH du
terminal (ou lancer VS Code depuis un terminal où CMake est accessible) :
`C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin`.

Exemple de `CMakeUserPresets.json` pour un poste Visual Studio 2022, en
conservant les noms utilisés par les tâches VS Code :

```json
{
  "version": 5,
  "configurePresets": [
    { "name": "windows", "generator": "Visual Studio 17 2022", "architecture": "x64", "binaryDir": "${sourceDir}/out/windows" }
  ]
}
```

Utiliser cet exemple avant la première configuration sur ce poste.

```powershell
cmake --preset windows
cmake --build --preset windows-debug
ctest --preset windows-debug
```

Sous macOS : installer CMake et les outils de compilation Xcode, ainsi que
`sdl3`, `sdl3_image` et `sdl3_ttf` avec Homebrew. Utiliser les presets `macos`
et `macos-debug`. Le binaire `build/lib/libglad.a` fourni doit correspondre
à l'architecture du Mac. Ce chemin doit être validé sur le second poste.

## Ce qui est recompilé

| Modification | Travail attendu |
| --- | --- |
| Aucun fichier modifié | Aucune compilation ou liaison |
| Seulement `src/main.cpp` | Objet du main et liaison de l'application |
| Un `.cpp` du moteur | Cet objet, archive moteur et liaison de l'application |
| Un en-tête inclus | Les fichiers qui incluent cet en-tête, puis liaison |

Si le code de la bibliothèque change, il faut la reconstruire. Ne pas
supprimer `out/` et ne pas utiliser `--clean-first` à chaque lancement.
Les résultats sous `out/` ne sont pas versionnés.

`verifierBibliotheques()` est uniquement une démonstration de liaison, pas
l'implémentation du moteur du sprint 2. Le test ne valide pas la création
d'un contexte OpenGL, le décodage d'images ni le rendu de texte.

## Validation sur Windows

Configuration, compilation et test de liaison réussis avec MSVC x64.
Une seconde compilation sans changement conserve les dates de `moteur.lib`
et `program.exe`. Après changement de date du seul `main.cpp`, l'application
est reconstruite et `moteur.lib` reste inchangée. macOS reste à valider.

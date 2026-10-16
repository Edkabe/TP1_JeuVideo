# Compilation Windows — environnement du cours

Cette configuration reprend les outils Windows de `VSCOGL (1).zip` :
VS Code, MSVC x64 (`cl.exe`), SDK Windows, bibliothèques et DLL dans `build/`.
Elle n'utilise plus CMake, vcpkg ou Ninja.

Installer VS Code, l'extension Microsoft C/C++, puis Visual Studio ou ses
Build Tools avec la charge « Développement Desktop en C++ » et un SDK Windows.
Ouvrir le dossier contenant `.vscode`, `src`, `include` et `build`.

- **Ctrl+Maj+B** : tâche `(Windows) Compiler`.
- **F5** : tâche de compilation suivie du débogage de `build/program.exe`.
- Palette **Tasks: Run Task → (Windows) Vérifier** : compilation puis test
  de liaison des dépendances, sans fenêtre.

Les tâches appellent `scripts/compiler-windows.ps1`. Ce script initialise
le terminal développeur x64 si nécessaire, compile les `.cpp` avec `cl.exe`,
archive le moteur avec `lib.exe` et lie l'application avec `link.exe`.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ./scripts/compiler-windows.ps1
./build/program.exe
./build/program.exe --check
./build/program.exe --smoke
```

Pour vérifier la sauvegarde et le rechargement des paramètres dans une
nouvelle instance (sans ouvrir de fenêtre) :

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ./scripts/compiler-windows.ps1 -TesterParametres
```

## Fichiers produits

| Chemin | Contenu |
| --- | --- |
| `build/obj/moteur/` | Objets compilés du moteur et dépendances aux en-têtes |
| `build/lib/moteur.lib` | Votre bibliothèque statique |
| `build/program.exe` | Application |
| `build/assets/shaders/` | Copie des nuanceurs de la démonstration |

Les dépendances SDL3, SDL3_image, SDL3_ttf et GLAD fournies par le cours
restent dans `build/lib/`; leurs DLL restent dans `build/`. GLM est dans
`external/glm/` et ne nécessite pas de `.lib`.

Le script mémorise les options et les en-têtes utilisés par chaque `.cpp`
avec `/sourceDependencies`. Sans changement, il ne recompile rien.
Un changement du main recompile le main puis relie l'application; un
changement du moteur reconstruit les objets concernés puis `moteur.lib`.
Un changement d'en-tête recompile les sources qui l'incluent.

Les anciens fichiers sous `out/` ne sont plus utilisés. Ne pas les confondre
avec les nouvelles sorties dans `build/`. Les sorties compilées sont ignorées
par Git. Cette configuration cible Windows, conformément à l'environnement
demandé; le précédent lancement macOS n'est pas utilisé.

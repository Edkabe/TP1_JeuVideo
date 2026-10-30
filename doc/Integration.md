# Validation de l'integration — issue #24

## Compilation et tests automatiques

Depuis **x64 Native Tools Command Prompt for VS 2026**, dans le dossier du projet :

```cmd
powershell -NoProfile -ExecutionPolicy Bypass -File ".\scripts\compiler-windows.ps1" -Verifier -TesterParametres -TesterMathematiques -TesterRessources -TesterEtiquette -TesterScenes -TesterApplicationParametres -TesterIntegration -CompilerDemoScenes
```

Les vérifications couvrent les bibliothèques, les paramètres, les ressources, les scènes, leurs observateurs, les clics du menu et les transitions répétées entre les vraies scènes. Le test d'intégration vérifie l'identité des objets `Font` et `Texture` partagés par le menu et les paramètres, puis leur réutilisation après plusieurs parcours. Pour exercer aussi le chargement graphique des ressources :

```cmd
cd /d build
resource-tests.exe --graphics
cd ..
```

## Parcours manuel

Lancer `build\program.exe`, puis parcourir les écrans plusieurs fois :

1. Menu → Paramètres → Retour, puis menu → Paramètres → Appliquer.
2. Paramètres → modifier chaque sélecteur, case et glisseur; vérifier les valeurs affichées et rouvrir l'écran après application.
3. Menu → Crédits → Retour au menu.
4. Menu → Jeu, vérifier que la barre de progression avance jusqu'à 100 %, puis retourner au menu.
5. Répéter les transitions et fermer avec Quitter puis avec le bouton de fermeture de la fenêtre.

`SettingsTests` vérifie la première exécution sans fichier, la sauvegarde et le rechargement dans une nouvelle instance. `SceneManagerTests` vérifie les transitions et qu'une case à cocher cesse de recevoir les événements après la désactivation de sa scène. `ResourceTests --graphics` vérifie le cycle des ressources avec un contexte OpenGL actif.

### Contrat de chargement des scènes

Un `load()` qui retourne `true` garantit que les fichiers requis (polices et images), les composants et les abonnements aux événements de la scène sont préparés pour ses méthodes `update()` et `render()`. S'il retourne `false`, `getError()` fournit le diagnostic et `unload()` peut être appelé pour libérer tout chargement partiel. Les erreurs détectées par une scène après son activation sont conservées par celle-ci et propagées par `Engine::run` jusqu'à l'appelant; l'application les affiche sur `stderr`, puis ferme le moteur avec son garde RAII.

Les scènes principales valident le contexte OpenGL, leurs fichiers et dimensions de fenêtre avant de s'activer. Les contrôles réalisent toutefois une partie de la création de leurs programmes OpenGL au premier rendu; un échec du pilote ou d'une compilation de nuanceur à ce stade devient une erreur de scène et fait échouer la boucle, plutôt que d'être ignoré.

### États OpenGL des contrôles

Les contrôles restaurent les états globaux qu'ils modifient afin que l'ordre de rendu ne change pas le résultat suivant. `Button`, `Slider`, `ProgressBar` et `Label` restaurent le programme, le VAO, le mélange (activation, facteurs et équations), ainsi que profondeur et culling lorsqu'ils les changent. `Image` restaure en plus la texture 2D de l'unité 0, son sampler et l'unité de texture active. `Selector` restaure profondeur, culling et équations de mélange; ses boutons et son étiquette restaurent leurs propres états. `CheckBox` restaure programme, VAO et binding `GL_ARRAY_BUFFER`, en initialisation comme en rendu; elle ne modifie pas les textures. Les opérations de chargement de texture restaurent le binding de texture, le pixel-unpack buffer et les paramètres `GL_UNPACK_*`.

### Résultat du parcours manuel

Parcours effectué le 30 octobre 2026 avec `build/program.exe` :

| Étape | Résultat observé |
| --- | --- |
| 1. Menu → Paramètres → Retour | Réussi. Le panneau s'ouvre et revient au menu. |
| 1–2. Appliquer langue anglaise, 1280×720 fenêtré, VSync désactivée, sensibilité 13,19 et FOV 104° | Réussi. La fenêtre change de taille, le message « Settings applied and saved. » apparaît et le menu se retraduit. Les valeurs ont ensuite été restaurées aux défauts. |
| 2. Plein écran à 960×540 | Échec matériel observé: SDL annonce « La resolution choisie n'est pas disponible en plein ecran. » L'erreur s'affiche sur `stderr` et le moteur se ferme proprement. |
| 3. Menu → Crédits → Retour | Réussi. |
| 4. Menu → Jeu → progression à 100 % → Retour | Réussi. L'écran affiche « Démarrage simulé terminé. » |
| 5. Transitions répétées, bouton Quitter | Réussi. Le processus se termine. |
| 5. Fermeture par la croix de la fenêtre | Réussi. Le processus se termine. |

Les tests automatisés couvrent aussi les transitions, l'identité des ressources partagées et la restauration des bindings de texture, buffer et VAO par `CheckBox`. Le plein écran dépend des modes disponibles sur la machine; l'échec observé est signalé, puis le moteur termine proprement.

L'application de la résolution, du plein écran et de la VSync est intégrée
dans `Engine::applySettings()` et couverte par `EngineSettingsApplicationTests`
et `SceneIntegrationTests`. Le test d'intégration vérifie aussi l'application
et la sauvegarde des choix de la scène Paramètres.

Le parcours graphique manuel reste complémentaire aux tests automatisés,
notamment pour vérifier visuellement les dispositions après redimensionnement
et sur des écrans à densité différente.

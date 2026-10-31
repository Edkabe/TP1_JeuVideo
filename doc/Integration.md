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

Contrat de `load()`: `true` signifie que le contexte requis existe, les noms des scènes de retour (et les destinations du menu principal) sont enregistrés, les fichiers de police et de texture requis sont chargés, les composants sont construits et configurés, les abonnements aux événements sont posés, et les dimensions nécessaires à la disposition sont disponibles. Une transition ne réussit donc pas avec une destination manifestement absente ou un fichier de ressource manquant. `false` doit laisser `getError()` renseigné; `SceneManager` appelle alors `unload()` pour nettoyer toute préparation partielle et conserve l’ancienne scène active.

`MainScene`, `SettingsScene`, `CreditsScene` et `DemoGameScene` vérifient respectivement leurs destinations, leurs fichiers et dimensions; leurs contrôles et abonnements sont préparés dans `load()`. `SettingsScene` initialise également les cases, vérifie les plages des glisseurs et la sélection des valeurs. Chaque `unload()` retire les observateurs puis libère les composants qu’il possède. Les scènes ne libèrent pas directement les polices et textures partagées; les références du gestionnaire et des clients gouvernent leur durée de vie.

Limite explicite du contrat: `Image`, `Label`, `Button`, `Slider` et `ProgressBar` créent encore leurs programmes/VAO/VBO au premier rendu (la case initialise son programme pendant `load()`). La compilation ou l’allocation GPU peut donc échouer après une transition malgré un `load()` réussi. Ces erreurs de rendu ne sont pas ignorées: le contrôle retourne `false` avec un diagnostic, la scène le conserve, `Engine::run` retourne `false` et l’application journalise l’erreur avant l’arrêt. `load()` garantit les ressources CPU et fichiers nécessaires, pas la réussite future du pilote pour chaque commande OpenGL.

### États OpenGL des contrôles

Les contrôles restaurent les états globaux qu'ils modifient afin que l'ordre de rendu ne change pas le résultat suivant. `Button`, `Slider` et `ProgressBar` restaurent le programme, le VAO, l’activation du mélange, ses facteurs et équations, ainsi que profondeur et culling. `Label` restaure programme, VAO, texture 2D de l’unité 0, unité de texture active, activation du mélange et facteurs de mélange; la création de ses sommets restaure aussi `GL_ARRAY_BUFFER`. `Image` restaure programme, VAO, texture 2D de l’unité 0, sampler, unité de texture active, mélange, profondeur et culling; sa création de sommets restaure aussi `GL_ARRAY_BUFFER`. `Selector` restaure profondeur, culling et équations de mélange, tandis que ses boutons et son étiquette restaurent leurs propres états. `CheckBox` restaure programme, VAO et binding `GL_ARRAY_BUFFER` pendant l’initialisation et le rendu; elle ne modifie pas les textures ni le mélange. Le rendu de chaque contrôle vérifie aussi l’erreur OpenGL après dessin et retourne `false` avec un diagnostic. Le chargement d’une texture restaure le binding de texture, le pixel-unpack buffer et les paramètres `GL_UNPACK_*`.

### Parcours manuel reproduit le 30 octobre 2026

Parcours effectué avec build/program.exe sous Windows, dans un répertoire temporaire vierge utilisé comme répertoire de travail pour isoler settings.bin. Il décrit cette machine de développement; le groupe doit encore le confirmer sur le poste de remise.

| Étape | Résultat observé |
| --- | --- |
| 1. Menu → Paramètres → Retour | Réussi. Le panneau s’ouvre puis revient au menu. |
| 1–2. Paramètres → Appliquer | Réussi avec anglais, 1280 × 720 fenêtré, VSync désactivée, sensibilité 13,69 et FOV 104°. Le message « Settings applied and saved. » apparaît; le menu est traduit en anglais et la fenêtre adopte la résolution. |
| 2. Plein écran à 1280 × 720 puis retour au mode fenêtré | Réussi. L’application des deux changements affiche le message de confirmation et le jeu reste utilisable. |
| 3. Menu → Crédits → Retour au menu | Réussi. |
| 4. Menu → Jeu → progression à 100 % → Retour | Réussi deux fois. L’écran finit par afficher « Simulated startup complete. » et la barre est pleine. |
| 5. Paramètres, crédits et jeu après plusieurs transitions | Réussi. Les scènes restent accessibles après les transitions répétées. |
| 5. Bouton Quitter | Réussi. La fenêtre et le processus du jeu disparaissent. |
| 5. Bouton de fermeture de la fenêtre | Réussi. Au lancement suivant, la fermeture de la fenêtre arrête également le processus. |

Un parcours antérieur du 30 octobre rapportait un échec matériel en plein écran à 960 × 540 (la résolution choisie n'est pas disponible en plein écran). Ce résultat concerne un autre essai et ne s’est pas reproduit avec 1280 × 720 lors du parcours ci-dessus; les modes plein écran dépendent de l’écran.

Les résultats ci-dessus sont des observations locales et non une confirmation sur l’ordinateur utilisé pour la remise. Refaire les étapes dans l’ordre sur cet ordinateur et consigner les erreurs avec leur numéro d’étape.
L'application de la résolution, du plein écran et de la VSync est intégrée
dans `Engine::applySettings()` et couverte par `EngineSettingsApplicationTests`
et `SceneIntegrationTests`. Le test d'intégration vérifie aussi l'application
et la sauvegarde des choix de la scène Paramètres.

Le parcours graphique a été reproduit localement le 30 octobre 2026; il reste à confirmer sur le poste de remise, notamment pour vérifier visuellement les dispositions après redimensionnement et sur des
écrans à densité différente. Consigner pour chaque étape le résultat observé,
le matériel utilisé, et toute erreur avec son numéro d’étape.

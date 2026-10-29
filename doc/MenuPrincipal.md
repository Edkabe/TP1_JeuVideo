# Scene Principale — issue #19

`MainScene` utilise les controles et ressources du projet : `Image` pour l'arriere-plan, `Label` pour le titre et quatre `Button` pour Demarrer, Parametres, Credits et Quitter.
Elle suit l'interface `IScene` et le gestionnaire effectivement pousses par Oceane.

```cpp
scenes.addScene("menu",std::make_unique<moteur::MainScene>(
    scenes,cheminPolice,cheminImage,
    moteur::MainScene::Targets{"demonstration", nomSceneParametres, "credits"}));
scenes.setActiveScene("menu");
engine.run(scenes);
scenes.clear(); // Avant engine.shutdown().
```

Les destinations sont les noms fournis par l'application, pas des noms imposes aux classes d'Oceane.

## Comportement

- Aucun contenu charge a la construction ; les controles sont crees dans `load()`.
- Texture d'arriere-plan et deux polices obtenues depuis le gestionnaire global de ressources. Les reactivations reutilisent ces ressources.
- Les controles sont detruits et leurs abonnements retires dans `unload()`, contexte encore actif.
- Les etiquettes de boutons sont centrees. Les survols et clics gauches utilisent le gestionnaire d'evenements propre a la scene.
- Un clic navigue vers la destination configuree. Une destination absente ou un chargement echoue conserve le menu et affiche un message ; `getError()` fournit le detail technique.
- Quitter arrete le moteur. L'application libere ensuite les scenes, les ressources et le contexte.
- L'arriere-plan vectoriel original `assets/images/menu-background.svg` est charge comme texture par SDL_image et etire a la taille du viewport.
- Titre et boutons sont recentres au redimensionnement, sans annuler l'appui d'un bouton lors d'un simple rendu.
- `getButton(Action)` donne un pointeur const emprunte pour consulter un bouton ; il devient invalide apres dechargement.

## Application locale

Le lancement normal de `build/program.exe` affiche maintenant le menu :

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\compiler-windows.ps1
.\build\program.exe
```

Demarrer ouvre `DemoGameScene`, une destination provisoire explicitement choisie avec Eddy. Elle affiche un titre, une barre de progression animee, le glisseur et un bouton Retour au menu ; aucun jeu complet n'est implemente.
Parametres ouvre `SettingsScene`. Les choix sont initialises avec les valeurs actuelles ; Appliquer les enregistre puis revient au menu, tandis que Retour abandonne les changements.
Credits ouvre la vraie `CreditsScene`, avec retour au menu.

L'enregistrement de la scene modifie le fichier `settings.bin`. L'application effective de la resolution, du plein ecran et de la VSync au moteur est suivie dans l'issue #23.

Le triangle reste disponible avec `--triangle` ; `--smoke` conserve son test graphique automatique et `--check` la verification des dependances.

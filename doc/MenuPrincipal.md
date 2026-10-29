# Scene Principale — issue #19

`MainScene` utilise les controles et ressources du projet : `Image` pour l'arriere-plan, `Label` pour le titre et quatre `Button` pour Jeu, Parametres, Credits et Quitter.
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

Jeu ouvre `DemoGameScene`, un mini-jeu de tir aux cibles chronométré. Déplace le viseur avec la souris et clique dans la cible pour marquer des points avant la fin du compte à rebours. Recommencer lance une nouvelle partie; Retour au menu ferme le jeu.
Parametres ouvre `SettingsScene`. Les choix sont initialises avec les valeurs
actuelles. **Appliquer** applique et enregistre les choix puis laisse l'écran
ouvert pour permettre d'autres changements; **Retour** revient au menu.
Les changements non appliques ne sont pas sauvegardes.
Credits ouvre la vraie `CreditsScene`, avec retour au menu.

La langue choisie traduit le menu, l'écran des paramètres, les crédits et le
mini-jeu. La sensibilité multiplie le déplacement du viseur; le champ de vision
change la taille apparente des cibles. L'application des paramètres modifie
`settings.bin` et applique aussi à la fenêtre la résolution, le plein écran
et la VSync choisis.

Le triangle reste disponible avec `--triangle` ; `--smoke` conserve son test graphique automatique et `--check` la verification des dependances.

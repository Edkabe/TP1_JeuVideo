# Validation de l'integration — issue #24

## Compilation et tests automatiques

Depuis **x64 Native Tools Command Prompt for VS 2026**, dans le dossier du projet :

```cmd
powershell -NoProfile -ExecutionPolicy Bypass -File ".\scripts\compiler-windows.ps1" -Verifier -TesterParametres -TesterMathematiques -TesterRessources -TesterEtiquette -TesterScenes -TesterApplicationParametres -TesterIntegration -CompilerDemoScenes
```

Les vérifications couvrent les bibliothèques, les paramètres, les ressources, les scènes, leurs observateurs, les clics du menu et les transitions répétées entre les vraies scènes. Le test d'intégration vérifie aussi que les ressources partagées ne s'accumulent pas après plusieurs parcours. Pour exercer aussi le chargement graphique des ressources :

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

L'application de la résolution, du plein écran et de la VSync est intégrée
dans `Engine::applySettings()` et couverte par `EngineSettingsApplicationTests`
et `SceneIntegrationTests`. Le test d'intégration vérifie aussi l'application
et la sauvegarde des choix de la scène Paramètres.

Le parcours graphique manuel reste complémentaire aux tests automatisés,
notamment pour vérifier visuellement les dispositions après redimensionnement
et sur des écrans à densité différente.

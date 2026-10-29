# Demonstration du sprint 2

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\compiler-windows.ps1 -CompilerDemoScenes
.\build\scene-demo.exe
```

Un clic gauche sur le bouton passe de la scene A (fond bleu) a la scene B (fond vert), puis permet de revenir. Chaque transition charge la destination et decharge la scene precedente. Fermer la fenetre termine le programme.

La demonstration utilise `Button`, `Label` et le gestionnaire de scenes d'Oceane. Ce sont deux scenes d'exemple, pas les interfaces finales du TP.
Arial est cherche dans le dossier Windows. Une autre police peut etre fournie en argument :

```powershell
.\build\scene-demo.exe "C:\chemin\police.ttf"
```

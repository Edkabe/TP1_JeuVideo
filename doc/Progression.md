# Barre de progression — issue #16, sprint 3

`ProgressBar` affiche une barre horizontale sans dependance au gestionnaire de scenes.
Les positions et dimensions sont en pixels du viewport courant, depuis le coin haut gauche.

```cpp
moteur::ProgressBar barre;
barre.setPosition({30, 150});
barre.setDimensions({420, 30});
barre.setRange(0, 100);
barre.setValue(50);
barre.setBorderColor({0.7f, 0.7f, 0.7f});
barre.setBackgroundColor({0.12f, 0.12f, 0.16f});
barre.setProgressColor({0.15f, 0.7f, 0.35f});
// Dans update : barre.advance(deltaTime * 20);
// Dans render, apres glViewport et glClear : barre.render();
```

- `setRange(min, max)` exige des bornes finies et `min < max`. En cas d'erreur, les anciennes bornes et la valeur restent intactes.
- `setValue()` borne la valeur entre minimum et maximum ; NaN et infinis sont refuses.
- `advance()` ajoute un increment (positif ou negatif), avec saturation aux bornes.
- `getProgress()` donne la proportion entre 0 et 1, en tenant compte du minimum. Le calcul intermediaire en double evite les debordements aux limites de float.
- Les dimensions negatives et les positions/dimensions non finies sont refusees. Une dimension nulle rend le controle invisible.
- La progression remplit uniquement l'interieur de la bordure. La largeur de bordure est configurable et limitee visuellement a la moitie du plus petit cote.
- Les sources GLSL sont chargees depuis `assets/shaders/progress.vert` et `progress.frag`, copiees par le script Windows.
- Les liaisons programme/VAO et les etats de melange, profondeur et culling modifies sont restaures apres rendu. Le framebuffer, viewport et scissor restent ceux de l'appelant.
- Les ressources graphiques sont creees au premier rendu et doivent etre liberees/detruites avant le contexte. `release()` permet une recreation au rendu suivant.

# Selecteur — issue #17, sprint 3

`Selector` compose les boutons precedent/suivant d'Eddy et l'etiquette `Label` poussee par Oceane. Aucun gestionnaire de scenes supplementaire n'est necessaire.

```cpp
auto selecteur = std::make_shared<moteur::Selector>(police,
    std::vector<std::string>{"Francais", "English", "Espagnol"});
selecteur->setPosition({30, 40});
selecteur->setDimensions({360, 50});
auto& evenements = scene.getEventManager();
for (Uint32 type : {SDL_EVENT_MOUSE_MOTION, SDL_EVENT_MOUSE_BUTTON_DOWN,
                   SDL_EVENT_MOUSE_BUTTON_UP, SDL_EVENT_WINDOW_FOCUS_LOST,
                   SDL_EVENT_WINDOW_MOUSE_LEAVE})
    evenements.addObserver(selecteur, type);
// Dans le rendu de la scene, apres glViewport et glClear :
selecteur->render();
// Consultation (nullptr si la liste est vide) :
const std::string* choix = selecteur->getSelectedItem();
```

La scene conserve le `shared_ptr<Selector>` ; le gestionnaire conserve une reference faible. Inscrire seulement le selecteur, qui transmet les evenements a ses deux boutons internes.

- Le bouton precedent est a gauche, le suivant a droite. Le texte selectionne est centre entre eux.
- La position et les dimensions suivent les conventions des autres controles : pixels du viewport, origine en haut a gauche.
- La liste initiale et chaque remplacement par `setItems()` selectionnent le premier element. Une liste vide donne `noSelection`, `nullptr` et un texte vide.
- `previous()` et `next()` retournent faux aux limites sans changer la selection. La navigation ne boucle pas.
- `setSelectedIndex()` refuse un indice hors limites sans changer la selection. Les indices commencent a zero.
- Le pointeur de `getSelectedItem()` est emprunte : ne pas le conserver apres remplacement de la liste ou destruction du selecteur.
- Position/dimensions non finies et dimensions negatives refusees. Une dimension nulle rend le controle invisible et ses boutons non cliquables.
- Le texte est recentre apres changement de selection, police ou geometrie et a chaque rendu. Les elements longs ne sont pas tronques automatiquement ; choisir une largeur adaptee.
- Les couleurs de texte et de boutons sont configurables. Les flèches précédent/suivant réutilisent le comportement de `Button`.
  Un clic maintenu puis relâché dans la moitié correspondante de la zone centrale sélectionne aussi l'élément précédent ou suivant.
- Detruire/liberer le selecteur avant la fermeture du contexte OpenGL. `release()` libere son rendu et celui des boutons sans decharger la police partagee.

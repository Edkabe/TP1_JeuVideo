# Diagramme de classes du TP1

Le diagramme reflète les classes et les responsabilités présentes dans `include/moteur/` et `src/moteur/`. Il résume les attributs principaux, les opérations publiques et les relations du moteur, des ressources, du rendu OpenGL, des scènes, des événements et des contrôles visuels.

- [Diagramme vectoriel SVG](Classes.svg)
- [Aperçu PNG](Classes.png)
- [Diagramme UMLet initial](Classes-initiale.uxf) — conservé comme document historique de conception.

Les scènes héritent de `IScene`; les contrôles interactifs héritent de `EventObserver`. `Engine` possède la fenêtre, le gestionnaire de ressources et les paramètres. `SceneManager` possède les scènes nommées, et chaque scène possède son gestionnaire d’événements. Les relations détaillées figurent au bas du diagramme.

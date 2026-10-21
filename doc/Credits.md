# Scene Credits — issue #21, sprint 3

`CreditsScene` derive de l'interface `IScene` effectivement poussee par Oceane. Elle affiche le projet, les auteurs `Edkabe` et `oceane-talla`, les bibliotheques utilisees et un bouton Retour au menu.

```cpp
scenes.addScene("credits", std::make_unique<moteur::CreditsScene>(
    scenes, cheminPolice, "menu"));
// Une scene nommee "menu" doit deja etre enregistree avant l'activation.
scenes.setActiveScene("credits");
engine.run(scenes);
scenes.clear(); // Avant engine.shutdown().
```

- Aucun contenu n'est charge a la construction.
- `load()` verifie la destination de retour et le contexte, obtient/charge les polices partagees via le gestionnaire de ressources et cree les etiquettes/bouton.
- Le bouton emet un clic gauche via le gestionnaire d'evenements propre a la scene, puis appelle `setActiveScene()` avec le nom fourni.
- `unload()` retire les inscriptions et detruit les controles avec le contexte actif. Les polices restent dans le gestionnaire de ressources afin d'etre reutilisees lors des prochaines activations.
- Les programmes des controles utilisent le cache binaire local de l'issue #22.
- Une destination absente ou une police introuvable fait echouer le chargement sans remplacer la scene courante. Si le retour echoue, les credits restent actifs et `getError()` fournit un diagnostic.
- `isLoaded()` expose l'etat du contenu. Les controles sont recentres lors du redimensionnement de la fenetre. Leur geometrie n'est pas reappliquee inutilement pour conserver un appui souris entre deux rendus.

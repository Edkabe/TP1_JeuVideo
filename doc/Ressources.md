# Issue 3 — gestionnaire de ressources

`Resource` est la base abstraite. `ResourceManager` associe un nom à un
`shared_ptr<Resource>` : aucune copie par valeur des ressources polymorphes.

| Opération | Résultat |
| --- | --- |
| `add(name, resource)` | true si ajoutée; false pour nom vide, pointeur nul ou doublon |
| `add(resource)` | Utilise le titre déjà défini dans la ressource |
| `get<Texture>(name)` / `get<Font>(name)` | Même objet partagé; nullptr pour nom absent ou mauvais type |
| `contains(name)` | Présence du nom |
| `remove(name)` | Retire ce nom; false si absent |
| `clear()` | Retire toutes les associations |
| `size()` | Nombre de noms enregistrés |

Charger la ressource avant de l'ajouter permet de distinguer erreur de
chargement et doublon. Le gestionnaire ne charge pas automatiquement les
fichiers. Plusieurs noms peuvent désigner le même objet. Un retrait ne
détruit pas un objet encore détenu par un contrôle. Une ressource non chargée
peut aussi être stockée; son état se consulte avec `isLoaded()`.

Conformément au diagramme de classes courant (`doc/Classes.md`), chaque
ressource possède un titre consultable avec `getTitle()`. Il peut être fourni
au constructeur : `Texture("fond_menu")` ou `Font(24.0f, "titre_menu")`.
Pour conserver les appels existants, `add(name, resource)` attribue le premier
nom comme titre si celui-ci est vide. Les alias restent possibles et ne
modifient pas le titre initial. Un doublon refusé ne change pas la ressource.

## Ressources réelles

- `Texture::load(path)` charge avec SDL_image puis envoie les pixels RGBA à
  OpenGL. `bind()`, `getWidth()`, `getHeight()` et le transtypage explicite
  vers `GLuint` permettent son utilisation. Contexte OpenGL/GLAD requis.
- `Font::load(path)` ouvre une police SDL_ttf. `getPointSize()`, `getHeight()`
  et le transtypage explicite vers `TTF_Font*` permettent son utilisation.
  Chaque Font équilibre ses appels TTF_Init/TTF_Quit; plusieurs polices
  peuvent coexister. Il n'est pas nécessaire d'initialiser SDL_ttf soi-même.
- Un chargement échoué conserve la ressource déjà chargée et renseigne
  `getError()`. `release()` peut être appelée plusieurs fois.

`Engine::getResourceManager()` expose le gestionnaire. `shutdown()` le vide
avant de fermer le contexte. **Les références clientes aux textures doivent
être relâchées avant shutdown**, sinon leur destruction arriverait après
la fermeture du contexte. Les opérations OpenGL et destructions des textures
se font sur le thread du contexte actif. Les identifiants et pointeurs
obtenus par transtypage sont empruntés : ne pas les supprimer manuellement.

```cpp
auto texture = std::make_shared<moteur::Texture>();
if (texture->load("assets/fond.png")) {
    engine.getResourceManager().add("fond_menu", texture);
}
auto fond = engine.getResourceManager().get<moteur::Texture>("fond_menu");
// Utiliser fond pendant la scène, puis libérer les références clientes.
fond.reset();
texture.reset();
```

## Vérification Windows

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ./scripts/compiler-windows.ps1 -TesterRessources
./build/resource-tests.exe --graphics
```

Les tests sans fenêtre vérifient noms, types, doublons et durée de vie.
Le mode graphique vérifie les pixels d'un PNG, la restauration de l'état
OpenGL, le rechargement, une police réelle et la fermeture du gestionnaire.

La précision du professeur inclut désormais toutes les ressources utiles au
TP dans cette partie : Texture et Font sont implémentées. Le contrôle Label
reste distinct et n'est pas modifié. La police de test vient de Windows et
n'est pas copiée dans le dépôt.

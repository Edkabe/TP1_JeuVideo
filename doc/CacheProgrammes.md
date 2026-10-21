# Cache binaire des programmes — issue #22

`ProgramCache` charge un programme depuis un fichier binaire compatible ou compile/lie ses fichiers GLSL puis sauvegarde un nouveau cache. Les sources restent necessaires pour verifier que le cache correspond bien au programme attendu.

```cpp
moteur::ProgramCache cache;
bool ok = cache.load(program,
    {{GL_VERTEX_SHADER, "assets/shaders/image.vert"},
     {GL_FRAGMENT_SHADER, "assets/shaders/image.frag"}},
    "cache/programs/image.bin", engine.getSettings());
if (!ok) { /* cache.getError() : programme non charge, ancien programme conserve */ }
// cache.wasLoadedFromCache() indique si la compilation a ete evitee.
// cache.getWarning() signale une sauvegarde impossible ou un pilote sans support.
```

## Fonctionnement

1. Lire les sources et calculer leur empreinte avec les types de nuanceurs et l'identite du pilote (constructeur, GPU, version OpenGL).
2. Verifier signature, version, format, taille et somme de controle du cache. Les entrees corrompues, tronquees, surdimensionnees, incompatibles ou perimees sont ignorees.
3. Tenter le chargement OpenGL et verifier la liaison. En cas de rejet, compiler et lier les sources.
4. Recuperer le binaire et ecrire un fichier temporaire voisin, puis remplacer le cache. Un echec d'ecriture n'empeche pas d'utiliser le programme compile.
5. Conserver le format binaire dans les parametres du moteur et les sauvegarder. Un echec est signale comme avertissement, sans perdre le programme graphique.

Si les sources ne peuvent pas etre compilees/liees, le programme precedemment utilisable et le cache existant sont conserves.
Les empreintes FNV-1a detectent les changements et corruptions accidentelles ; elles ne constituent pas une signature cryptographique.
Un cache est limite a 64 Mio de binaire et chaque source a 16 Mio.

Le support est detecte avec OpenGL 4.1 ou `GL_ARB_get_program_binary` et au moins un format disponible. Les entrees de l'extension sont resolues seulement si elle est annoncee par le contexte courant. Le moteur reste en OpenGL 3.3. Sans support, les sources sont utilisees sans creer de faux cache.

## Integration locale

- Image : `build/cache/programs/image.bin`
- Bouton : `build/cache/programs/button.bin`
- Barre de progression : `build/cache/programs/progress.bin`
- Etiquette existante d'Oceane : `build/cache/programs/label.bin`

Le selecteur utilise les programmes de ses boutons et de son etiquette. Les sources de l'etiquette ont ete deplacees vers `assets/shaders/label.vert` et `label.frag` sans changer leur contenu ni l'API du controle.
Les objets OpenGL doivent toujours etre detruits/liberes avant la fermeture du contexte. Les valeurs des uniformes sont reappliquees par les controles apres chargement.

## Parametres

Les fichiers `Settings` effectivement pousses par Oceane ont ete recuperes depuis `origin/main`, puis etendus avec `getProgramBinaryFormat()` et `setProgramBinaryFormat()`.
Le format de fichier passe en version 2. Les fichiers de version 1 restent lisibles, tous leurs champs sont conserves et le nouveau champ vaut initialement zero (format inconnu). La prochaine sauvegarde ecrit la version 2. Le fichier de parametres utilise par defaut est `settings.bin`, relatif au dossier de lancement, comme dans son implementation.

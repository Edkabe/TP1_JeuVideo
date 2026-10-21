# Classes adaptées de l'atelier 2

Les en-têtes sont dans `include/moteur/`, les implémentations dans
`src/moteur/`. Ils sont regroupés dans `build/lib/moteur.lib`.

| Classe | Responsabilité |
| --- | --- |
| Engine | Singleton, fenêtre unique, initialisation SDL, boucle avec deltaTime et arrêt. |
| Window | Fenêtre SDL3 et contexte OpenGL 3.3 Core, chargement GLAD et dimensions en pixels. |
| Shader | Compilation depuis un fichier ou une chaîne et journal d'erreur. |
| VertexShader | Spécialisation pour GL_VERTEX_SHADER. |
| Program | Liaison des nuanceurs et activation du programme. |
| VBO | Propriété du tampon de sommets GPU, envoi et libération de ses données. |
| VAO | Format des attributs de sommets et capture du VBO lié lors de leur configuration. |

Les propriétaires d'identifiants OpenGL ne sont pas copiables. Détruire leurs
instances **avant `Engine::shutdown()`**, sur le thread du contexte actif.
Le programme de démonstration respecte cet ordre.

Après compilation depuis VS Code (**Ctrl+Maj+B**), lancer `build/program.exe`
pour afficher le triangle orange. Échap ou la croix arrête la boucle.
Les sources GLSL sont copiées depuis `assets/shaders` à côté de l'application;
aucune macro CMake n'est nécessaire pour les retrouver.

`--check` vérifie la liaison des dépendances. `--smoke` crée une fenêtre
masquée et vérifie les erreurs de nuanceur/liaison, la propriété et le
transfert des VBO/VAO, la capture du VBO par le VAO, la protection contre une
boucle réentrante et le rendu du triangle alimenté par un VBO.

Les paramètres, scènes, gestionnaires, contrôles et programmes binaires
restent des étapes ultérieures. Voir [la compilation Windows](Bibliotheques.md).

## Diagramme de classes

Le diagramme distingue en bleu les classes déjà présentes dans la version
d'Eddy et en vert les classes proposées pour compléter le TP. Le SVG est la
version vectorielle éditable; le PNG sert d'aperçu.

[Ouvrir le diagramme éditable](Classes.svg) · [Ouvrir l'aperçu PNG](Classes.png)

L'ancien diagramme UMLet a été conservé sous le nom [Classes-initiale.uxf](Classes-initiale.uxf).

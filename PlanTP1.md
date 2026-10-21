# Plan de réalisation du TP1 à partir d’Atelier2JV

## But et règle de travail

Le TP1 consiste à faire évoluer l’atelier d’Eddy en un moteur graphique orienté objet, mono-fenêtre, puis à construire les contrôles et les trois scènes demandés dans [`enonce.md`](enonce.md).

Règle de portée : implémenter seulement ce que l’énoncé demande et réutiliser seulement les notions vues au cours. Chaque étape sera expliquée et validée avec l’équipe avant de passer à la suivante. Ce document est un plan; il ne constitue pas une autorisation de générer le code des étapes suivantes.

## Point de départ : l’atelier

L’archive de départ est `Atelier2JV.zip`, téléchargée depuis Eddy. Elle contient notamment :

- `Engine` : singleton, initialisation SDL/OpenGL, boucle d’événements, mise à jour, rendu et libération;
- `Window` : classe abstraite avec callbacks d’événement, mise à jour, rendu et libération;
- `Shader`, `VertexShader` et `VAO` : premières abstractions OpenGL;
- `TriangleWindow` et `main.cpp` : démonstration interactive d’un triangle.

Adaptations nécessaires au TP :

1. Passer du moteur qui conserve plusieurs fenêtres à une seule fenêtre.
2. Préserver et adapter le cycle SDL/OpenGL et la boucle de jeu déjà présents.
3. Séparer proprement les objets OpenGL demandés (nuanceurs, programme, tampons, tableau de sommets) et assurer leur libération avant la destruction du contexte.
4. Ajouter paramètres, ressources titrées, scènes et gestionnaire d’événements par scène.
5. Ajouter les contrôles visuels et les scènes demandées.

L’archive contient aussi des fichiers générés par Visual Studio (`.vs`, sorties `x64/Debug`) et des chemins de dépendances propres au poste d’Eddy. Ils ne doivent pas devenir des sources du TP; la structure de travail propre sera décidée avant leur copie ou leur retrait. Le projet Git actuel ne remplace pas l’atelier : il fournit une démonstration de liaison et les dépendances prêtes à l’emploi. La compilation souhaitée se fera dans VS Code avec une tâche qui appelle directement MSVC, sur le modèle d’Atelier3; CMake ne sera pas utilisé.

## Étapes proposées

### Étape 0 — Valider la base et l’organisation

- Confirmer que `Atelier2JV.zip` est bien la version de référence.
- Établir une copie de travail propre sans fichiers de compilation temporaires.
- Comparer les fichiers source de l’atelier à la branche actuelle du projet.
- Convenir de la structure finale et vérifier la compilation de la base avec la tâche MSVC de VS Code avant les changements.

**Issues liées :** [#2 — Implémentation des librairies](https://github.com/Edkabe/TP1_JeuVideo/issues/2).

### Étape 1 — Architecture et fondations du moteur

- Finaliser le diagramme de classes à partir des responsabilités réellement demandées.
- Adapter `Engine` à une seule fenêtre; démarrer seulement si l’initialisation réussit et si le moteur n’est pas déjà actif.
- Garder une boucle claire : événements, mise à jour, rendu; libérer les ressources graphiques avant le contexte.
- Mettre en place les types utilitaires nécessaires : vecteur, matrice, couleur.
- Séparer les abstractions `Shader` et `Program`; prendre en charge le chargement, la compilation, la liaison, l’utilisation et la libération.
- Encapsuler les tampons et le tableau de sommets requis pour le rendu.

**Issues liées :** [#1 — Diagramme de classe](https://github.com/Edkabe/TP1_JeuVideo/issues/1), [#6 — Moteur mono-fenêtre et boucle](https://github.com/Edkabe/TP1_JeuVideo/issues/6), [#10 — Shader et Program](https://github.com/Edkabe/TP1_JeuVideo/issues/10), [#11 — Tampons et objets de sommets](https://github.com/Edkabe/TP1_JeuVideo/issues/11), [#12 — Vecteurs, matrices et couleurs](https://github.com/Edkabe/TP1_JeuVideo/issues/12).

### Étape 2 — Paramètres, ressources, scènes et événements

- Charger les paramètres au démarrage; créer des valeurs par défaut si le fichier n’existe pas; permettre leur modification et leur sauvegarde.
- Créer une abstraction de ressource et un gestionnaire qui ajoute, retrouve et retire les ressources titrées sans les charger en double.
- Définir `IScene` et le gestionnaire de scènes : une scène active, transitions, chargement à l’activation et déchargement à la désactivation.
- Donner à chaque scène son gestionnaire d’événements; permettre l’ajout, le retrait et la notification des observateurs.

**Issues liées :** [#3 — Gestionnaire de ressources](https://github.com/Edkabe/TP1_JeuVideo/issues/3), [#4 — Gestionnaire d’événements](https://github.com/Edkabe/TP1_JeuVideo/issues/4), [#7 — Paramètres](https://github.com/Edkabe/TP1_JeuVideo/issues/7), [#8 — Gestionnaire de scènes](https://github.com/Edkabe/TP1_JeuVideo/issues/8), [#9 — Événements dans les scènes](https://github.com/Edkabe/TP1_JeuVideo/issues/9).

### Étape 3 — Ressources graphiques et contrôles

Construire les contrôles au-dessus des ressources et des fondations de l’étape 1, sans dupliquer la gestion des événements ou du rendu :

- texture et image, avec redimensionnement avec ou sans conservation du rapport d’aspect;
- police et étiquette, avec modification du texte;
- case à cocher et bouton interactifs;
- barre de progression;
- sélecteur;
- glisseur avec gestion du clic maintenu et du relâchement.

**Issues liées :** [#5 — Case à cocher](https://github.com/Edkabe/TP1_JeuVideo/issues/5), [#13 — Texture et Image](https://github.com/Edkabe/TP1_JeuVideo/issues/13), [#14 — Police et Étiquette](https://github.com/Edkabe/TP1_JeuVideo/issues/14), [#15 — Bouton](https://github.com/Edkabe/TP1_JeuVideo/issues/15), [#16 — Barre de progression](https://github.com/Edkabe/TP1_JeuVideo/issues/16), [#17 — Sélecteur](https://github.com/Edkabe/TP1_JeuVideo/issues/17), [#18 — Glisseur](https://github.com/Edkabe/TP1_JeuVideo/issues/18).

### Étape 4 — Programmes binaires et scènes de l’application

- Charger les programmes graphiques depuis un fichier binaire quand c’est possible; sinon les lier et sauvegarder leur binaire dans le format défini par les paramètres du moteur.
- Créer la scène principale, la scène des paramètres et la scène des crédits avec les contrôles demandés.
- Relier les actions aux transitions, à l’application/sauvegarde des paramètres et à la fermeture du programme.

**Issues liées :** [#19 — Scène principale](https://github.com/Edkabe/TP1_JeuVideo/issues/19), [#20 — Scène Paramètres](https://github.com/Edkabe/TP1_JeuVideo/issues/20), [#21 — Scène Crédits](https://github.com/Edkabe/TP1_JeuVideo/issues/21), [#22 — Cache binaire des programmes](https://github.com/Edkabe/TP1_JeuVideo/issues/22), [#23 — Application des paramètres](https://github.com/Edkabe/TP1_JeuVideo/issues/23).

### Étape 5 — Intégration, documentation et remise

- Parcourir les scènes et les contrôles; vérifier les transitions répétées et les cycles de chargement/déchargement.
- Vérifier les paramètres avec et sans fichier existant, puis après redémarrage.
- Vérifier la libération des ressources et la fermeture propre du moteur.
- Finaliser diagramme, documentation des fichiers/classes/données/méthodes et méthodologie des sprints/issues/révisions/contributions.

**Issues liées :** [#24 — Intégration et cycle des ressources](https://github.com/Edkabe/TP1_JeuVideo/issues/24), ainsi que [#1 — Diagramme de classe](https://github.com/Edkabe/TP1_JeuVideo/issues/1) et [#2 — Librairies et compilation](https://github.com/Edkabe/TP1_JeuVideo/issues/2).

## Répartition des tâches selon les issues existantes

Les responsabilités ci-dessous reprennent les indications présentes dans les issues GitHub. Une issue sans responsable nommé reste à attribuer ensemble; je n’invente pas une attribution. Le travail commun doit être intégré et compris par les deux personnes, même lorsqu’une issue a un responsable principal.

| Membre | Issues attribuées dans les issues existantes |
| --- | --- |
| **Edkabe** | [#6 Moteur mono-fenêtre](https://github.com/Edkabe/TP1_JeuVideo/issues/6), [#9 Événements des scènes](https://github.com/Edkabe/TP1_JeuVideo/issues/9), [#10 Shader et Program](https://github.com/Edkabe/TP1_JeuVideo/issues/10), [#13 Image/Texture](https://github.com/Edkabe/TP1_JeuVideo/issues/13), [#15 Bouton](https://github.com/Edkabe/TP1_JeuVideo/issues/15), [#16 Barre de progression](https://github.com/Edkabe/TP1_JeuVideo/issues/16), [#17 Sélecteur](https://github.com/Edkabe/TP1_JeuVideo/issues/17), [#19 Scène principale](https://github.com/Edkabe/TP1_JeuVideo/issues/19), [#21 Crédits](https://github.com/Edkabe/TP1_JeuVideo/issues/21), [#22 Programmes binaires](https://github.com/Edkabe/TP1_JeuVideo/issues/22). |
| **oceane-talla** | [#7 Paramètres](https://github.com/Edkabe/TP1_JeuVideo/issues/7), [#8 Scènes](https://github.com/Edkabe/TP1_JeuVideo/issues/8), [#11 Tampons et objets de sommets](https://github.com/Edkabe/TP1_JeuVideo/issues/11), [#12 Mathématiques](https://github.com/Edkabe/TP1_JeuVideo/issues/12), [#14 Étiquette/Font](https://github.com/Edkabe/TP1_JeuVideo/issues/14), [#18 Glisseur](https://github.com/Edkabe/TP1_JeuVideo/issues/18), [#20 Scène Paramètres](https://github.com/Edkabe/TP1_JeuVideo/issues/20), [#23 Application des paramètres](https://github.com/Edkabe/TP1_JeuVideo/issues/23), [#24 Intégration](https://github.com/Edkabe/TP1_JeuVideo/issues/24). |
| **À répartir ensemble** | [#1 Diagramme](https://github.com/Edkabe/TP1_JeuVideo/issues/1); [#2 Librairies/compilation](https://github.com/Edkabe/TP1_JeuVideo/issues/2); [#3 Ressources](https://github.com/Edkabe/TP1_JeuVideo/issues/3); [#4 Gestionnaire d’événements](https://github.com/Edkabe/TP1_JeuVideo/issues/4); [#5 Case à cocher](https://github.com/Edkabe/TP1_JeuVideo/issues/5), dont l’issue laisse la responsabilité individuelle à décider selon les modalités du professeur. |

## Dépendances entre étapes

```text
Atelier validé
    ↓
Moteur mono-fenêtre + rendu de base + diagramme
    ↓
Paramètres + ressources + scènes + événements
    ↓
Contrôles visuels
    ↓
Trois scènes + programmes binaires
    ↓
Intégration, documentation et remise
```

Les contrôles dépendent du rendu et des ressources; les scènes dépendent des contrôles, des paramètres et du gestionnaire de scènes. On ne commence donc pas les scènes finales avant d’avoir validé leurs fondations.

## Point de validation avant tout code

Avant de commencer l’étape suivante, les deux membres devraient pouvoir expliquer avec leurs mots :

1. quelles parties de l’atelier seront conservées ou adaptées;
2. pourquoi le moteur doit passer de plusieurs fenêtres à une seule;
3. ce que chaque étape apporte et de quelles étapes elle dépend;
4. qui prend en charge chaque issue, notamment les issues sans responsable nommé;
5. ce qui est explicitement hors portée pour respecter « seulement le strict nécessaire ».

**Prochaine étape proposée :** relire ce plan ensemble, décider de la répartition des issues #2 à #5, puis valider une copie de travail propre de l’atelier avant toute modification de code.

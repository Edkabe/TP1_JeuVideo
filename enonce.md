Seul le strict nécessaire demandé devra être implémenté et seules les notions couvertes devront être utilisées.

1. Moteur
Concevoir et implémenter une bibliothèque orientée objet d'un moteur graphique mono fenêtre qui sera utilisée pour réaliser les contrôles visuels et les interfaces graphiques.

Paramètres chargés, à partir d'un fichier, lors de l'instanciation, ou créés si inexistants.
Obtention des différents paramètres actuels, et sauvegarde des modifications.
Ensemble de classes représentant les entités nécessaires, comme les nuanceurs, programme, tampons de données, vecteur, matrice, etc., et permettant de les utiliser convivialement.
Démarrage du moteur (boucle de jeu) seulement si possible et pas déjà en activité.
1.1. Gestionnaire de ressources
Les ressources, comme les textures, les polices de caractères, etc., devront être représentées.

Méthodes utilitaires.
Opérateur de transtypage.
Elles seront stockées dans un gestionnaire afin de pouvoir être réutilisées.

Ajout, obtention, et retrait de ressources titrées.
1.2. Gestionnaire de scènes
Les affichages, comme les interfaces graphiques, seront représentés sous forme de scènes.

Chargement du contenu uniquement à l'activation, et déchargement lors de la désactivation.
Étapes d'une itération de la boucle de jeu.
Elles seront stockées dans un gestionnaire afin de pouvoir transiter d'une à l'autre.

Ajout et retrait de scènes titrées.
Transition entre deux scènes.
1.2.1. Gestionnaire d'événements
Chaque scène comporte son gestionnaire d'événements.

Ajout et retrait d'observateur d'événement.
Notification des observateurs d'événement.
2. Contrôles visuels
Concevoir et implémenter des contrôles visuels qui seront utilisés pour les interfaces graphiques.

2.1. Image
Permets d'afficher une image.

Position.
Dimensions.
Redimensionnement, avec et sans respect de son aspect.
2.2. Étiquette
Permets d'afficher une ligne de texte.

Police de caractères.
Couleur.
Position.
Modification du texte.
2.3. Barre de progression
Permets d'afficher une barre de progression.

Position.
Dimensions.
Valeur minimum, actuelle, et maximum.
Couleur de bordure, d'arrière-plan, et de progression.
Progression.
2.4. Case à cocher
Permets d'afficher une case à cocher carrée.

Position.
Taille.
Couleur de bordure, d'arrière-plan, et de coche.
Obtention de l'état.
Coche ou décoche lors d'un clic avec le bouton de gauche de la souris.
2.5. Bouton
Permets d'afficher un bouton titré.

Position.
Dimensions.
Couleur de bordure, d'arrière-plan, et de survol.
Police de caractères et couleur de texte.
Étiquette de titre centrée.
Change de couleur d'arrière-plan lors d'un survol avec le curseur de la souris.
Déclenche un événement lors d'un clic avec le bouton de gauche de la souris.
2.6. Sélecteur
Permets de sélectionner un élément parmi une liste.

Position.
Dimensions.
Étiquette d'élément sélectionné centrée.
Boutons précédent, à gauche, et suivant, à droite, pour naviguer parmi la liste d'éléments.
2.7. Glisseur
Permets de sélectionner une valeur avec un glisseur.

Position.
Dimensions.
Valeur minimum, actuelle, maximum, et d'incrément.
Obtention de la valeur.
Couleur de bordure, d'arrière-plan, et de progression.
Glisseur change de couleur d'arrière-plan lors d'un survol avec le curseur de la souris.
Glisseur se déplace lors d'un clic et du maintient du bouton de gauche de la souris sur celui-ci.
3. Interfaces graphiques
Implémenter les interfaces graphiques suivantes. Les programmes utilisés devraient être chargé à partir d'un fichier binaire, sinon être lié et sauvegardé dans dans un fichier binaire. (Le format des fichiers binaires doit être conservé dans les paramètres du moteur graphique.)

3.1. Principale
Scène principale.

Image d'arrière-plan.
Étiquette de titre.
Bouton pour démarrer le jeu.
Bouton pour passer à la scène des paramètres.
Bouton pour passer à la scène des crédits.
Bouton pour quitter le programme.
3.2. Paramètres
Scène permettant de paramétrer le jeu.

Image d'arrière-plan.
Étiquettes de titre.
Sélecteur de langage.
Sélecteur de résolution.
Case à cocher de plein écran.
Case à cocher de synchronisation verticale.
Glisseur de sensibilité de la souris.
Glisseur d'angle de champ de vision.
Bouton pour appliquer (et sauvegarder) les changements.
Bouton pour revenir à la scène principale.
3.3. Crédits
Scène permettant de présenter le projet et ses auteurs.

À votre guise.
4. Barème
Les deux dernières sections sont une évaluation individuelle, tandis que les autres sont une évaluation d'équipe.

 Normes	 / 4
 Programmation	 / 2
 Messages	 / 2
 Diagramme de classes	 / 4
 Classes	 / 1
 Relations	 / 1
 Données membres et Méthodes	 / 2
 Texture	 / 6
 Allocation	 / 1
 Affectation	 / 1
 Paramétrage	 / 2
 Engagement	 / 1
 Libération	 / 1
 Police de caractères	 / 3
 Chargement	 / 1
 Génération de texture	 / 1
 Libération	 / 1
 Conteneur de ressources	 / 3
 Abstraction de ressource	 / 1
 Ajout et Retrait	 / 1
 Obtention	 / 1
 Gestionnaire de scènes	 / 5
 Abstraction de scène	 / 1
 Ajout et Retrait	 / 1
 Chargement et Déchargement (Patron « Lazy loading »)	 / 1
 Transitions	 / 2
 Gestionnaire d'événements	 / 3
 Patron Observateur	 / 1
 Attachement et Détachement	 / 1
 Notification	 / 1
 Nuanceurs	 / 6
 Abstraction	 / 1
 Instanciation	 / 1
 Chargement	 / 1
 Compilation	 / 1
 Attribut	 / 1
 Libération	 / 1
 Tampons	 / 5
 Abstraction	 / 1
 Instanciation	 / 1
 Affectation	 / 1
 Engagement	 / 1
 Libération	 / 1
 Tableau de sommet	 / 4
 Allocation	 / 1
 Attribut	 / 1
 Engagement	 / 1
 Libération	 / 1
 Programme	 / 8
 Allocation	 / 1
 Attachement	 / 1
 Obtention de location	 / 1
 Liaison	 / 1
 Sauvegarde et Chargement	 / 2
 Utilisation	 / 1
 Libération	 / 1
 Matrice	 / 3
 Donnée membres	 / 1
 Chargement de la matrice Orthogonale	 / 1
 Opérateur de transtypage	 / 1
 Moteur	 / 7
 Patron Singleton	 / 1
 Chargement et Sauvegarde	 / 3
 Boucle de jeu	 / 3
 Composant visuel	 / 3
 Abstraction	 / 1
 Allocation	 / 1
 Libération	 / 1
 Image	 / 3
 Allocation	 / 1
 Redimensionnement	 / 1
 Rendu	 / 1
 Étiquette	 / 5
 Allocation	 / 1
 Modification du texte	 / 2
 Rendu	 / 1
 Libération	 / 1
 Barre de progression	 / 4
 Instanciation	 / 1
 Gestion de la progression	 / 1
 Rendu	 / 1
 Libération	 / 1
 Case à cocher	 / 5
 Instanciation	 / 1
 Événement et État	 / 2
 Rendu	 / 1
 Libération	 / 1
 Bouton	 / 6
 Instanciation	 / 1
 Événements et Action	 / 3
 Rendu	 / 1
 Libération	 / 1
 Sélecteur	 / 6
 Instanciation	 / 1
 Événements et État	 / 2
 Sélection	 / 1
 Rendu	 / 1
 Libération	 / 1
 Glisseur	 / 6
 Instanciation	 / 1
 Événements et Gestion du curseur	 / 3
 Rendu	 / 1
 Libération	 / 1
 Scène principale	 / 4
 Chargement	 / 1
 Rendu	 / 1
 Transitions	 / 1
 Déchargement	 / 1
 Scène de paramètres	 / 5
 Chargement	 / 1
 Rendu	 / 1
 Application	 / 1
 Transition	 / 1
 Déchargement	 / 1
 Scène de crédits	 / 4
 Chargement	 / 1
 Rendu	 / 1
 Transition	 / 1
 Déchargement	 / 1
 Documentation	 / 4
 Fichiers	 / 1
 Classes	 / 1
 Données membres	 / 1
 Méthodes	 / 1
 Méthodologie	 / 4
 Sprints	 / 1
 Tâches	 / 1
 Révisions	 / 1
 Gestion des versions	 / 1
 Contribution	 / 27
 Mêlés	 / 6
 Poids	 / 6
 Implémentations et Soumissions	 / 6
 Révisions	 / 6
 Évaluations	 / 3
 Comportement	 / 24
 Communication	 / 6
 Respect	 / 6
 Rigueur	 / 6
 Adaptation	 / 6
Évaluation du comportement.docx Évaluation du comportement.docx4 octobre 2026, 11:23

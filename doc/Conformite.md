# Correspondance avec l’énoncé et le barème

Cette grille reprend les critères remis dans l’énoncé. Elle sert de repère de revue, pas de prédiction de note : la présence d’une classe ou d’un test ne prouve pas à elle seule que le critère recevra tous ses points. Les critères individuels et l’évaluation finale restent du ressort de l’enseignant.

Les changements de cette branche ajoutent des vérifications ciblées pour les erreurs de scène/rendu, l’arrêt après échec de sauvegarde, l’identité des ressources partagées, l’isolation des chemins de paramètres et le retrait des observateurs. Les résultats automatisés sont consignés dans la PR. Le parcours manuel reproduit localement le 30 octobre 2026 est décrit dans Integration.md et reste à confirmer sur le poste de remise.

## Équipe — code et documentation

| Critère du barème | Fichiers / preuve à examiner | Validation à la remise |
| --- | --- | --- |
| Normes | Tous les fichiers `include/moteur/` et `src/moteur/` | Revue des conventions demandées par le cours |
| Programmation | Sources C++ et script `scripts/compiler-windows.ps1` | Compilation propre dans l’environnement de remise |
| Messages | `src/main.cpp`, méthodes `getError()`, journal SDL | Examiner les messages visibles et les diagnostics d’échec |
| Diagramme — Classes | [`Classes.md`](Classes.md) | Diagramme courant; `Classes-initiale.uxf` est le modèle antérieur |
| Diagramme — Relations | [`Classes.md`](Classes.md) | Vérifier multiplicités, composition, références partagées et observation |
| Diagramme — Données membres et Méthodes | [`Classes.md`](Classes.md), en-têtes `include/moteur/` | Comparer chaque signature et membre aux déclarations actuelles |
| Texture — Allocation | `src/moteur/Texture.cpp` | Test graphique et inspection de `glGenTextures` |
| Texture — Affectation | `src/moteur/Texture.cpp` | Chargement d’image et transfert des pixels |
| Texture — Paramétrage | `Texture::setFiltering`, `Texture::setWrapping` | Test des valeurs valides/invalides et état OpenGL |
| Texture — Engagement | `Texture::bind`, composants de rendu | Démonstration de liaison et utilisation |
| Texture — Libération | `Texture::release`, destructeur, `ResourceManager` | Vérifier avec un contexte courant et après transitions |
| Police — Chargement | `src/moteur/Font.cpp` | Charger une vraie police disponible sur le poste |
| Police — Génération de texture | `Font::createTextTexture` | Test graphique de génération et remplacement |
| Police — Libération | `Font::release`, destructeur | Vérifier fermeture et coexistence de polices |
| Conteneur de ressources — Abstraction de ressource | `Resource`, `Texture`, `Font` | Vérifier abstraction et transtypages |
| Conteneur de ressources — Ajout et Retrait | `ResourceManager::add/remove/clear` | Tests des doublons, aliases et retrait |
| Conteneur de ressources — Obtention | `ResourceManager::get<T>` | Test de type et identité du `shared_ptr` obtenu |
| Gestionnaire de scènes — Abstraction de scène | `IScene` | Vérifier contrat virtuel et diagnostics |
| Gestionnaire de scènes — Ajout et Retrait | `SceneManager::addScene/removeScene` | Tests des noms invalides, doublons et retrait actif |
| Gestionnaire de scènes — Chargement et Déchargement (Lazy loading) | `SceneManager::setActiveScene`, scènes concrètes | Tests de chargement à l’activation et déchargement |
| Gestionnaire de scènes — Transitions | `SceneManager::setActiveScene`, `SceneManagerTests` | Vérifier destination chargée avant départ; échec conserve l’ancienne scène |
| Gestionnaire d’événements — Patron Observateur | `EventManager`, `EventObserver` | Inspection des références faibles et notifications |
| Gestionnaire d’événements — Attachement et Détachement | `EventManager`, `unload()` des scènes | Test d’observateur absent après transition et échec partiel |
| Gestionnaire d’événements — Notification | `EventManager::notifyObservers` | Vérifier filtrage par type et inscriptions retirées pendant notification |
| Nuanceurs — Abstraction | `Shader`, `VertexShader` | Revue des interfaces et spécialisations |
| Nuanceurs — Instanciation | `Shader`, `Program` | Vérifier création avec contexte OpenGL |
| Nuanceurs — Chargement | `Shader::loadShader` | Essayer fichiers existants et absents |
| Nuanceurs — Compilation | `Shader::compile` | Compilation et diagnostic d’un nuanceur invalide |
| Nuanceurs — Attribut | `Program::getAttributeLocation` | Vérifier location valide et noms absents |
| Nuanceurs — Libération | `Shader::release` | Vérifier suppression avec contexte valide |
| Tampons — Abstraction | `Buffer` | Revue du contrat virtuel |
| Tampons — Instanciation | `VBO` | Test graphique de création |
| Tampons — Affectation | `VBO::setData` | Vérifier contenu et taille envoyés |
| Tampons — Engagement | `VBO::bind` et rendu des contrôles | Vérifier utilisation OpenGL |
| Tampons — Libération | `VBO::release` | Vérifier suppression et état après libération |
| Tableau de sommet — Allocation | `VAO::create` | Test graphique de création |
| Tableau de sommet — Attribut | `VAO::setAttribute` | Vérifier configuration d’attributs et VBO lié |
| Tableau de sommet — Engagement | `VAO::bind` et appels de rendu | Démonstration du rendu avec le VAO |
| Tableau de sommet — Libération | `VAO::release` | Vérifier suppression avec contexte valide |
| Programme — Allocation | `Program::create/link` | Revue des échecs de création et liaison |
| Programme — Attachement | `Program::attachShader` | Vérifier attachement des shaders requis |
| Programme — Obtention de location | `Program::getUniformLocation/getAttributeLocation` | Tester locations existantes et absentes |
| Programme — Liaison | `Program::link` | Vérifier lien réussi et diagnostic |
| Programme — Sauvegarde et Chargement | `Program::getBinary/loadBinary`, `ProgramCache` | Tester binaire valide, format incompatible et repli source |
| Programme — Utilisation | `Program::use`, contrôles | Vérifier activation pour dessin |
| Programme — Libération | `Program::release` | Vérifier libération avant fermeture du contexte |
| Matrice — Données membres | `Matrix4` | Comparer stockage et opérations aux signatures du diagramme |
| Matrice — Chargement de la matrice orthogonale | `Matrix4::orthographic` | Tester les bornes et l’effet sur les coordonnées |
| Matrice — Opérateur de transtypage | `Matrix4::operator const float*` | Vérifier pointeur et durée de vie |
| Moteur — Patron Singleton | `Engine::getInstance` | Vérifier instance unique et non copiable |
| Moteur — Chargement et Sauvegarde | `Settings`, `Engine::shutdown` | Tests binaires, fichier temporaire, remplacement et échec d’écriture |
| Moteur — Boucle de jeu | `Engine::run` | Vérifier démarrage, fermeture normale et propagation d’erreur |
| Composant visuel — Abstraction | `VisualComponent` | Vérifier interface commune aux sept contrôles |
| Composant visuel — Allocation | Constructeurs et `initialize()` des contrôles | Vérifier ressources GPU et erreurs de création |
| Composant visuel — Libération | `release()` des contrôles | Vérifier libération répétable avant fermeture du contexte |
| Image — Allocation | `Image` | Vérifier création et partage de texture |
| Image — Redimensionnement | `Image::resize` | Tester respect et absence de respect des proportions |
| Image — Rendu | `Image::render` | Test graphique et restauration des états OpenGL |
| Étiquette — Allocation | `Label` | Vérifier police, texture et ressources de dessin |
| Étiquette — Modification du texte (2 points) | `Label::setText`, `Font::createTextTexture` | Tester texte valide, vide, UTF-8 et échec de génération |
| Étiquette — Rendu | `Label::render` | Test graphique; vérifier remontée de l’erreur OpenGL |
| Étiquette — Libération | `Label::release` | Vérifier répétabilité et contexte courant |
| Barre de progression — Instanciation | `ProgressBar` | Vérifier valeurs et composants internes |
| Barre de progression — Gestion de la progression | `setRange`, `setValue`, `getProgress` | Tests min/max et bornes |
| Barre de progression — Rendu | `ProgressBar::render` | Test graphique et états restaurés |
| Barre de progression — Libération | `ProgressBar::release` | Vérifier libération |
| Case à cocher — Instanciation | `CheckBox::initialize` | Test d’initialisation et reprise après `release()` |
| Case à cocher — Événement et état (2 points) | `CheckBox::onEvent`, `isChecked` | Tester clic gauche, droit et hors zone |
| Case à cocher — Rendu | `CheckBox::render` | Vérifier état visible et bindings restaurés |
| Case à cocher — Libération | `CheckBox::release` | Vérifier libération répétable |
| Bouton — Instanciation | `Button` | Vérifier titre, disposition et composants internes |
| Bouton — Événements et Action (3 points) | `Button::onEvent`, `setOnClick` | Tester survol, appui, relâchement et action |
| Bouton — Rendu | `Button::render` | Vérifier états visuels et OpenGL restaurés |
| Bouton — Libération | `Button::release` | Vérifier libération et réinitialisation |
| Sélecteur — Instanciation | `Selector` | Vérifier éléments, titre centré et deux boutons |
| Sélecteur — Événements et État (2 points) | `Selector::onEvent` | Tester clics et limites de liste |
| Sélecteur — Sélection | `setSelectedIndex`, `previous`, `next` | Vérifier index et texte sélectionné |
| Sélecteur — Rendu | `Selector::render` | Vérifier rendu et états restaurés |
| Sélecteur — Libération | `Selector::release` | Vérifier libération de ses composants |
| Glisseur — Instanciation | `Slider` | Vérifier min, max, valeur et incrément |
| Glisseur — Événements et Gestion du curseur (3 points) | `Slider::onEvent` | Tester survol, clic, maintien et déplacement |
| Glisseur — Rendu | `Slider::render` | Vérifier rendu et états restaurés |
| Glisseur — Libération | `Slider::release` | Vérifier libération |
| Scène principale — Chargement | `MainScene::load` | Vérifier ressources et quatre boutons prêts |
| Scène principale — Rendu | `MainScene::render` | Test d’intégration graphique |
| Scène principale — Transitions | `MainScene::navigate` | Tester jeu, paramètres, crédits et quitter |
| Scène principale — Déchargement | `MainScene::unload` | Vérifier observateurs détachés et composants libérés |
| Scène de paramètres — Chargement | `SettingsScene::load` | Vérifier ressources et contrôles prêts avant succès |
| Scène de paramètres — Rendu | `SettingsScene::render` | Test d’intégration graphique |
| Scène de paramètres — Application | `SettingsScene::applyChanges`, `Engine::commitSettings` | Vérifier validation, application fenêtre, sauvegarde et retour arrière |
| Scène de paramètres — Transition | `SettingsScene::returnToMenu` | Vérifier retour après chargement de destination |
| Scène de paramètres — Déchargement | `SettingsScene::unload` | Vérifier désabonnement après succès et échec partiel |
| Scène de crédits — Chargement | `CreditsScene::load` | Vérifier police et étiquettes disponibles |
| Scène de crédits — Rendu | `CreditsScene::render` | Test d’intégration graphique |
| Scène de crédits — Transition | `CreditsScene::returnToMenu` / bouton Retour | Vérifier retour au menu |
| Scène de crédits — Déchargement | `CreditsScene::unload` | Vérifier détachement et libération |
| Documentation — Fichiers | `doc/`, `README.md`, `PlanTP1.md` | Vérifier liens, procédures et diagramme courant |
| Documentation — Classes | [`Classes.md`](Classes.md) | Comparer le diagramme au code actuel |
| Documentation — Données membres | [`Classes.md`](Classes.md) et en-têtes | Comparaison déclaration par déclaration |
| Documentation — Méthodes | [`Classes.md`](Classes.md) et en-têtes | Comparaison signature par signature |
| Méthodologie — Sprints | Historique de dépôt et documents de cours | Évaluation par l’enseignant |
| Méthodologie — Tâches | Issues, commits et documents de cours | Évaluation par l’enseignant |
| Méthodologie — Révisions | PR et commentaires de revue | Évaluation par l’enseignant |
| Gestion des versions | Historique Git, branches, PR | Vérifier contributions et intégration réelle |

## Évaluation individuelle — non déductible du seul code

| Critère du barème | Preuve à fournir / vérifier |
| --- | --- |
| Contribution — Mêlés | Historique des fusions et attribution des auteurs |
| Contribution — Poids | Répartition réelle du travail dans l’équipe |
| Contribution — Implémentations et Soumissions | Soumissions et commits attribués à chaque personne |
| Contribution — Révisions | Revues effectuées par chaque personne |
| Contribution — Évaluations | Évaluations individuelles prévues par le cours |
| Comportement — Communication | Évaluation de l’équipe par l’enseignant |
| Comportement — Respect | Évaluation de l’équipe par l’enseignant |
| Comportement — Rigueur | Évaluation de l’équipe par l’enseignant |
| Comportement — Adaptation | Évaluation de l’équipe par l’enseignant |

## Résultats reproduits et résultats rapportés

Les lignes ci-dessus indiquent où examiner les critères; elles ne certifient pas tous les scénarios sur chaque machine. Les tests automatisés exécutés pour cette PR sont indiqués dans sa description. Le parcours manuel consigné dans [`Integration.md`](Integration.md) a été reproduit localement le 30 octobre 2026 et doit être confirmé sur l’ordinateur utilisé pour la remise. Les modes plein écran dépendent des résolutions exposées par l’écran.

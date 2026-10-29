# TP1_JeuVideo

## 📦 Atelier 2 — téléchargement

**[⬇ Télécharger Atelier2JV.zip (175 Mo)](https://github.com/Edkabe/TP1_JeuVideo/releases/download/atelier2-reference/Atelier2JV.zip)**

Archive originale de l'atelier réalisé en cours, fournie comme référence distincte du projet. Connectez-vous à GitHub avec un compte ayant accès à ce dépôt privé pour la télécharger.

---

Projet de moteur graphique mono fenêtre et de contrôles visuels.

Équipe : Edkabe et oceane-talla.

Les tâches sont suivies dans les issues et les jalons Sprint 1, Sprint 2 et Sprint 3. Une branche est dédiée à chaque tâche des sprints 2 et 3.

## Référence de cours

Atelier2JV.zip est un atelier distinct du projet. Son archive originale est destinée à la section Releases afin de pouvoir être téléchargée par l’équipe.

## Compilation et bibliothèques

Voir [le guide de compilation incrémentale](doc/Bibliotheques.md).
Le moteur se compile dans une bibliothèque statique indépendante du `main`.
Les dépendances déjà fournies sont réutilisées, sans vcpkg.

## Menu principal

Apres compilation, lancer `build/program.exe` pour ouvrir le menu, la scene de demarrage avec barre de progression, les parametres ou les credits. Les menus et ecrans sont disponibles en français et en anglais. Les parametres de sensibilité et de champ de vision sont sauvegardés pour le jeu.
Voir [les commandes et le fonctionnement](doc/MenuPrincipal.md) ainsi que le [guide de validation](doc/Integration.md). Le triangle reste accessible avec `--triangle` et son test avec `--smoke`.

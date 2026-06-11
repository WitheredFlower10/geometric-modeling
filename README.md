# Projet de Geometry Modelling 3D

Ce dépôt contient l'ensemble des travaux pratiques et des fonctionnalités obligatoires demandées dans le cadre du cours.

---

## Checklist des Fonctionnalités Implémentées

### 1. Lecture de Fichier (`readFile`)
- **Description** : Module d'importation de fichiers de maillage 3D (formats `.obj`).
- **Statut** : 🟩 Terminé
- **Détails** : Récupère les lignes de sommets (v) et de faces (f) pour construire notre structure de demi-arêtes.
- **Captures** : ![](screenshots/readFileDolphin.png)

### 2. Calcul des Normales (`compute normals`)
- **Description** : Calcul des normales des faces et des sommets.
- **Statut** : 🟩 Terminé
- **Détails** : Permet au visualisateur d'afficher correctement les ombres et les reflets de lumière sur l'objet.
- **Captures** : ![](screenshots/normals.png)
![](screenshots/normals2.png)

### 3. Détection de Silhouette (`Silhouette`)
- **Description** : Dessin des contours extérieurs du modèle selon l'angle de la caméra.
- **Statut** : 🟩 Terminé
- **Détails** : Repose sur le test du produit scalaire entre le vecteur de vue et la normale des faces adjacentes à une arête ($\vec{n}_1 \cdot \vec{v} \times \vec{n}_2 \cdot \vec{v} < 0$).
- **Captures** : ![](screenshots/silhouette.png)

### 4. Triangulation de Polygones (`triangulation`)
- **Description** : Algorithme de découpage des faces polygonales complexes en triangles simples.
- **Statut** : 🟩 Terminé (2 Niveaux)
- **Détails** : 
  - *Basique* : Gestion des faces convexes par fan-triangulation. (Ne marche pas avec les surfaces concaves en testant avec `gear.obj`)
  ![](screenshots/gear.png)
  - *Avancé* : Gestion des faces concaves (via l'algorithme des oreilles / *Ear Clipping*).
- **Captures** :  ![](screenshots/earClipping.png)
 ![](screenshots/triangulation.png)

### 5. Tests de Structure Half-Edge (`half-edge data structure tests`)
- **Description** : Suite de tests unitaires et de cohérence topologique intégrée via la fonction `checkMesh()`.
- **Statut** : 🟩 Terminé
- **Détails** : Des boucles testent si tous les `twin` se répondent bien, si les cycles `next/prev` tournent rond, et si la formule d'`Euler` est respectée pour être sûr qu'il n'y a pas de bug topologique. L'implémentation a été organisée à raison d'**une fonction par test** à partir de la structure initiale fournie dans le squelette de code de `checkMesh()` par le professeur.
- **Captures** : ![](screenshots/tests.png)

### 6. Surface de Révolution (`surface of revolution`)
- **Description** : Génération d'un maillage 3D complet (Quads) à partir d'un profil de points 2D pivotant autour de l'axe vertical.
- **Statut** : 🟩 Terminé
- **Détails** : Génère un pion d'échecs en faisant tourner une suite de points autour de l'axe vertical, en reliant bien les arêtes jumeaux (twin) pour fermer la forme. (profile généré par Gemini).
- **Captures** : ![](screenshots/surfaceRevolution.png)

### 7. Simplification de Maillage (`mesh simplification via shortest edge collapse`)
- **Description** : Allégement du modèle en supprimant des arêtes.
- **Statut** : 🟩 Terminé
- **Détails** : L'algorithme cherche l'arête la plus courte du maillage entier et la fusionne en un seul sommet, puis nettoie les faces écrasées et reconnecte proprement les voisins.
- **Captures** : ![](screenshots/simplification.png)

### 8. Subdivision de Catmull-Clark (`Catmull-Clark mesh subdivision`)
- **Description** : Algorithme de subdivision de surface pour lisser et arrondir les maillages de manière itérative.
- **Statut** : 🟩 Terminé
- **Détails** : Divise chaque face en quadrilatères et recalcule la position de tous les points (formules de Catmull-Clark).
- **Captures** : ![](screenshots/catmull.png)
![](screenshots/catmull1.png)

---

## Déclaration et Justification de l'Utilisation de l'IA (Gemini)

Le LLM **Gemini** a été utilisé comme **assistant de programmation, outil de débogage et collaborateur technique** tout au long du développement de ce projet.

L'usage de l'intelligence artificielle a été ciblé sur des aspects précis :

* **Structures de Données :** Une aide précieuse pour déterminer et structurer les collections de données les plus adaptées de la bibliothèque standard C++ (`std::map`, `std::vector`, paires de clés pour les dictionnaires, etc.).
* **Génération de Profils Géométriques :** L'IA a été sollicitée pour générer les coordonnées mathématiques initiales (le tableau de points 2D) décrivant le profil du pion d'échecs utilisé dans la fonction `surfaceRevolution`.
* **Reverse Engineering et Débogage (Le cas du `readFile`) :** Lors des premières phases, le chargement des fichiers `.obj` fonctionnait, mais le maillage se brisait (bugs graphiques) dès que la fonction `triangulate` était appelée. Pour résoudre ce problème, l'IA a été utilisée pour analyser et comparer la logique de mon `readFile` avec celle d'un projet étudiant de la promotion trouvé sur GitHub. Cette comparaison guidée par le LLM a permis d'identifier les incohérences dans ma version et de corriger l'algorithme de lecture au fur et à mesure.
* **Gestion des Erreurs et Revue de Code :** Une fois mes fonctions écrites et implémentées, le code lui a été soumis systématiquement pour une phase de relecture ("Code Review"). Cela a permis d'obtenir des conseils d'optimisation, de sécuriser les cas limites (bords ouverts) et d'ajouter une gestion d'erreurs robuste pour éviter les plantages ou les fuites de mémoire.

---

### Traçabilité et Rigueur Académique

* **Balises de traçabilité :** Tous les blocs de code ayant requis une assistance forte, une correction algorithmique majeure ou une formule mathématique soufflée par l'IA sont explicitement documentés et identifiables dans le code source par la mention : 
  `// [NOTE ÉTUDIANT & IA]`
* **Périmètre d'exclusion :** Aucune intelligence artificielle n'a été utilisée pour le fichier **`main.cpp`** ni pour la gestion de l'interface et de la boucle d'affichage.

> **Note d'intégrité académique** : L'IA n'a jamais été utilisée pour concevoir passivement le projet à ma place. Chaque fonction générée ou optimisée a fait l'objet d'une analyse critique, d'une phase de tests rigoureuse (notamment via notre protocole applicatif `checkMesh()`) et d'une réadaptation logique pour correspondre parfaitement aux structures de données imposées par l'enseignant. L'étudiant reste le concepteur, le pilote et le garant de la validité du code final.
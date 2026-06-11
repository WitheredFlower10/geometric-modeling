#include "myVertex.h"
#include "myFace.h"
#include "myHalfedge.h"
#include "myVector3D.h"


myVertex::myVertex(void) {
  point = NULL;
  originof = NULL;
  normal = new myVector3D(1.0, 1.0, 1.0);
}

myVertex::~myVertex(void) {
  if (normal)
    delete normal;
}

// Calcule la normale du sommet en faisant la moyenne des normales des faces voisines
void myVertex::computeNormal() {
  // Si le sommet n'est relié à aucune arête, on ne peut rien calculer
    if (!originof) return;
// On crée un vecteur vide (0,0,0) pour accumuler les normales des faces
    myVector3D sum(0.0, 0.0, 0.0);
// On initialise nos pointeurs pour faire le tour des arêtes connectées à ce sommet
    myHalfedge* start = originof;
    myHalfedge* he = start;

    do {
      // Si l'arête actuelle appartient à une face valide et que cette face a une normale calculée, on l'ajoute à notre somme
        if (he->adjacent_face && he->adjacent_face->normal) {
            sum += *(he->adjacent_face->normal);
        }

        if (!he->twin) break;
        // On passe à l'arête suivante en suivant le twin et le next pour faire le tour du sommet
        he = he->twin->next;

    } while (he != start); // On s'arrête quand on a fait le tour complet et qu'on revient à l'arête de départ

    sum.normalize();
    *normal = sum;
}
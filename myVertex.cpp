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

void myVertex::computeNormal() {
    if (!originof) return;

    myVector3D sum(0.0, 0.0, 0.0);

    myHalfedge* start = originof;
    myHalfedge* he = start;

    do {
        if (he->adjacent_face && he->adjacent_face->normal) {
            sum += *(he->adjacent_face->normal);
        }

        if (!he->twin) break; // sécurité
        he = he->twin->next;

    } while (he != start);

    sum.normalize();
    *normal = sum;
}
#include "myHalfedge.h"

myHalfedge::myHalfedge(void)
{
	source = NULL; 
	adjacent_face = NULL; 
	next = NULL;  
	prev = NULL;  
	twin = NULL;  
}

// Copie les données et les liaisons d'une autre demi-arête (ie) dans celle-ci
void myHalfedge::copy(myHalfedge *ie)
{
   if (!ie) return;
    source = ie->source;
    adjacent_face = ie->adjacent_face;
    next = ie->next;
    prev = ie->prev;
    twin = ie->twin;
}

myHalfedge::~myHalfedge(void)
{
}

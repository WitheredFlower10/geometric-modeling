#include "myMesh.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <utility>

// #include <GL/glew.h>
#include "myVector3D.h"

using namespace std;

myMesh::myMesh(void) {}

myMesh::~myMesh(void) { clear(); }

void myMesh::clear() {
  for (unsigned int i = 0; i < vertices.size(); i++)
    if (vertices[i])
      delete vertices[i];
  for (unsigned int i = 0; i < halfedges.size(); i++)
    if (halfedges[i])
      delete halfedges[i];
  for (unsigned int i = 0; i < faces.size(); i++)
    if (faces[i])
      delete faces[i];

  vector<myVertex *> empty_vertices;
  vertices.swap(empty_vertices);
  vector<myHalfedge *> empty_halfedges;
  halfedges.swap(empty_halfedges);
  vector<myFace *> empty_faces;
  faces.swap(empty_faces);
}

void myMesh::checkMesh() {
  vector<myHalfedge *>::iterator it;
  for (it = halfedges.begin(); it != halfedges.end(); it++) {
    if ((*it)->twin == NULL)
      break;
  }
  if (it != halfedges.end())
    cout << "Error! Not all edges have their twins!\n";
  else
    cout << "Each edge has a twin!\n";
}

bool myMesh::readFile(std::string filename) {
  string s, t, u;
  vector<int> faceids;
  myHalfedge **hedges;

  ifstream fin(filename);
  if (!fin.is_open()) {
    cout << "Unable to open file!\n";
    return false;
  }
  name = filename;

  map<pair<int, int>, myHalfedge *> twin_map;
  map<pair<int, int>, myHalfedge *>::iterator it;

  while (getline(fin, s)) {
    stringstream myline(s);
    myline >> t;
    if (t == "g") {
    } else if (t == "v") {
      float x, y, z;
      myline >> x >> y >> z;
      cout << "v " << x << " " << y << " " << z << endl;
      myPoint3D *p = new myPoint3D(x, y, z);
      myVertex *v = new myVertex();
      v->point = p;
      vertices.push_back(v);
    } else if (t == "mtllib") {
    } else if (t == "usemtl") {
    } else if (t == "s") {
    } else if (t == "f") {
    vector<int> ids;

    while (myline >> u) {
        int id = atoi((u.substr(0, u.find("/"))).c_str());
        ids.push_back(id - 1);
    }
    int n = ids.size();
    myFace* face = new myFace();
    faces.push_back(face);
    vector<myHalfedge*> face_edges;
    for (int i = 0; i < n; i++) {
        myHalfedge* he = new myHalfedge();
        halfedges.push_back(he);
        face_edges.push_back(he);

        he->adjacent_face = face;
        he->source = vertices[ids[i]];
    }

    for (int i = 0; i < n; i++) {
        face_edges[i]->next = face_edges[(i + 1) % n];
        face_edges[i]->prev = face_edges[(i - 1 + n) % n];
    }

    for (int i = 0; i < n; i++) {
        int v1 = ids[i];
        int v2 = ids[(i + 1) % n];

        pair<int, int> edge = make_pair(v1, v2);
        pair<int, int> twin_edge = make_pair(v2, v1);

        myHalfedge* he = face_edges[i];

        auto it = twin_map.find(twin_edge);
        if (it != twin_map.end()) {
            he->twin = it->second;
            it->second->twin = he;
        } else {
            twin_map[edge] = he;
        }
       if (vertices[ids[i]]->originof == NULL) {
            vertices[ids[i]]->originof = he;
        }
    }
    face->adjacent_halfedge = face_edges[0];
  }
}
  checkMesh();
  normalize();
  return true;
}

void myMesh::computeNormals() { 
  for (myFace* f : faces)
      f->computeNormal();
  for (myVertex* v : vertices)
      v->computeNormal();
 }

void myMesh::normalize() {
  if (vertices.size() < 1)
    return;

  int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0,
      tmpzmax = 0;

  for (unsigned int i = 0; i < vertices.size(); i++) {
    if (vertices[i]->point->X < vertices[tmpxmin]->point->X)
      tmpxmin = i;
    if (vertices[i]->point->X > vertices[tmpxmax]->point->X)
      tmpxmax = i;

    if (vertices[i]->point->Y < vertices[tmpymin]->point->Y)
      tmpymin = i;
    if (vertices[i]->point->Y > vertices[tmpymax]->point->Y)
      tmpymax = i;

    if (vertices[i]->point->Z < vertices[tmpzmin]->point->Z)
      tmpzmin = i;
    if (vertices[i]->point->Z > vertices[tmpzmax]->point->Z)
      tmpzmax = i;
  }

  double xmin = vertices[tmpxmin]->point->X, xmax = vertices[tmpxmax]->point->X,
         ymin = vertices[tmpymin]->point->Y, ymax = vertices[tmpymax]->point->Y,
         zmin = vertices[tmpzmin]->point->Z, zmax = vertices[tmpzmax]->point->Z;

  double scale = (xmax - xmin) > (ymax - ymin) ? (xmax - xmin) : (ymax - ymin);
  scale = scale > (zmax - zmin) ? scale : (zmax - zmin);

  for (unsigned int i = 0; i < vertices.size(); i++) {
    vertices[i]->point->X -= (xmax + xmin) / 2;
    vertices[i]->point->Y -= (ymax + ymin) / 2;
    vertices[i]->point->Z -= (zmax + zmin) / 2;

    vertices[i]->point->X /= scale;
    vertices[i]->point->Y /= scale;
    vertices[i]->point->Z /= scale;
  }
}

void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p)
{
  myVertex *v_new = new myVertex();
  v_new->point = new myPoint3D(p->X, p->Y, p->Z);
  v_new->index = vertices.size();
  vertices.push_back(v_new);

  myHalfedge *e1 = f->adjacent_halfedge;
  myHalfedge *e2 = e1->next;
  myHalfedge *e3 = e2->next;

  myVertex *v1 = e1->source;
  myVertex *v2 = e2->source;
  myVertex *v3 = e3->source;
}

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p) { /**** TODO ****/ }

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p) { /**** TODO ****/ }

void myMesh::subdivisionCatmullClark() { /**** TODO ****/ }

void myMesh::simplify() { /**** TODO ****/ }

void myMesh::simplify(myVertex *) { /**** TODO ****/ }

void myMesh::triangulate() { /**** TODO ****/ }

// return false if already triangle, true othewise.
bool myMesh::triangulate(myFace *f) {
  /**** TODO ****/
  return false;
}

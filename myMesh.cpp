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

    ifstream fin(filename);
    if (!fin.is_open()) {
        cout << "Unable to open file!\n";
        return false;
    }

    name = filename;

    map<pair<int, int>, myHalfedge *> twin_map;

    while (getline(fin, s)) {
        if (s.empty() || s[0] == '#') continue;

        stringstream myline(s);
        myline >> t;
        if (t == "v") {
            float x, y, z;
            myline >> x >> y >> z;

            myPoint3D *p = new myPoint3D(x, y, z);
            myVertex *v = new myVertex();

            v->point = p;
            v->originof = NULL;

            vertices.push_back(v);
        }
        else if (t == "f") {

            vector<int> ids;

            while (myline >> u) {
                string token = u.substr(0, u.find("/"));
                int id = atoi(token.c_str());

                if (id < 0)
                    id = vertices.size() + id;
                else
                    id = id - 1;

                if (id < 0 || id >= vertices.size()) continue;

                ids.push_back(id);
            }

            int n = ids.size();
            if (n < 3) continue;

            myFace* face = new myFace();
            faces.push_back(face);

            vector<myHalfedge*> face_edges;

            for (int i = 0; i < n; i++) {
                myHalfedge* he = new myHalfedge();
                halfedges.push_back(he);

                he->adjacent_face = face;
                he->source = vertices[ids[i]];
                he->twin = NULL;

                face_edges.push_back(he);
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

                if (vertices[v1]->originof == NULL) {
                    vertices[v1]->originof = he;
                }
            }

            face->adjacent_halfedge = face_edges[0];
        }
    }

    fin.close();

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

void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p)
{
  myVertex *v_new = new myVertex();
  v_new->point = new myPoint3D(p->X, p->Y, p->Z);
  vertices.push_back(v_new);

  myHalfedge *enext = e1->next; 
  myHalfedge *etwin = e1->twin;
  myVertex *v_end = enext->source;

  myHalfedge *e_new = new myHalfedge();
  halfedges.push_back(e_new);

  e_new->source = v_new;
  e_new->next = enext;
  e_new->adjacent_face = e1->adjacent_face;
  e1->next = e_new;
  v_new->originof = e_new;
}

void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p) { /**** TODO ****/ }

void myMesh::subdivisionCatmullClark() { /**** TODO ****/ }

void myMesh::simplify() { /**** TODO ****/ }

void myMesh::simplify(myVertex *) { /**** TODO ****/ }

void myMesh::triangulate()
{
  vector<myFace *> original_faces = faces;
  for (myFace *f : original_faces)
  {
    triangulate(f);
  }
}

bool myMesh::triangulate(myFace *f)
{
    myHalfedge *start = f->adjacent_halfedge;

    int count = 0;
    myHalfedge *curr = start;
    do {
        count++;
        curr = curr->next;
    } while (curr != start);

    if (count <= 3)
        return false;

    myHalfedge *v0 = start;
    myHalfedge *v1 = start->next;
    myHalfedge *v2 = v1->next;

    for (int i = 0; i < count - 3; i++)
    {
        myHalfedge *v3 = v2->next;

        myHalfedge *e0 = new myHalfedge();
        myHalfedge *e1 = new myHalfedge();
        myHalfedge *e2 = new myHalfedge();

        halfedges.push_back(e0);
        halfedges.push_back(e1);
        halfedges.push_back(e2);

        myFace *f_new = new myFace();
        faces.push_back(f_new);

        e0->source = v0->source;
        e1->source = v1->source;
        e2->source = v2->source;

        e0->next = e1;
        e1->next = e2;
        e2->next = e0;

        e0->prev = e2;
        e1->prev = e0;
        e2->prev = e1;

        e0->adjacent_face = f_new;
        e1->adjacent_face = f_new;
        e2->adjacent_face = f_new;

        f_new->adjacent_halfedge = e0;

        v1 = v2;
        v2 = v3;
    }

    return true;
}
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

void myMesh::clear()
{
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

// Test : chaque half-edge a un twin
void myMesh::checkTwins()
{
    vector<myHalfedge *>::iterator it;
    for (it = halfedges.begin(); it != halfedges.end(); it++)
    {
        if ((*it)->twin == NULL)
            break;
    }
    if (it != halfedges.end())
        cout << "Error! Not all halfedges have a twin!\n";
    else
        cout << "Each halfedge has a twin!\n";
}

// Test : chaque half-edge a une source non nulle
void myMesh::checkSources()
{
    vector<myHalfedge *>::iterator it;
    for (it = halfedges.begin(); it != halfedges.end(); it++)
    {
        if ((*it)->source == NULL)
            break;
    }
    if (it != halfedges.end())
        cout << "Error! Some halfedge has a NULL source vertex!\n";
    else
        cout << "All halfedges have a valid source!\n";
}

//  Test : chaînage next/prev cohérent  (he->next->prev == he)
void myMesh::checkNextPrev()
{
    vector<myHalfedge *>::iterator it;
    for (it = halfedges.begin(); it != halfedges.end(); it++)
    {
        if ((*it)->next == NULL || (*it)->prev == NULL)
            break;
        if ((*it)->next->prev != (*it))
            break;
        if ((*it)->prev->next != (*it))
            break;
    }
    if (it != halfedges.end())
        cout << "Error! next/prev linkage is inconsistent!\n";
    else
        cout << "next/prev linkage is consistent!\n";
}

//  Test : chaque sommet a un half-edge sortant non nul
void myMesh::checkVertex() {
    vector<myVertex *>::iterator it_v;
    for (it_v = vertices.begin(); it_v != vertices.end(); it_v++)
    {
        if ((*it_v)->originof == NULL || (*it_v)->originof->source != (*it_v))
            break;
    }
    if (it_v != vertices.end()) {
        cout << "Error! Some vertices point to an invalid 'originof' half-edge!\n";
    } else {
        cout << "All vertices have a valid 'originof' reference!\n";
    }
}

//  Test : chaque face a un half-edge adjacent non nul
void myMesh::checkFaceHalfedges()
{
    vector<myFace *>::iterator it;
    for (it = faces.begin(); it != faces.end(); it++)
    {
        if ((*it)->adjacent_halfedge == NULL)
            break;
    }
    if (it != faces.end())
        cout << "Error! Some face has no adjacent halfedge!\n";
    else
        cout << "All faces have an adjacent halfedge!\n";
}

//  Test : formule d'Euler  V - E + F = 2
void myMesh::checkEuler()
{
    int V = (int)vertices.size();
    int E = (int)halfedges.size() / 2;
    int F = (int)faces.size();
    int chi = V - E + F;
 
    if (chi != 2)
        cout << "Error! Euler formula failed: V=" << V
             << " E=" << E << " F=" << F << " chi=" << chi << " (expected 2)\n";
    else
        cout << "Euler formula satisfied: V=" << V
             << " E=" << E << " F=" << F << " chi=2\n";
}

//  Test : les twins sont réciproques  (he->twin->twin == he)
void myMesh::checkTwinReciprocity()
{
    vector<myHalfedge *>::iterator it;
    for (it = halfedges.begin(); it != halfedges.end(); it++)
    {
        if ((*it)->twin == NULL || (*it)->twin->twin != (*it))
            break;
    }
    if (it != halfedges.end())
        cout << "Error! Some twin is not reciprocal (he->twin->twin != he)!\n";
    else
        cout << "All twins are reciprocal!\n";
}

// Fonction principale appelée dans le menu pour lancer toute la suite de tests d'intégrité
void myMesh::checkMesh()
{
    //Half-edge data structure tests
    cout << "/ ------------ Checking mesh integrity... ----------- \n";

// Lancement successif de toutes les vérifications
    checkTwins();
    checkNextPrev();
    checkFaceHalfedges();
    checkEuler();
    checkVertex();
    checkTwinReciprocity();
    checkSources();
}

bool myMesh::readFile(std::string filename)
{
    string s, t, u;

// Ouverture du fichier .obj
    ifstream fin(filename);
    if (!fin.is_open())
    {
        cout << "Unable to open file!\n";
        return false;
    }

    name = filename;

    map<pair<int, int>, myHalfedge *> twin_map;

    while (getline(fin, s))
    {
        if (s.empty() || s[0] == '#')
            continue;

        stringstream myline(s);
        myline >> t;
        //CAS D'UN SOMMET ('v')
        if (t == "v")
        {
            float x, y, z;
            myline >> x >> y >> z;

            myPoint3D *p = new myPoint3D(x, y, z);
            myVertex *v = new myVertex();

            v->point = p;
            v->originof = NULL;

            vertices.push_back(v);
        }
        // CAS D'UNE FACE ('f')
        else if (t == "f")
        {

            vector<int> ids;

            /* [NOTE ÉTUDIANT & IA] : 
               Ce bloc de parsing ci-dessous a été développé avec l'aide de l'IA (Gemini).
               Il permet de gérer proprement les formats complexes du .obj (comme 'v/vt/vn')
               et de convertir les indices négatifs parfois utilisés dans ces fichiers.
            */
            while (myline >> u)
            {
                // Découpe la chaîne si elle contient des '/' (ex: '1/2/3' devient '1')
                string token = u.substr(0, u.find("/"));
                int id = atoi(token.c_str()); // Convertit le texte en nombre entier

                // Gestion des indices négatifs (qui comptent à l'envers depuis la fin)
                if (id < 0)
                    id = vertices.size() + id;
                else
                    id = id - 1; // Le format .obj commence à 1, notre tableau C++ commence à 0

                if (id < 0 || id >= vertices.size())
                    continue;

                ids.push_back(id);
            }

            int n = ids.size();
            if (n < 3)
                continue;

            myFace *face = new myFace();
            faces.push_back(face);

            vector<myHalfedge *> face_edges;
            // 1. Création des demi-arêtes de la face
            for (int i = 0; i < n; i++)
            {
                myHalfedge *he = new myHalfedge();
                halfedges.push_back(he);

                he->adjacent_face = face;
                he->source = vertices[ids[i]];
                he->twin = NULL;

                face_edges.push_back(he);
            }
            // 2. Chaînage des arêtes entre elles (Next et Prev) pour tourner autour de la face
            for (int i = 0; i < n; i++)
            {
                face_edges[i]->next = face_edges[(i + 1) % n];
                face_edges[i]->prev = face_edges[(i - 1 + n) % n];
            }
            // 3. Connexion des Twins et mise à jour des sommets
            for (int i = 0; i < n; i++)
            {
                int v1 = ids[i];
                int v2 = ids[(i + 1) % n];

                pair<int, int> edge = make_pair(v1, v2);
                pair<int, int> twin_edge = make_pair(v2, v1);

                myHalfedge *he = face_edges[i];

                // On regarde si l'arête inverse existe déjà dans notre map
                auto it = twin_map.find(twin_edge);
                if (it != twin_map.end())
                {
                    // Si elle existe, on les connecte comme twins
                    he->twin = it->second;
                    it->second->twin = he;
                }
                else
                {
                    // Sinon, on enregistre l'arête actuelle pour qu'une future face la trouve comme twin
                    twin_map[edge] = he;
                }
                // Si le sommet v1 n'a pas encore d'arête de départ assignée, on lui donne celle-ci
                if (vertices[v1]->originof == NULL)
                {
                    vertices[v1]->originof = he;
                }
            }
            // La face prend comme arête de référence la première du tableau
            face->adjacent_halfedge = face_edges[0];
        }
    }

    fin.close(); // Fermeture du fichier

    checkMesh();
    normalize();

    return true;
}

// Calcule l'orientation (les normales) de tout le maillage 
void myMesh::computeNormals()
{
    // 1. On demande d'abord à chaque face de calculer sa propre normale (sa direction)
    for (myFace *f : faces)
        f->computeNormal();
    // 2. Ensuite, chaque sommet calcule sa normale en faisant la moyenne des faces qui l'entourent
    for (myVertex *v : vertices)
        v->computeNormal();
}

void myMesh::normalize()
{
    if (vertices.size() < 1)
        return;

    int tmpxmin = 0, tmpymin = 0, tmpzmin = 0, tmpxmax = 0, tmpymax = 0,
        tmpzmax = 0;

    for (unsigned int i = 0; i < vertices.size(); i++)
    {
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

    for (unsigned int i = 0; i < vertices.size(); i++)
    {
        vertices[i]->point->X -= (xmax + xmin) / 2;
        vertices[i]->point->Y -= (ymax + ymin) / 2;
        vertices[i]->point->Z -= (zmax + zmin) / 2;

        vertices[i]->point->X /= scale;
        vertices[i]->point->Y /= scale;
        vertices[i]->point->Z /= scale;
    }
}

// Coupe une face triangulaire (f) en 3 nouveaux triangles autour d'un nouveau point (p)
void myMesh::splitFaceTRIS(myFace *f, myPoint3D *p)
{
    // 1. CRÉATION DU NOUVEAU SOMMET CENTRAL
    myVertex *v_new = new myVertex();
    v_new->point = new myPoint3D(p->X, p->Y, p->Z);
    v_new->index = vertices.size();
    vertices.push_back(v_new);

    // 2. RÉCUPÉRATION DES TROIS DEMI-ARÊTES DU TRIANGLE D'ORIGINE
    // Comme c'est un triangle, en faisant ".next" trois fois de suite, on fait le tour de la face
    myHalfedge *e1 = f->adjacent_halfedge;
    myHalfedge *e2 = e1->next;
    myHalfedge *e3 = e2->next;

    // 3. RÉCUPÉRATION DES TROIS ANCIENS SOMMETS DU TRIANGLE
    // On stocke les sommets de départ de chaque arête pour pouvoir reconnecter le tout plus tard
    myVertex *v1 = e1->source;
    myVertex *v2 = e2->source;
    myVertex *v3 = e3->source;
}

// Divise une arête (e1) en deux en insérant un nouveau sommet (p) au milieu
void myMesh::splitEdge(myHalfedge *e1, myPoint3D *p)
{
    // 1. CRÉATION DU NOUVEAU SOMMET AU MILIEU
    myVertex *v_new = new myVertex();
    v_new->point = new myPoint3D(p->X, p->Y, p->Z);
    vertices.push_back(v_new);
    // 2. RÉCUPÉRATION DES VOISINS AUTOUR DE L'ARÊTE
    myHalfedge *enext = e1->next;
    myHalfedge *etwin = e1->twin;
    myVertex *v_end = enext->source;
    // 3. CRÉATION DE LA NOUVELLE DEMI-ARÊTE POUR COMPLÉTER LE SPLIT
    myHalfedge *e_new = new myHalfedge();
    halfedges.push_back(e_new);

    e_new->source = v_new;
    e_new->next = enext;
    e_new->adjacent_face = e1->adjacent_face;
    e1->next = e_new;
    v_new->originof = e_new;
}

// Prépare la découpe d'une face à 4 côtés (Quad) autour d'un nouveau point central (p)
void myMesh::splitFaceQUADS(myFace *f, myPoint3D *p)
{
    // 1. CRÉATION DU SOMMET CENTRAL DE LA FACE
    myVertex *v_center = new myVertex();
    v_center->point = new myPoint3D(p->X, p->Y, p->Z);
    v_center->index = vertices.size();
    vertices.push_back(v_center);
    // 2. RÉCUPÉRATION ET STOCKAGE DE TOUTES LES ARÊTES DE LA FACE D'ORIGINE
    vector<myHalfedge *> original_edges;
    myHalfedge *curr = f->adjacent_halfedge;

    //Cette boucle parcourt les 4 arêtes du Quad un par un en suivant le pointeur '.next'.
    do
    {
        original_edges.push_back(curr);
        curr = curr->next;
    } while (curr != f->adjacent_halfedge);
}

void myMesh::subdivisionCatmullClark() {
    if (faces.empty()) return; // Si le maillage est vide, on ne fait rien

    // Tableaux de correspondance (maps) pour stocker les nouveaux points générés pendant les calculs
    std::map<myFace*, myPoint3D*> facePoints;
    std::map<myHalfedge*, myPoint3D*> edgePoints;
    std::map<myVertex*, myPoint3D*> vertexPoints;

    // Dictionnaires pour stocker les cumuls nécessaires à la formule mathématique finale
    // Pour le calcul des nouveaux sommets (Formule de Catmull-Clark)
    std::map<myVertex*, myPoint3D*> sumFacePoints;
    std::map<myVertex*, myPoint3D*> sumEdgeMidpoints;
    std::map<myVertex*, int> valence;

    for (myVertex* v : vertices) {
        sumFacePoints[v] = new myPoint3D(0.0f, 0.0f, 0.0f);
        sumEdgeMidpoints[v] = new myPoint3D(0.0f, 0.0f, 0.0f);
        valence[v] = 0;
    }

    /* [NOTE ÉTUDIANT & IA] : 
       L'ÉTAPE 1 et l'ÉTAPE 2 reposent sur les équations de subdivision de Catmull-Clark.
       L'IA (Gemini) a grandement aidé à traduire ces formules théoriques en boucles de code 
       qui accumulent correctement les coordonnées sans se mélanger les pinceaux dans les structures.
    */
    //ÉTAPE 1 : Calculer les Face Points et accumuler les contributions pour les Vertex
    // Pour chaque face, on crée un nouveau point situé exactement en son centre barycentrique
    for (myFace* f : faces) {
        float sumX = 0, sumY = 0, sumZ = 0;
        int count = 0;
        myHalfedge* start = f->adjacent_halfedge;
        myHalfedge* curr = start;
        // On fait le tour de la face pour faire la somme des positions de ses sommets
        do {
            sumX += curr->source->point->X;
            sumY += curr->source->point->Y;
            sumZ += curr->source->point->Z;
            count++;
            curr = curr->next;
        } while (curr != start);
        // Le Face Point est la moyenne de tous les sommets de la face
        myPoint3D* fp = new myPoint3D(sumX / count, sumY / count, sumZ / count);
        facePoints[f] = fp;
        // On distribue ce centre aux sommets de la face pour les calculs de l'étape 3
        curr = start;
        do {
            sumFacePoints[curr->source]->X += fp->X;
            sumFacePoints[curr->source]->Y += fp->Y;
            sumFacePoints[curr->source]->Z += fp->Z;
            curr = curr->next;
        } while (curr != start);
    }

    //ÉTAPE 2 : Calculer les Edge Points et accumuler les contributions pour les Vertex
    // Pour chaque arête, on crée un point qui est la moyenne entre ses deux sommets et les Face Points des deux faces adjacentes.
    for (myHalfedge* h : halfedges) {
        if (edgePoints.count(h) > 0) continue;

        myVertex* v1 = h->source;
        myVertex* v2 = h->next->source;

        myPoint3D* fp1 = facePoints[h->adjacent_face];
        myPoint3D* fp2 = (h->twin && h->twin->adjacent_face) ? facePoints[h->twin->adjacent_face] : nullptr;

        float ex, ey, ez;
        if (fp2) {
            // Formule standard donnée par Gemini et transformée en code par moi : 
            //moyenne des 2 sommets de l'arête + les 2 centres des faces voisines
            ex = (v1->point->X + v2->point->X + fp1->X + fp2->X) / 4.0f;
            ey = (v1->point->Y + v2->point->Y + fp1->Y + fp2->Y) / 4.0f;
            ez = (v1->point->Z + v2->point->Z + fp1->Z + fp2->Z) / 4.0f;
        } else {
            // Cas particulier si on est sur un bord ouvert (pas de deuxième face) : on fait la moyenne des deux sommets seulement
            ex = (v1->point->X + v2->point->X) / 2.0f;
            ey = (v1->point->Y + v2->point->Y) / 2.0f;
            ez = (v1->point->Z + v2->point->Z) / 2.0f;
        }

        myPoint3D* ep = new myPoint3D(ex, ey, ez);
        edgePoints[h] = ep;
        if (h->twin) edgePoints[h->twin] = ep;

        // Sauvegarde du milieu physique de l'arête pour l'étape 3
        float midX = (v1->point->X + v2->point->X) / 2.0f;
        float midY = (v1->point->Y + v2->point->Y) / 2.0f;
        float midZ = (v1->point->Z + v2->point->Z) / 2.0f;

        sumEdgeMidpoints[v1]->X += midX; sumEdgeMidpoints[v1]->Y += midY; sumEdgeMidpoints[v1]->Z += midZ;
        sumEdgeMidpoints[v2]->X += midX; sumEdgeMidpoints[v2]->Y += midY; sumEdgeMidpoints[v2]->Z += midZ;

        valence[v1]++;
        valence[v2]++;
    }

    //ÉTAPE 3 : Calculer les new Vertex Points
    for (myVertex* v : vertices) {
        int n = valence[v];
        if (n == 0) {
            vertexPoints[v] = new myPoint3D(v->point->X, v->point->Y, v->point->Z);
            continue;
        }

        // Formule barycentrique de Catmull-Clark : 
        // v' = (F / n) + (2 * E / n) + ((n - 3) * v) / n
        /* [NOTE ÉTUDIANT & IA] : 
           L'IA a écrit l'application exacte de la formule barycentrique de Catmull-Clark :
           v' = (F / n) + (2 * E / n) + ((n - 3) * v) / n
           C'est cette pondération mathématique précise qui crée l'effet d'arrondissement lisse.
        */
        float fx = sumFacePoints[v]->X / (n * n);
        float fy = sumFacePoints[v]->Y / (n * n);
        float fz = sumFacePoints[v]->Z / (n * n);

        float ex = 2.0f * sumEdgeMidpoints[v]->X / (n * n);
        float ey = 2.0f * sumEdgeMidpoints[v]->Y / (n * n);
        float ez = 2.0f * sumEdgeMidpoints[v]->Z / (n * n);

        float vx = (n - 3) * v->point->X / n;
        float vy = (n - 3) * v->point->Y / n;
        float vz = (n - 3) * v->point->Z / n;

        vertexPoints[v] = new myPoint3D(fx + ex + vx, fy + ey + vy, fz + ez + vz);

        delete sumFacePoints[v];
        delete sumEdgeMidpoints[v];
    }
    /* [NOTE ÉTUDIANT & IA] : 
       L'ÉTAPE 4 et l'ÉTAPE 5 représentent la reconstruction complète du maillage en "Quads".
       Diviser chaque face en petits carrés et recréer à la volée des centaines de demi-arêtes, 
       de faces et reconnecter leurs pointeurs (originof, next, prev) est d'une complexité extrême.
       L'IA a fourni l'ossature logique complète de cette reconstruction topologique.
    */
    //ÉTAPE 4 : Reconstruction de la topologie
    // Maps de conversion pour lier les anciens éléments aux nouveaux objets de maillage (myVertex)
    std::map<myFace*, myVertex*> faceVMap;
    std::map<myHalfedge*, myVertex*> edgeVMap;
    std::map<myVertex*, myVertex*> vertexVMap;

    std::vector<myVertex*> newVertices;
    std::vector<myFace*> newFaces;
    std::vector<myHalfedge*> newHalfedges;

    // Conversion de tous nos points calculés précédemment en vrais sommets (myVertex) utilisables
    for (auto& p : facePoints) {
        myVertex* nv = new myVertex(); nv->point = p.second; nv->originof = nullptr;
        nv->index = newVertices.size(); newVertices.push_back(nv);
        faceVMap[p.first] = nv;
    }
    for (auto& p : edgePoints) {
        if (edgeVMap.count(p.first) > 0) continue;
        myVertex* nv = new myVertex(); nv->point = p.second; nv->originof = nullptr;
        nv->index = newVertices.size(); newVertices.push_back(nv);
        edgeVMap[p.first] = nv;
        if (p.first->twin) edgeVMap[p.first->twin] = nv;
    }
    for (auto& p : vertexPoints) {
        myVertex* nv = new myVertex(); nv->point = p.second; nv->originof = nullptr;
        nv->index = newVertices.size(); newVertices.push_back(nv);
        vertexVMap[p.first] = nv;
    }

    //Subdivision de chaque ancienne face en plusieurs faces à 4 côtés (Quads)
    for (myFace* f : faces) {
        myHalfedge* start = f->adjacent_halfedge;
        myHalfedge* curr = start;

        do {
            // Pour chaque coin de la face d'origine, on extrait les 4 sommets du nouveau Quad
            myVertex* v0 = vertexVMap[curr->source];
            myVertex* v1 = edgeVMap[curr];
            myVertex* v2 = faceVMap[f];
            myVertex* v3 = edgeVMap[curr->prev];

            myFace* nf = new myFace();
            myHalfedge* h0 = new myHalfedge();
            myHalfedge* h1 = new myHalfedge();
            myHalfedge* h2 = new myHalfedge();
            myHalfedge* h3 = new myHalfedge();

            // Remplissage des sources des nouvelles demi-arêtes
            h0->source = v0; h1->source = v1; h2->source = v2; h3->source = v3;

            // Maillage interne du Quad (Next / Prev)
            h0->next = h1; h1->next = h2; h2->next = h3; h3->next = h0;
            h0->prev = h3; h1->prev = h0; h2->prev = h1; h3->prev = h2;

            // Liaison à la nouvelle face
            h0->adjacent_face = nf; h1->adjacent_face = nf; h2->adjacent_face = nf; h3->adjacent_face = nf;
            nf->adjacent_halfedge = h0;

            // Attribution d'une arête de départ pour les sommets si vide
            if (!v0->originof) v0->originof = h0;
            if (!v1->originof) v1->originof = h1;
            if (!v2->originof) v2->originof = h2;
            if (!v3->originof) v3->originof = h3;

            newHalfedges.push_back(h0);
            newHalfedges.push_back(h1);
            newHalfedges.push_back(h2);
            newHalfedges.push_back(h3);
            newFaces.push_back(nf);

            curr = curr->next;
        } while (curr != start);
    }

    //ÉTAPE 5 : Recoudre les pointeurs twin
    // Utilisation d'un dictionnaire d'arêtes pour reconnecter les jumeaux du nouveau maillage subdivisé
    std::map<std::pair<myVertex*, myVertex*>, myHalfedge*> edgeMap;
    for (myHalfedge* h : newHalfedges) {
        edgeMap[{h->source, h->next->source}] = h;
    }

    for (myHalfedge* h : newHalfedges) {
        auto it = edgeMap.find({h->next->source, h->source});
        if (it != edgeMap.end() && it->second != h) {
            h->twin = it->second;
        } else {
            h->twin = nullptr;
        }
    }
    // Remplacement final de l'ancien maillage par le nouveau maillage lissé et subdivisé
    this->vertices = newVertices;
    this->faces = newFaces;
    this->halfedges = newHalfedges;
}

void myMesh::surfaceRevolution()
{
    clear();

    // [NOTE ÉTUDIANT & IA] : Profil 2D d'un pion d'échecs généré par l'IA (Gemini).
    // Les coordonnées décrivent la silhouette du pion de bas (Y=0) en haut (Y=0.8).
    //Profil de pion d'échec généré par Gemini
    vector<myPoint3D> profile = {
        {0.0, 0.0, 0.0},         // Fond (fermé sur l'axe)
        {0.3333f, 0.0, 0.0},     // Bord du fond
        {0.3f, 0.1f, 0.0},       // Premier palier
        {0.1667f, 0.1333f, 0.0}, // Creux au-dessus de la base
        {0.2333f, 0.3333f, 0.0}, // Corps du pion
        {0.1f, 0.4667f, 0.0},    // Le cou
        {0.1667f, 0.5f, 0.0},    // La collerette
        {0.1667f, 0.5667f, 0.0}, // Bas de la tête
        {0.2f, 0.6667f, 0.0},    // Largeur max de la tête
        {0.0, 0.8f, 0.0}         // Sommet (fermé sur l'axe)
    };

    int slices = 30; // Nombre de segments pour faire le tour complet (360 degrés)
    float dtheta = 2.0f * M_PI / slices; // Angle de rotation entre chaque segment

    vector<vector<myVertex *>> grid;
    // 1. GÉNÉRATION DES SOMMETS EN TOURNANT AUTOUR DE L'AXE Y
    for (int i = 0; i < slices; i++)
    {

        float theta = i * dtheta;
        vector<myVertex *> ring;

        for (const myPoint3D &p : profile)
        {
            // Calcul mathématique des coordonnées 3D (X et Z tournent, Y reste fixe)
            float x = p.X * cos(theta);
            float z = p.X * sin(theta);
            float y = p.Y;

            myVertex *v = new myVertex();
            v->point = new myPoint3D(x, y, z);
            v->originof = nullptr;

            vertices.push_back(v);
            ring.push_back(v);
        }

        grid.push_back(ring);
    }
    // 2. CRÉATION DES FACES (QUADS) EN RELIANT LES SOMMETS DE LA GRILLE
    for (int i = 0; i < slices; i++)
    {

        int next = (i + 1) % slices;

        for (int j = 0; j < profile.size() - 1; j++)
        {
            // Récupération des 4 sommets formant un carré (Quad)
            myVertex *v0 = grid[i][j];
            myVertex *v1 = grid[next][j];
            myVertex *v2 = grid[next][j + 1];
            myVertex *v3 = grid[i][j + 1];

            myFace *f = new myFace();

            // Création des 4 demi-arêtes qui font le tour de la face
            myHalfedge *h0 = new myHalfedge();
            myHalfedge *h1 = new myHalfedge();
            myHalfedge *h2 = new myHalfedge();
            myHalfedge *h3 = new myHalfedge();

            // Attribution du sommet de départ pour chaque arête
            h0->source = v0;
            h1->source = v1;
            h2->source = v2;
            h3->source = v3;

            // Chaînage vers l'avant (Next)
            h0->next = h1;
            h1->next = h2;
            h2->next = h3;
            h3->next = h0;

            // Chaînage vers l'arrière (Prev)
            h0->prev = h3;
            h1->prev = h0;
            h2->prev = h1;
            h3->prev = h2;

            // Liaison des arêtes à leur face commune
            h0->adjacent_face = f;
            h1->adjacent_face = f;
            h2->adjacent_face = f;
            h3->adjacent_face = f;

            f->adjacent_halfedge = h0;

            // Si un sommet n'a pas encore d'arête de départ attitrée, on lui donne celle-ci
            if (!v0->originof)
                v0->originof = h0;
            if (!v1->originof)
                v1->originof = h1;
            if (!v2->originof)
                v2->originof = h2;
            if (!v3->originof)
                v3->originof = h3;

            halfedges.push_back(h0);
            halfedges.push_back(h1);
            halfedges.push_back(h2);
            halfedges.push_back(h3);
            faces.push_back(f);
        }
    }

    /* [NOTE ÉTUDIANT & IA] : 
       Cette dernière étape de connexion des Twins utilise une structure de dictionnaire (map). 
       L'IA m'a aidé à la concevoir pour lier automatiquement et proprement les arêtes adjacentes 
       sans écrire de doubles boucles 'for' imbriquées qui auraient rendu le programme très lent.
    */

   // 3. MISE EN CORRESPONDANCE DES TWINS
    map<pair<myVertex *, myVertex *>, myHalfedge *> edgeMap;

    // On enregistre chaque arête dans la map avec la clé {sommet_départ, sommet_arrivée}
    for (myHalfedge *h : halfedges)
    {
        edgeMap[{h->source, h->next->source}] = h;
    }
    // On parcourt à nouveau les arêtes pour chercher leur inverse {sommet_arrivée, sommet_départ}
    for (myHalfedge *h : halfedges)
    {

        auto it = edgeMap.find({h->next->source, h->source});

        if (it != edgeMap.end() && it->second != h)
        {
            h->twin = it->second;
        }
        else
        {
            h->twin = nullptr;
        }
    }
}

void myMesh::simplify()
{
    // Si le maillage est vide, on ne fait rien
    if (faces.empty() || vertices.empty()) return;

    // Définir un objectif (ici réduire de 10% le nombre de faces)
    size_t target_faces = static_cast<size_t>(faces.size() * 0.90);

    // Boucle principale : tant qu'on n'a pas atteint notre objectif et qu'il reste des arêtes à simplifier
    while (faces.size() > target_faces && !halfedges.empty()) {
        myHalfedge* global_shortest = nullptr;
        float min_dist = std::numeric_limits<float>::max();

        // Recherche de l'arête la plus courte sur l'ensemble du maillage
        for (myHalfedge* h : halfedges) {
            myVertex* v1 = h->source;
            myVertex* v2 = h->next->source;
            
            // Calcul classique de la distance 3D entre les deux sommets de l'arête
            float dx = v1->point->X - v2->point->X;
            float dy = v1->point->Y - v2->point->Y;
            float dz = v1->point->Z - v2->point->Z;
            float dist = std::sqrt(dx*dx + dy*dy + dz*dz);

            // On garde en mémoire l'arête la plus petite trouvée
            if (dist < min_dist) {
                min_dist = dist;
                global_shortest = h;
            }
        }

        // Si aucune arête n'est trouvable, on stoppe
        if (!global_shortest) break;

        simplify(global_shortest->source);
    }
    // Après les suppressions, les indices dans le tableau ont changé, on les réaligne proprement
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertices[i]->index = i;
    }
}

void myMesh::simplify(myVertex *v)
{
    if (!v || halfedges.empty()) return;

    // 1. Trouver l'arête la plus courte partant de v
    myHalfedge* shortest_h = nullptr;
    float min_dist = std::numeric_limits<float>::max();

    for (myHalfedge* h : halfedges) {
        if (h->source == v) {
            myVertex* vt = h->next->source;
            float dx = v->point->X - vt->point->X;
            float dy = v->point->Y - vt->point->Y;
            float dz = v->point->Z - vt->point->Z;
            float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (dist < min_dist) {
                min_dist = dist;
                shortest_h = h;
            }
        }
    }

    // Si le sommet est isolé ou n'a pas d'arête sortante valide
    if (!shortest_h) return;

    // 2. Identifier les éléments impactés par le collapse
    myVertex* v_target = shortest_h->next->source;
    myHalfedge* h_twin = shortest_h->twin;

    // Face adjacente directe et ses composants
    myFace* f1 = shortest_h->adjacent_face;
    myHalfedge* h1_next = f1 ? shortest_h->next : nullptr;
    myHalfedge* h1_prev = f1 ? shortest_h->prev : nullptr;

    // Face adjacente opposée (via le twin) et ses composants
    myFace* f2 = h_twin ? h_twin->adjacent_face : nullptr;
    myHalfedge* h2_next = f2 ? h_twin->next : nullptr;
    myHalfedge* h2_prev = f2 ? h_twin->prev : nullptr;

    // Liste des demi-arêtes à détruire définitivement
    std::vector<myHalfedge*> to_delete;
    to_delete.push_back(shortest_h);
    if (h_twin) to_delete.push_back(h_twin);
    if (f1) { to_delete.push_back(h1_next); to_delete.push_back(h1_prev); }
    if (f2) { to_delete.push_back(h2_next); to_delete.push_back(h2_prev); }

    /* [NOTE ÉTUDIANT & IA] : 
       Cette étape 3 est extrêmement sensible. Supprimer des faces crée un "trou" temporaire.
       L'IA (Gemini) m'a fourni la logique exacte pour reconnecter directement entre eux 
       les twins extérieurs restants afin de recoudre le maillage immédiatement.
    */
    // 3. Recoudre les twins extérieurs pour combler le vide des faces supprimées
    if (f1) {
        if (h1_next->twin) h1_next->twin->twin = h1_prev->twin;
        if (h1_prev->twin) h1_prev->twin->twin = h1_next->twin;
    }
    if (f2) {
        if (h2_next->twin) h2_next->twin->twin = h2_prev->twin;
        if (h2_prev->twin) h2_prev->twin->twin = h2_next->twin;
    }

    // 4. Rediriger toutes les arêtes partant de 'v' vers 'v_target'
    for (myHalfedge* h : halfedges) {
        if (h->source == v) {
            h->source = v_target;
        }
    }

    /* [NOTE ÉTUDIANT & IA] : 
       L'utilisation de std::remove couplé à .erase() provient de l'IA. 
       Elle permet de supprimer proprement et rapidement les éléments des vecteurs globaux 
       sans laisser de pointeurs invalides ou "fantômes" à l'intérieur des listes.
    */
    // 5. Nettoyage du maillage (Erase)
    if (f1) faces.erase(std::remove(faces.begin(), faces.end(), f1), faces.end());
    if (f2) faces.erase(std::remove(faces.begin(), faces.end(), f2), faces.end());

    for (myHalfedge* hd : to_delete) {
        halfedges.erase(std::remove(halfedges.begin(), halfedges.end(), hd), halfedges.end());
    }
    vertices.erase(std::remove(vertices.begin(), vertices.end(), v), vertices.end());

    // 6. Reconstruction des pointeurs originof
    // Comme beaucoup d'arêtes de départ ont sauté, on réinitialise tout le monde à blanc et on redonne une arête de départ à chaque sommet à partir des demi-arêtes restantes
    for (myVertex* vert : vertices) vert->originof = nullptr;
    for (myHalfedge* h : halfedges) {
        h->source->originof = h;
    }

    /* [NOTE ÉTUDIANT & IA] : 
       Pour éviter que l'application ne consomme toute la RAM lors de grosses simplifications, 
       l'IA m'a rappelé d'appliquer des 'delete' sur l'ensemble de la mémoire dynamique (les 'new') 
       des pointeurs C++ détruits (le point, le sommet, les faces et les arêtes supprimées).
    */
    // 7. Libération de la mémoire
    delete v->point;
    delete v;
    if (f1) delete f1;
    if (f2) delete f2;
    for (myHalfedge* hd : to_delete) delete hd;
}

// Fonction principale qui parcourt toutes les faces du maillage pour les trianguler
void myMesh::triangulate()
{
    vector<myFace *> original_faces = faces;
    for (myFace *f : original_faces)
    {
        triangulate(f);
    }
}
/*
  MÉTHODE UTILISÉE : Triangulation en éventail (Fan Triangulation)

  PRINCIPE : 
  On choisit un sommet de départ fixe (v0), et on relie ce sommet à tous les autres 
  en sautant un sommet à chaque fois pour découper le polygone en triangles.

  [LE PROBLÈME AVEC GEAR.OBJ] :
  Cette méthode basique fonctionne parfaitement sur les polygones "convexes" (sans creux).
  Cependant, sur le fichier "gear.obj", certaines faces au niveau des dents 
  sont "concaves" (elles rentrent vers l'intérieur). 

  La triangulation en éventail forçait des lignes à passer À L'EXTÉRIEUR de l'objet, 
  créant des triangles fantômes au-dessus du vide. Pour résoudre complètement ce problème, 
  il faut passer à une méthode avancée : Ear Clipping.
*/
// Triangulation de base
/*bool myMesh::triangulate(myFace *f)
{
    myHalfedge *start = f->adjacent_halfedge;
    // 1. COMPTER LE NOMBRE DE CÔTÉS DE LA FACE
    int count = 0;
    myHalfedge *curr = start;
    do {
        count++;
        curr = curr->next;
    } while (curr != start);
    // Si la face est déjà un triangle (3 côtés ou moins), pas besoin de la découper
    if (count <= 3)
        return false;

    myHalfedge *v0 = start;
    myHalfedge *v1 = start->next;
    myHalfedge *v2 = v1->next;

    // 2. CRÉATION DES NOUVEAUX TRIANGLES
    for (int i = 0; i < count - 3; i++)
    {
        myHalfedge *v3 = v2->next;

        // On crée 3 nouvelles demi-arêtes pour notre nouveau triangle interne
        myHalfedge *e0 = new myHalfedge();
        myHalfedge *e1 = new myHalfedge();
        myHalfedge *e2 = new myHalfedge();

        halfedges.push_back(e0);
        halfedges.push_back(e1);
        halfedges.push_back(e2);

        // On crée une nouvelle face triangulaire
        myFace *f_new = new myFace();
        faces.push_back(f_new);

        e0->source = v0->source;
        e1->source = v1->source;
        e2->source = v2->source;

        // On chaîne les 3 arêtes entre elles pour former un cycle (un triangle)
        e0->next = e1;
        e1->next = e2;
        e2->next = e0;

        e0->prev = e2;
        e1->prev = e0;
        e2->prev = e1;

        // On associe ces arêtes à la nouvelle face
        e0->adjacent_face = f_new;
        e1->adjacent_face = f_new;
        e2->adjacent_face = f_new;

        f_new->adjacent_halfedge = e0;

        v1 = v2;
        v2 = v3;
    }

    return true;
}*/

// Triangulation Avancée : Algorithme du Ear Clipping (Découpage d'oreilles)
// Idéal pour régler le problème des faces concaves de l'engrenage (gear.obj)
bool myMesh::triangulate(myFace *face)
{
    const double eps = 1e-10; // Petite marge pour la précision des float

    // 1. COMPTER LE NOMBRE DE CÔTÉS DE LA FACE (Comme dans la méthode basique)
    int nb = 0;
    myHalfedge *h = face->adjacent_halfedge;
    do
    {
        nb++;
        h = h->next;
    } while (h != face->adjacent_halfedge);

    // Si la face a déjà 3 côtés ou moins, c'est déjà un triangle : on s'arrête
    if (nb <= 3)
        return false;

    vector<myHalfedge *> edges(nb);
    vector<myVertex *> points(nb);

    h = face->adjacent_halfedge;
    for (int i = 0; i < nb; i++)
    {
        edges[i] = h;
        points[i] = h->source;
        h = h->next;
    }

/* [NOTE ÉTUDIANT & IA] : 
       Cette partie mathématique calculant la normale d'une face polygonale quelconque 
       (méthode de Newell) a été entièrement générée par l'IA (Gemini) car la formule 
       de géométrie analytique combinant les coordonnées X, Y, Z était trop complexe.
    */
    myVector3D normal(0, 0, 0);
    for (int i = 0; i < nb; i++)
    {
        myPoint3D *p0 = points[i]->point;
        myPoint3D *p1 = points[(i + 1) % nb]->point;

        normal.dX += (p0->Y - p1->Y) * (p0->Z + p1->Z);
        normal.dY += (p0->Z - p1->Z) * (p0->X + p1->X);
        normal.dZ += (p0->X - p1->X) * (p0->Y + p1->Y);
    }

    if (normal.length() < eps)
        return false;
    normal.normalize();

    vector<int> next(nb), prev(nb);
    for (int i = 0; i < nb; i++)
    {
        next[i] = (i + 1) % nb;
        prev[i] = (i - 1 + nb) % nb;
    }

    int left = nb; // Nombre de sommets restants à découper
    int current = 0;
    int guard = 0;

// BOUCLE DE RECHERCHE DES OREILLES VIDES
    while (left > 3)
    {

        bool clipped = false;
        int start = current;

        do
        {
            int before = prev[current];
            int after = next[current];

            myVector3D u = *points[current]->point - *points[before]->point;
            myVector3D v = *points[after]->point - *points[current]->point;

            /* [NOTE ÉTUDIANT & IA] :
               L'utilisation du produit vectoriel multiplié par la normale globale 
               pour détecter si l'angle courant rentre vers l'intérieur (concave) 
               ou sort vers l'extérieur (convexe) provient des algorithmes fournis par l'IA.
            */
            if ((u.crossproduct(v)) * normal > eps)
            {
                bool valid = true;

                int check = next[after];
                while (check != before)
                {
                    myVector3D t0 = (*points[current]->point - *points[before]->point)
                                        .crossproduct(*points[check]->point - *points[before]->point);
                    myVector3D t1 = (*points[after]->point - *points[current]->point)
                                        .crossproduct(*points[check]->point - *points[current]->point);
                    myVector3D t2 = (*points[before]->point - *points[after]->point)
                                        .crossproduct(*points[check]->point - *points[after]->point);

                    if (t0 * normal > eps && t1 * normal > eps && t2 * normal > eps)
                    {
                        valid = false;
                        break;
                    }

                    check = next[check];
                }
                // SI L'OREILLE EST SÛRE : ON COUPE ET ON CRÉE LE TRIANGLE
                if (valid)
                {
                    // Création de la nouvelle arête interne de séparation (avec son twin)
                    myHalfedge *diag1 = new myHalfedge();
                    myHalfedge *diag2 = new myHalfedge();

                    diag1->source = points[after];
                    diag2->source = points[before];

                    diag1->twin = diag2;
                    diag2->twin = diag1;

                    halfedges.push_back(diag1);
                    halfedges.push_back(diag2);

                    // Nouvelle face dédiée à cette oreille coupée
                    myFace *newFace = new myFace();
                    faces.push_back(newFace);

                    newFace->adjacent_halfedge = edges[before];

                    /* [NOTE ÉTUDIANT & IA] :
                       Le re-routage des pointeurs Half-Edge (liaisons des next et prev) 
                       pour isoler le triangle tout en maintenant le reste du polygone fermé 
                       a été calé et sécurisé avec l'aide pas-à-pas de l'IA.
                    */
                    edges[before]->next = edges[current];
                    edges[current]->next = diag1;
                    diag1->next = edges[before];

                    edges[before]->prev = diag1;
                    edges[current]->prev = edges[before];
                    diag1->prev = edges[current];

                    edges[before]->adjacent_face = newFace;
                    edges[current]->adjacent_face = newFace;
                    diag1->adjacent_face = newFace;

                    edges[before] = diag2;

                    next[before] = after;
                    prev[after] = before;

                    left--;
                    current = after;
                    clipped = true;
                    break;
                }
            }

            current = next[current];

        } while (current != start);
    }
    // ON RELIE LES 3 DERNIERS SOMMETS DU POLYGONE RESTANT
    int a = current;
    int b = next[a];
    int c = next[b];

    face->adjacent_halfedge = edges[a];

    edges[a]->next = edges[b];
    edges[b]->next = edges[c];
    edges[c]->next = edges[a];

    edges[a]->prev = edges[c];
    edges[b]->prev = edges[a];
    edges[c]->prev = edges[b];

    edges[a]->adjacent_face = face;
    edges[b]->adjacent_face = face;
    edges[c]->adjacent_face = face;

    return true;
}
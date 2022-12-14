// -------------------------------------------
// gMini : a minimal OpenGL/GLUT application
// for 3D graphics.
// Copyright (C) 2006-2008 Tamy Boubekeur
// All rights reserved.
// -------------------------------------------

// -------------------------------------------
// Disclaimer: this code is dirty in the
// meaning that there is no attention paid to
// proper class attribute access, memory
// management or optimisation of any kind. It
// is designed for quick-and-dirty testing
// purpose.
// -------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <GL/glut.h>
#include <float.h>
#include "src/Vec3.h"
#include "src/Camera.h"
#include <map>

#define X_VALUE 0
#define Y_VALUE 1
#define Z_VALUE 2

enum DisplayMode{ WIRE=0, SOLID=1, LIGHTED_WIRE=2, LIGHTED=3 };

struct Triangle {
    inline Triangle () {
        v[0] = v[1] = v[2] = 0;
    }
    inline Triangle (const Triangle & t) {
        v[0] = t.v[0];   v[1] = t.v[1];   v[2] = t.v[2];
    }
    inline Triangle (unsigned int v0, unsigned int v1, unsigned int v2) {
        v[0] = v0;   v[1] = v1;   v[2] = v2;
    }
    unsigned int & operator [] (unsigned int iv) { return v[iv]; }
    unsigned int operator [] (unsigned int iv) const { return v[iv]; }
    inline virtual ~Triangle () {}
    inline Triangle & operator = (const Triangle & t) {
        v[0] = t.v[0];   v[1] = t.v[1];   v[2] = t.v[2];
        return (*this);
    }
    // membres indices des sommets du triangle:
    unsigned int v[3];
};

int weight_type;
unsigned int resolution = 25;
bool vertex_cube_display = false;
std::vector<Vec3> verticesCube;

struct Mesh {

    std::vector<Vec3> vertices;           // array of mesh vertices positions
    std::vector<Triangle> triangles;      // array of mesh triangles
    std::vector<Vec3> triangle_normals;   // triangle normals to display face normals
    std::vector<Vec3> normals;            // array of vertices normals useful for the display

    //Compute face normals for the display
    void computeTrianglesNormals() {

        Vec3 edge1;
        Vec3 edge2;
        Vec3 normal;

        this->triangle_normals.clear();

        for(const Triangle& triangle : this->triangles) {

            // Calcul des vecteurs d??finissant les deux premi??res arr??tes du triangle
            edge1 = Vec3(this->vertices[triangle[1]][X_VALUE] - this->vertices[triangle[0]][X_VALUE], 
                         this->vertices[triangle[1]][Y_VALUE] - this->vertices[triangle[0]][Y_VALUE], 
                         this->vertices[triangle[1]][Z_VALUE] - this->vertices[triangle[0]][Z_VALUE]);

            edge2 = Vec3(this->vertices[triangle[2]][X_VALUE] - this->vertices[triangle[0]][X_VALUE], 
                         this->vertices[triangle[2]][Y_VALUE] - this->vertices[triangle[0]][Y_VALUE], 
                         this->vertices[triangle[2]][Z_VALUE] - this->vertices[triangle[0]][Z_VALUE]);

            // Calcul de la normale du triangle
            normal = Vec3::cross(edge1,edge2);

            // Normalisation
            normal.normalize();

            // Stockage
            this->triangle_normals.push_back(normal);
        }    
    }

    //Compute vertices normals as the average of its incident faces normals
    void computeVerticesNormals(int weight_type = 0) {

        Vec3 edge1;
        Vec3 edge2;
        Vec3 normal;
        float theta;

        int vertices_count = this->vertices.size();
        int triangles_count = this->triangles.size();

        this->normals.clear();
        this->normals.resize(vertices_count,Vec3(0.,0.,0.));

        for(int triangle_id = 0; triangle_id < triangles_count; ++triangle_id) {

            for(int triangle_vertex_id = 0; triangle_vertex_id < 3; ++triangle_vertex_id){

                switch(weight_type) {

                    // Version uniforme
                    case 0:

                        // Pour chaque sommet du triangle on somme ?? sa normale la normale de la face du triangle
                        this->normals[this->triangles[triangle_id][triangle_vertex_id]] += this->triangle_normals[triangle_id];
                        
                        break;

                    // Version aire : |U x V| / 2
                    case 1:

                        // Pour chaque sommet on d??finit les deux arr??tes du triangle partants de celui-ci
                        edge1 = this->vertices[this->triangles[triangle_id][(triangle_vertex_id + 1) % 3]] - this->vertices[this->triangles[triangle_id][triangle_vertex_id]];
                        edge2 = this->vertices[this->triangles[triangle_id][(triangle_vertex_id + 2) % 3]] - this->vertices[this->triangles[triangle_id][triangle_vertex_id]];

                        // Produit vectoriel des deux arr??tes donnant la normale du triangle
                        normal = Vec3::cross(edge1,edge2);

                        // Pour chaque sommet du triangle on somme ?? sa normale la normale de la face du triangle pond??r??e par l'aire du triangle
                        this->normals[this->triangles[triangle_id][triangle_vertex_id]] += (normal.length() / 2) * this->triangle_normals[triangle_id];

                        break;

                    // Version angle : arcos( (U . V) / (|u| * |v|) )
                    case 2:

                        // Pour chaque sommet on d??finit les deux arr??tes du triangle partants de celui-ci
                        edge1 = this->vertices[this->triangles[triangle_id][(triangle_vertex_id + 1) % 3]] - this->vertices[this->triangles[triangle_id][triangle_vertex_id]];
                        edge2 = this->vertices[this->triangles[triangle_id][(triangle_vertex_id + 2) % 3]] - this->vertices[this->triangles[triangle_id][triangle_vertex_id]];

                        // Produit scalaire donnant apr??s substitution la valeur de l'angle form?? par les deux arr??tes du triangle
                        theta = acos(Vec3::dot(edge1,edge2) / (edge1.length() * edge2.length()));

                        // Pour chaque sommet du triangle on somme ?? sa normale la normale de la face du triangle pond??r??e par l'angle des deux arr??tes
                        this->normals[this->triangles[triangle_id][triangle_vertex_id]] += theta * this->triangle_normals[triangle_id];

                        break;
                }

            }
        }

        // Pour chaque normale au sommet on op??re la normalisation de son vecteur
        for(Vec3& normal : this->normals) {

            normal.normalize();

        }

    }

    void computeNormals(){
        computeTrianglesNormals();
        computeVerticesNormals();
    }

    void simplify(unsigned int resolution){
        
        int x, y, z, key, index;
        float stepX, stepY, stepZ;
        Vec3 min, max;
        std::vector<int> tmp;
        std::vector<Vec3> new_vertices;
        std::vector<Vec3> new_normals;
        std::vector<Triangle> new_triangles;
        std::map <int,Vec3> new_vertices_map;
        std::map <int,int> new_vertices_compteur;
        std::map <int,int> calculated_vertice_in_new_vertices;

        createCube(resolution,min,max);

        stepX = (max[0] - min[0])/ resolution;
        stepY = (max[1] - min[1])/ resolution;
        stepZ = (max[2] - min[2])/ resolution;

        for (long unsigned int i = 0; i < vertices.size(); i++){
            
            x = (int)floor((vertices[i][0] -  min[0]) / stepX);
            y = (int)floor((vertices[i][1] -  min[1]) / stepY);
            z = (int)floor((vertices[i][2] -  min[2]) / stepZ);

            key = x + y * resolution + z * resolution * resolution;

            if (new_vertices_map.find(key) == new_vertices_map.end()) {
                
                new_vertices_map[key] = vertices[i];
                new_vertices_compteur[key] = 1;
            
            } else {
                
                new_vertices_map[key] += vertices[i];
                new_vertices_compteur[key]++; 
            } 
        }

        index = 0;
        for (auto key : new_vertices_map) {

            calculated_vertice_in_new_vertices[key.first] = index++;

            new_vertices.push_back(key.second / new_vertices_compteur[key.first]);
        }

        tmp.resize(3);
         for (unsigned int t = 0; t < triangles.size(); t++){

            for (int i = 0;i < 3; i ++) {

                x = floor((vertices[triangles[t][i]][0] -  min[0]) / stepX);
                y = floor((vertices[triangles[t][i]][1] -  min[1]) / stepY);
                z = floor((vertices[triangles[t][i]][2] -  min[2]) / stepZ);

                tmp[i] = x + y * resolution + z * resolution * resolution;
            }

            if (!(tmp[1]==tmp[2] || tmp[1] == tmp[0] || tmp[2] == tmp[0])) {
                
                new_triangles.push_back(Triangle(calculated_vertice_in_new_vertices[tmp[0]],calculated_vertice_in_new_vertices[tmp[1]],calculated_vertice_in_new_vertices[tmp[2]]));
            }
        }

        vertices = new_vertices;
        triangles = new_triangles;

        computeNormals();
    }


    void createCube(unsigned int resolution, Vec3 &mn, Vec3 &mx) {

        float stepX, stepY, stepZ;

        Vec3 min = Vec3(0., 0., 0.);
        Vec3 max = Vec3(0., 0., 0.);

        verticesCube.clear();
        verticesCube.resize(resolution * resolution * resolution);

        for (unsigned int i = 0; i < vertices.size(); i++) {

            for (unsigned int j = 0; j < 3; j++) {

                if (vertices[i][j] < min[j]) min[j] = vertices[i][j];
                if (vertices[i][j] > max[j]) max[j] = vertices[i][j];
            }
        }
        mn = min;
        mx = max;

        stepX = (max[0] - min[0]) / resolution;
        stepY = (max[1] - min[1]) / resolution;
        stepZ = (max[2] - min[2]) / resolution;

        for (unsigned int x = 0; x < resolution; x++){
            
            for (unsigned int y = 0; y < resolution; y++){
                
                for (unsigned int z = 0; z < resolution; z++){
                    
                    verticesCube[x + y * resolution + z * resolution * resolution] = Vec3(x * stepX + min[0], y * stepY + min[1], z * stepZ + min[2]);
                }
            }
        }
    }
};

//Transformation made of a rotation and translation
struct Transformation {
    Mat3 rotation;
    Vec3 translation;
};

//Basis ( origin, i, j ,k )
struct Basis {
    inline Basis ( Vec3 const & i_origin,  Vec3 const & i_i, Vec3 const & i_j, Vec3 const & i_k) {
        origin = i_origin; i = i_i ; j = i_j ; k = i_k;
    }

    inline Basis ( ) {
        origin = Vec3(0., 0., 0.);
        i = Vec3(1., 0., 0.) ; j = Vec3(0., 1., 0.) ; k = Vec3(0., 0., 1.);
    }
    Vec3 operator [] (unsigned int ib) {
        if(ib==0) return i;
        if(ib==1) return j;
        return k;}

    Vec3 origin;
    Vec3 i;
    Vec3 j;
    Vec3 k;
};

//Fonction ?? completer
void collect_one_ring (std::vector<Vec3> const & i_vertices, std::vector< Triangle > const & i_triangles, std::vector<std::vector<unsigned int> > & o_one_ring) {

    //Initialiser le vecteur de o_one_ring de la taille du vecteur vertices

    o_one_ring.clear();
    o_one_ring.resize(i_vertices.size());

    //Parcourir les triangles et ajouter les voisins dans le 1-voisinage
    //Attention verifier que l'indice n'est pas deja present
    //Tous les points oppos??s dans le triangle sont reli??s

    for(const Triangle& triangle : i_triangles) {

        for(unsigned int current_vertex = 0; current_vertex < 3; ++current_vertex) {

            for(unsigned int neighbor_vertex = 0; neighbor_vertex < 3; ++neighbor_vertex) {

                if(neighbor_vertex != current_vertex) {

                    if( (std::find(o_one_ring[triangle[current_vertex]].begin(),o_one_ring[triangle[current_vertex]].end(),triangle[neighbor_vertex])) == o_one_ring[triangle[current_vertex]].end() ) {

                        o_one_ring[triangle[current_vertex]].push_back(triangle[neighbor_vertex]);

                    }
                }
            }
        }
    }

}

//Fonction ?? completer
void compute_vertex_valences (const std::vector<Vec3> & i_vertices, const std::vector< Triangle > & i_triangles, std::vector<unsigned int> & o_valences ) {

    //Utiliser la fonction collect_one_ring pour r??cuperer le 1-voisinage
    
    std::vector< std::vector<unsigned int> > o_one_ring;

    collect_one_ring(i_vertices,i_triangles,o_one_ring);

    for(unsigned int vertex_id = 0; vertex_id < o_one_ring.size(); ++vertex_id) {

        o_valences.push_back(o_one_ring[vertex_id].size());

    }

}

//Input mesh loaded at the launch of the application
Mesh mesh;

std::vector< float > mesh_valence_field; //normalized valence of each vertex

Basis basis;

bool display_normals;
bool display_smooth_normals;
bool display_mesh;
bool display_basis;
DisplayMode displayMode;


// -------------------------------------------
// OpenGL/GLUT application code.
// -------------------------------------------

static GLint window;
static unsigned int SCREENWIDTH = 1600;
static unsigned int SCREENHEIGHT = 900;
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX=0, lastY=0, lastZoom=0;
static bool fullScreen = true;

// ------------------------------------
// File I/O
// ------------------------------------
bool saveOFF( const std::string & filename, std::vector< Vec3 > const & i_vertices ,
              std::vector< Vec3 > const & i_normals ,
              std::vector< Triangle > const & i_triangles,
              std::vector< Vec3 > const & i_triangle_normals ,
              bool save_normals = true ) 
{
    std::ofstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open()) {
        std::cout << filename << " cannot be opened" << std::endl;
        return false;
    }

    myfile << "OFF" << std::endl ;

    unsigned int n_vertices = i_vertices.size() , n_triangles = i_triangles.size();
    myfile << n_vertices << " " << n_triangles << " 0" << std::endl;

    for( unsigned int v = 0 ; v < n_vertices ; ++v ) {
        myfile << i_vertices[v][0] << " " << i_vertices[v][1] << " " << i_vertices[v][2] << " ";
        if (save_normals) myfile << i_normals[v][0] << " " << i_normals[v][1] << " " << i_normals[v][2] << std::endl;
        else myfile << std::endl;
    }
    for( unsigned int f = 0 ; f < n_triangles ; ++f ) {
        myfile << 3 << " " << i_triangles[f][0] << " " << i_triangles[f][1] << " " << i_triangles[f][2]<< " ";
        if (save_normals) myfile << i_triangle_normals[f][0] << " " << i_triangle_normals[f][1] << " " << i_triangle_normals[f][2];
        myfile << std::endl;
    }
    myfile.close();
    return true;
}

void openOFF( std::string const & filename,
              std::vector<Vec3> & o_vertices,
              std::vector<Vec3> & o_normals,
              std::vector< Triangle > & o_triangles,
              std::vector< Vec3 > & o_triangle_normals,
              bool load_normals = true )
{
    std::ifstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open())
    {
        std::cout << filename << " cannot be opened" << std::endl;
        return;
    }

    std::string magic_s;

    myfile >> magic_s;

    if( magic_s != "OFF" )
    {
        std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
        myfile.close();
        exit(1);
    }

    int n_vertices , n_faces , dummy_int;
    myfile >> n_vertices >> n_faces >> dummy_int;

    o_vertices.clear();
    o_normals.clear();

    for( int v = 0 ; v < n_vertices ; ++v )
    {
        float x , y , z ;

        myfile >> x >> y >> z ;
        o_vertices.push_back( Vec3( x , y , z ) );

        if( load_normals ) {
            myfile >> x >> y >> z;
            o_normals.push_back( Vec3( x , y , z ) );
        }
    }

    o_triangles.clear();
    o_triangle_normals.clear();
    for( int f = 0 ; f < n_faces ; ++f )
    {
        int n_vertices_on_face;
        myfile >> n_vertices_on_face;

        if( n_vertices_on_face == 3 )
        {
            unsigned int _v1 , _v2 , _v3;
            myfile >> _v1 >> _v2 >> _v3;

            o_triangles.push_back(Triangle( _v1, _v2, _v3 ));

            if( load_normals ) {
                float x , y , z ;
                myfile >> x >> y >> z;
                o_triangle_normals.push_back( Vec3( x , y , z ) );
            }
        }
        else if( n_vertices_on_face == 4 )
        {
            unsigned int _v1 , _v2 , _v3 , _v4;
            myfile >> _v1 >> _v2 >> _v3 >> _v4;

            o_triangles.push_back(Triangle(_v1, _v2, _v3 ));
            o_triangles.push_back(Triangle(_v1, _v3, _v4));
            if( load_normals ) {
                float x , y , z ;
                myfile >> x >> y >> z;
                o_triangle_normals.push_back( Vec3( x , y , z ) );
            }

        }
        else
        {
            std::cout << "We handle ONLY *.off files with 3 or 4 vertices per face" << std::endl;
            myfile.close();
            exit(1);
        }
    }

}

// ------------------------------------
// Application initialization
// ------------------------------------
void initLight () {
    GLfloat light_position1[4] = {22.0f, 16.0f, 50.0f, 0.0f};
    GLfloat direction1[3] = {-52.0f,-16.0f,-50.0f};
    GLfloat color1[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat ambient[4] = {0.3f, 0.3f, 0.3f, 0.5f};

    glLightfv (GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv (GL_LIGHT1, GL_SPOT_DIRECTION, direction1);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, color1);
    glLightfv (GL_LIGHT1, GL_SPECULAR, color1);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHTING);
}

void init () {
    camera.resize (SCREENWIDTH, SCREENHEIGHT);
    initLight ();
    glCullFace (GL_BACK);
    glDisable (GL_CULL_FACE);
    glDepthFunc (GL_LESS);
    glEnable (GL_DEPTH_TEST);
    glClearColor (0.2f, 0.2f, 0.3f, 1.0f);
    glEnable(GL_COLOR_MATERIAL);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glutFullScreen();

    display_normals = false;
    display_mesh = true;
    display_smooth_normals = true;
    displayMode = LIGHTED;
    display_basis = false;
}


// ------------------------------------
// Rendering.
// ------------------------------------

void drawVector( Vec3 const & i_from, Vec3 const & i_to ) {

    glBegin(GL_LINES);
    glVertex3f( i_from[0] , i_from[1] , i_from[2] );
    glVertex3f( i_to[0] , i_to[1] , i_to[2] );
    glEnd();
}

void drawAxis( Vec3 const & i_origin, Vec3 const & i_direction ) {

    glLineWidth(4); // for example...
    drawVector(i_origin, i_origin + i_direction);
}

void drawReferenceFrame( Vec3 const & origin, Vec3 const & i, Vec3 const & j, Vec3 const & k ) {

    glDisable(GL_LIGHTING);
    glColor3f( 0.8, 0.2, 0.2 );
    drawAxis( origin, i );
    glColor3f( 0.2, 0.8, 0.2 );
    drawAxis( origin, j );
    glColor3f( 0.2, 0.2, 0.8 );
    drawAxis( origin, k );
    glEnable(GL_LIGHTING);

}

void drawReferenceFrame( Basis & i_basis ) {
    
    drawReferenceFrame( i_basis.origin, i_basis.i, i_basis.j, i_basis.k );
}

typedef struct {
    float r;       // ??? [0, 1]
    float g;       // ??? [0, 1]
    float b;       // ??? [0, 1]
} RGB;

RGB scalarToRGB( float scalar_value ) //Scalar_value ??? [0, 1]
{
    RGB rgb;
    float H = scalar_value*360., S = 1., V = 0.85,
            P, Q, T,
            fract;

    (H == 360.)?(H = 0.):(H /= 60.);
    fract = H - floor(H);

    P = V*(1. - S);
    Q = V*(1. - S*fract);
    T = V*(1. - S*(1. - fract));

    if      (0. <= H && H < 1.)
        rgb = (RGB){.r = V, .g = T, .b = P};
    else if (1. <= H && H < 2.)
        rgb = (RGB){.r = Q, .g = V, .b = P};
    else if (2. <= H && H < 3.)
        rgb = (RGB){.r = P, .g = V, .b = T};
    else if (3. <= H && H < 4.)
        rgb = (RGB){.r = P, .g = Q, .b = V};
    else if (4. <= H && H < 5.)
        rgb = (RGB){.r = T, .g = P, .b = V};
    else if (5. <= H && H < 6.)
        rgb = (RGB){.r = V, .g = P, .b = Q};
    else
        rgb = (RGB){.r = 0., .g = 0., .b = 0.};

    return rgb;
}

void drawSmoothTriangleMesh( Mesh const & i_mesh , bool draw_field = false ) {
    glBegin(GL_TRIANGLES);
    for(unsigned int tIt = 0 ; tIt < i_mesh.triangles.size(); ++tIt) {

        for(unsigned int i = 0 ; i < 3 ; i++) {
            const Vec3 & p = i_mesh.vertices[i_mesh.triangles[tIt][i]]; //Vertex position
            const Vec3 & n = i_mesh.normals[i_mesh.triangles[tIt][i]]; //Vertex normal

            if( draw_field && mesh_valence_field.size() > 0 ){
                RGB color = scalarToRGB( mesh_valence_field[i_mesh.triangles[tIt][i]] );
                glColor3f( color.r, color.g, color.b );
            }
            glNormal3f( n[0] , n[1] , n[2] );
            glVertex3f( p[0] , p[1] , p[2] );
        }
    }
    glEnd();

}

void drawTriangleMesh( Mesh const & i_mesh , bool draw_field = false  ) {
    glBegin(GL_TRIANGLES);
    for(unsigned int tIt = 0 ; tIt < i_mesh.triangles.size(); ++tIt) {
        const Vec3 & n = i_mesh.triangle_normals[ tIt ]; //Triangle normal
        for(unsigned int i = 0 ; i < 3 ; i++) {
            const Vec3 & p = i_mesh.vertices[i_mesh.triangles[tIt][i]]; //Vertex position

            if( draw_field ){
                RGB color = scalarToRGB( mesh_valence_field[i_mesh.triangles[tIt][i]] );
                glColor3f( color.r, color.g, color.b );
            }
            glNormal3f( n[0] , n[1] , n[2] );
            glVertex3f( p[0] , p[1] , p[2] );
        }
    }
    glEnd();

}

void drawMesh( Mesh const & i_mesh , bool draw_field = false ){
    if(display_smooth_normals)
        drawSmoothTriangleMesh(i_mesh, draw_field) ; //Smooth display with vertices normals
    else
        drawTriangleMesh(i_mesh, draw_field) ; //Display with face normals
}

void drawVectorField( std::vector<Vec3> const & i_positions, std::vector<Vec3> const & i_directions ) {
    glLineWidth(1.);
    for(unsigned int pIt = 0 ; pIt < i_directions.size() ; ++pIt) {
        Vec3 to = i_positions[pIt] + 0.02*i_directions[pIt];
        drawVector(i_positions[pIt], to);
    }
}

void drawNormals(Mesh const& i_mesh){

    if(display_smooth_normals){
        drawVectorField( i_mesh.vertices, i_mesh.normals );
    } else {
        std::vector<Vec3> triangle_baricenters;
        for ( const Triangle& triangle : i_mesh.triangles ){
            Vec3 triangle_baricenter (0.,0.,0.);
            for( unsigned int i = 0 ; i < 3 ; i++ )
                triangle_baricenter += i_mesh.vertices[triangle[i]];
            triangle_baricenter /= 3.;
            triangle_baricenters.push_back(triangle_baricenter);
        }

        drawVectorField( triangle_baricenters, i_mesh.triangle_normals );
    }
}

void drawPoints(std::vector<Vec3> &vertices) {
    
    glColor3f(0.,0.,0.);
    glPointSize(2.);

    glBegin(GL_POINTS);
    
    for (long unsigned int i = 0; i < vertices.size(); i++){
        
        glVertex3fv(vertices[i].mVals);
    }

    glEnd();
}

//Draw fonction
void draw () {

    if(displayMode == LIGHTED || displayMode == LIGHTED_WIRE){

        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);

    }  else if(displayMode == WIRE){

        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        glDisable (GL_LIGHTING);

    }  else if(displayMode == SOLID ){
        glDisable (GL_LIGHTING);
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

    }

    mesh.simplify(resolution);
    drawMesh(mesh, true);
    if(vertex_cube_display) drawPoints(verticesCube);
    
    if(displayMode == SOLID || displayMode == LIGHTED_WIRE){
        glEnable (GL_POLYGON_OFFSET_LINE);
        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth (1.0f);
        glPolygonOffset (-2.0, 1.0);

        glColor3f(0.,0.,0.);
        drawMesh(mesh, false);

        glDisable (GL_POLYGON_OFFSET_LINE);
        glEnable (GL_LIGHTING);
    }

    glDisable(GL_LIGHTING);
    if(display_normals){
        glColor3f(1.,0.,0.);
        drawNormals(mesh);
    }

    if( display_basis ){
        drawReferenceFrame(basis);
    }
    glEnable(GL_LIGHTING);
}

void changeDisplayMode(){
    if(displayMode == LIGHTED)
        displayMode = LIGHTED_WIRE;
    else if(displayMode == LIGHTED_WIRE)
        displayMode = SOLID;
    else if(displayMode == SOLID)
        displayMode = WIRE;
    else
        displayMode = LIGHTED;
}

void display () {
    glLoadIdentity ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply ();
    draw ();
    glFlush ();
    glutSwapBuffers ();
}

void idle () {
    glutPostRedisplay ();
}

// ------------------------------------
// User inputs
// ------------------------------------
//Keyboard event
void key (unsigned char keyPressed, int x, int y) {
    switch (keyPressed) {
        
        case 'q':

            glutDestroyWindow(window);
            exit(EXIT_SUCCESS);
            
            break;

        case 'f':
            
            if (fullScreen == true) {

                glutReshapeWindow(SCREENWIDTH, SCREENHEIGHT);
                fullScreen = false;

            } else {

                glutFullScreen();
                fullScreen = true;
            
            }

            break;

        case 's':

            if(resolution > 1) { 

                resolution--;
                std::cout << "R??solution : " << resolution << std::endl;
            }

            break;

        case 'c':

            if(vertex_cube_display == false) {

                vertex_cube_display = true;
            } else {

                vertex_cube_display = false;
            }

            break;

        case 'b': 
            
            display_basis = !display_basis;
            
            break;

        case 'n':

            display_normals = !display_normals;
            
            break;

        case 'm':

            display_smooth_normals = !display_smooth_normals;
            
            break;

        case 'w':
            
            changeDisplayMode();

            break;

        case 't':

            weight_type++;

            if(weight_type == 3) weight_type = 0;
            
            mesh.computeVerticesNormals();
            
            break;
    }
    
    idle ();
}

//Mouse events
void mouse (int button, int state, int x, int y) {
    if (state == GLUT_UP) {
        mouseMovePressed = false;
        mouseRotatePressed = false;
        mouseZoomPressed = false;
    } else {
        if (button == GLUT_LEFT_BUTTON) {
            camera.beginRotate (x, y);
            mouseMovePressed = false;
            mouseRotatePressed = true;
            mouseZoomPressed = false;
        } else if (button == GLUT_RIGHT_BUTTON) {
            lastX = x;
            lastY = y;
            mouseMovePressed = true;
            mouseRotatePressed = false;
            mouseZoomPressed = false;
        } else if (button == GLUT_MIDDLE_BUTTON) {
            if (mouseZoomPressed == false) {
                lastZoom = y;
                mouseMovePressed = false;
                mouseRotatePressed = false;
                mouseZoomPressed = true;
            }
        }
    }

    idle ();
}

//Mouse motion, update camera
void motion (int x, int y) {
    if (mouseRotatePressed == true) {
        camera.rotate (x, y);
    }
    else if (mouseMovePressed == true) {
        camera.move ((x-lastX)/static_cast<float>(SCREENWIDTH), (lastY-y)/static_cast<float>(SCREENHEIGHT), 0.0);
        lastX = x;
        lastY = y;
    }
    else if (mouseZoomPressed == true) {
        camera.zoom (float (y-lastZoom)/SCREENHEIGHT);
        lastZoom = y;
    }
}


void reshape(int w, int h) {
    camera.resize (w, h);
}

// ------------------------------------
// Start of graphical application
// ------------------------------------
int main (int argc, char ** argv) {
    
    if (argc > 2) {
        exit (EXIT_FAILURE);
    }
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow ("TP5 Simplification");

    init ();
    glutIdleFunc (idle);
    glutDisplayFunc (display);
    glutKeyboardFunc (key);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);

    openOFF("data/elephant_n.off", mesh.vertices, mesh.normals, mesh.triangles, mesh.triangle_normals);    

    basis = Basis();

    std::vector<unsigned int> valences;
    compute_vertex_valences(mesh.vertices, mesh.triangles, valences);

    long unsigned int valences_size = valences.size();
    mesh_valence_field.clear();
    mesh_valence_field.resize(valences_size);
    
    unsigned int max = 0;
    for (long unsigned int i = 0; i < valences_size; i++) {
        
        if (valences[i] > max) max = valences[i];
    }
    
    for (long unsigned int i = 0; i < valences_size; i++){
        
        mesh_valence_field[i] = ((float)valences[i]) / (float)max;
    }

    glutMainLoop ();

    return EXIT_SUCCESS;
}


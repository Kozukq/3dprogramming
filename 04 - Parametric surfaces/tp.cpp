#include <vector>
#include <GL/glut.h>
#include "src/Camera.h"

#define NORMAL_SURFACE 0
#define RULED_SURFACE 1
#define BEZIER_SURFACE 2

// ------------------------------------
// Paramètres pour le TP
// ------------------------------------
uint8_t surface_mode = NORMAL_SURFACE;
std::vector<Vec3> curve_points;
std::vector<Vec3> control_points;
std::vector< std::vector<Vec3> > control_points_map;
unsigned long int nb_u = 100;
unsigned long int nb_v = 20;
std::vector< std::vector<Vec3> > surface_normal;
std::vector< std::vector<Vec3> > surface_ruled;
std::vector< std::vector<Vec3> > surface_bezier;
Vec3 line;



// ------------------------------------
// Paramètres pour OpenGL via GLUT
// ------------------------------------
static GLint window;
static bool fullScreen = true;
static unsigned int SCREENWIDTH = 1600;
static unsigned int SCREENHEIGHT = 900;
static Camera camera;
static bool mouseMovePressed = false;
static bool mouseRotatePressed = false;
static bool mouseZoomPressed = false;
static int lastX = 0, lastY = 0, lastZoom = 0;


// ------------------------------------
// Fonctions définies pour le TP
// ------------------------------------
// Fonction factorielle
int fact(int n) {

    if(n > 0) return n * fact(n-1);

    else return 1;
}

// Fonction de calcul de la représentation d'une courbe de Bézier avec l'usage du polynome de Bernstein
void bezierCurveByBernstein(std::vector<Vec3>& curve_points, const std::vector<Vec3>& control_points, const long n, const long nb_u) {

    float bernstein_polynomial;
    // long nb_control = control_points.size();
    // long n = nb_control - 1;

    curve_points.clear();

    for(float u = 0; u < 1; u += 1 / float(nb_u)) {

        // On initialise le point à la position origine
        Vec3 point = Vec3(0.,0.,0.);

        // Pour chaque point de contrôle on lui ajoute le poto Bernstein
        // for(int i = 0; i <= nb_control; i++) {
        for(int i = 0; i <= n + 1; i++) {

            bernstein_polynomial = ( fact(n) / ((fact(i) * fact(n-i))) ) * pow(u,i) * pow(1 - u,n - i);

            point += bernstein_polynomial * control_points[i]; 

        }

        curve_points.push_back(point);
    }
}

// Fonction de rendu d'une courbe
void drawCurve(const std::vector<Vec3>& curve_points) {

    // Execution d'une instance d'OpenGL avec affichage sous forme de droites
    glBegin(GL_LINE_STRIP);

    // Définition de la couleur d'affichage des points
    glColor3f(1.,0.3,0.);

    // Dessin pour chaque point de la courbe
    for(const Vec3& curve_point : curve_points) {

        glVertex3f(curve_point[X_VALUE],curve_point[Y_VALUE],curve_point[Z_VALUE]);

    }

    // Fin de l'instance
    glEnd();
}

// Génération d'une surface projetée par translation d'une courbe
void projectedSurface(std::vector< std::vector<Vec3> >& surface_points, const std::vector<Vec3>& bezier_curve_points, const Vec3& line, const long nb_u) {

    long v_id = 0;

    // On parcourt chaque point de la courbe de Bézier
    for(const Vec3& bezier_curve_point : bezier_curve_points) {

        // On ajoute le point initial de la courbe à la surface
        surface_points.push_back(std::vector<Vec3>());
        surface_points[v_id].push_back(bezier_curve_point);

        // Pour chaque point on définit une translation de point le long de la droite
        for(float u = 0; u < 1; u += (float)(1 / (float)nb_u)) {

            // Translation
            Vec3 translated_point = Vec3(u * line) + bezier_curve_point;

            // On ajoute le point translaté de la courbe à la surface
            surface_points[v_id].push_back(translated_point);
        }

        v_id++;
    }
}

// Génération d'une surface réglée
void ruledSurface(std::vector< std::vector<Vec3> >& surface_points) {}

// Génération d'une surface de Bézier
void bezierSurface(std::vector< std::vector<Vec3> >& surface_points, const std::vector< std::vector<Vec3> >& control_points_map, const long nb_u, const long nb_v) {

    long n;
    std::vector< std::vector<Vec3> > directors;

    for(unsigned long int i = 0; i < control_points.size(); i++) {

        std::vector<Vec3> directing_curve;

        n = control_points_map[i].size() - 1;
        
        bezierCurveByBernstein(directing_curve,control_points_map[i],n,nb_u);
        
        directors.push_back(directing_curve);
    }

    for(unsigned long int i = 0; i < directors[0].size(); i++) {

        std::vector<Vec3> generating_curve;

        for(unsigned long int j = 0; j < control_points.size(); j++) {

            generating_curve.push_back(directors[j][i]);
        }

        std::vector<Vec3> curve_points;

        n = generating_curve.size() - 1;

        bezierCurveByBernstein(curve_points,generating_curve,n,nb_v);

        surface_points.push_back(curve_points);
    }
}

void drawSurface(const std::vector< std::vector<Vec3> >& surface_points, const unsigned long int nb_u, const unsigned long int nb_v) {

    glBegin(GL_POINTS);

    switch(surface_mode) {

        case NORMAL_SURFACE:

            glColor3f(1.,1.,1.);
            break;

        case RULED_SURFACE:

            glColor3f(1.,0.,0.);
            break;

        case BEZIER_SURFACE:

            glColor3f(0.,1.,0.);
            break;
    }

    // Rendu des "méridiens" à la courbe
    for(long unsigned int v = 0; v < nb_v; v++) {

        for(long unsigned int u = 0; u < nb_u; u++) {

            glVertex3f(surface_points[u][v][X_VALUE],surface_points[u][v][Y_VALUE],surface_points[u][v][Z_VALUE]);

        }

    } 

    // Rendu des "latitudes" à la courbe
    for(long unsigned int u = 0; u < nb_u; u++) {

        for(long unsigned int v = 0; v < nb_v; v++) {

            glVertex3f(surface_points[u][v][X_VALUE],surface_points[u][v][Y_VALUE],surface_points[u][v][Z_VALUE]);

        }

    }  

    glEnd();
}

// Fonction de rendu
void draw() {

    switch(surface_mode) {

        case NORMAL_SURFACE:

            bezierCurveByBernstein(curve_points,control_points,control_points.size() - 1,nb_u);
            projectedSurface(surface_normal,curve_points,line,nb_u);
            drawSurface(surface_normal,nb_u,nb_v);
            break;

        case RULED_SURFACE:

            bezierCurveByBernstein(curve_points,control_points,control_points.size() - 1,nb_u);
            projectedSurface(surface_normal,curve_points,line,nb_u);
            drawSurface(surface_normal,nb_u,nb_v);
            break;

        case BEZIER_SURFACE:

            bezierSurface(surface_bezier,control_points_map,nb_u,nb_v);
            drawSurface(surface_bezier,nb_u,nb_v);
            break;
    }
}


// ------------------------------------
// Fonctions liées à OpenGL
// ------------------------------------
// Fonction d'initialisation du rendu de la lumière
void initLight() {

    GLfloat light_position1[4] = { 22.0f, 16.0f, 50.0f, 0.0f };
    GLfloat direction1[3] = { -52.0f, -16.0f, -50.0f };
    GLfloat color1[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat ambient[4] = { 1., 1., 1., 1. };

    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, direction1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, color1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, color1);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);
}

// Fonction d'initialisation générale
void init(int* argc, char* argv[]) {

	glutInit(argc,argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(SCREENWIDTH,SCREENHEIGHT);
    window = glutCreateWindow("Modelisation - TP4 Surfaces parametriques");
    glutFullScreen();
    
    camera.resize(SCREENWIDTH,SCREENHEIGHT);
    initLight();

    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f,0.1f,0.1f,1.0f);
    glEnable(GL_COLOR_MATERIAL);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

// Callback de gestion de l'affichage
void display() {
    
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply();
    draw();
    glFlush();
    glutSwapBuffers();
}

// Callback de gestion des actions souris
void mouse(int button, int state, int x, int y) {

    if(state == GLUT_UP) {
        
        mouseMovePressed = false;
        mouseRotatePressed = false;
        mouseZoomPressed = false;
    
    } else {
        
        if(button == GLUT_LEFT_BUTTON) {
            
            camera.beginRotate(x, y);
            mouseMovePressed = false;
            mouseRotatePressed = true;
            mouseZoomPressed = false;
        
        } else if(button == GLUT_RIGHT_BUTTON) {
            
            lastX = x;
            lastY = y;
            mouseMovePressed = true;
            mouseRotatePressed = false;
            mouseZoomPressed = false;
        
        } else if(button == GLUT_MIDDLE_BUTTON) {
            
            if(mouseZoomPressed == false) {
                
                lastZoom = y;
                mouseMovePressed = false;
                mouseRotatePressed = false;
                mouseZoomPressed = true;
            
            }
        }
    }

    glutPostRedisplay();
}

// Callback de gestion des entrées clavier
void key(unsigned char keyPressed, int x, int y) {
    
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

        case 'm':

            if(surface_mode < 2) {

                surface_mode++;

            } else {

                surface_mode = NORMAL_SURFACE;

            }

	    default:

	        break;

    }

    glutPostRedisplay();
}

// Callback de gestion du mouvement de la caméra
void motion(int x, int y) {
    
    if(mouseRotatePressed == true) {
        
        camera.rotate(x, y);
    
    } else if(mouseMovePressed == true) {
        
        camera.move ((x - lastX) / static_cast<float> (SCREENWIDTH),(lastY - y) / static_cast<float> (SCREENHEIGHT),0.0);
        lastX = x;
        lastY = y;
    
    } else if (mouseZoomPressed == true) {
        
        camera.zoom (float (y-lastZoom)/SCREENHEIGHT);
        lastZoom = y;
    }
}

// Callback de gestion du scaling du FoV de la caméra
void reshape(int w, int h) {
    
    camera.resize(w, h);
}

// Callback de gestion de l'attente
void idle() {

	glutPostRedisplay();
}


// ------------------------------------
// Exécution
// ------------------------------------
int main (int argc, char* argv[]) {

    if(argc > 2) exit(EXIT_FAILURE);

    init(&argc,argv);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(key);
    glutMotionFunc(motion);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    // Initialisation pour la surface balayée
    control_points.push_back(Vec3(0.,0.,0.));
    control_points.push_back(Vec3(1.,0.,0.));
    control_points.push_back(Vec3(1.,1.,0.));
    control_points.push_back(Vec3(1.,1.,1.));
    control_points.push_back(Vec3(0.,1.,0.));

    line[X_VALUE] = 2.;
    line[Y_VALUE] = 2.;
    line[Z_VALUE] = 2.;

    // Initialisation pour la surface de Bézier
    control_points_map.push_back(std::vector<Vec3>());
    control_points_map.push_back(std::vector<Vec3>());
    control_points_map.push_back(std::vector<Vec3>());
    control_points_map.push_back(std::vector<Vec3>());
    control_points_map.push_back(std::vector<Vec3>());

    control_points_map[0].push_back(Vec3(0.,0.,0.));
    control_points_map[0].push_back(Vec3(0.,0.,0.));
    control_points_map[0].push_back(Vec3(0.,0.,0.));
    control_points_map[0].push_back(Vec3(0.,0.,0.));
    control_points_map[0].push_back(Vec3(0.,0.,0.));

    control_points_map[1].push_back(Vec3(0.,0.,0.));
    control_points_map[1].push_back(Vec3(0.,1.,0.));
    control_points_map[1].push_back(Vec3(0.,2.,0.));
    control_points_map[1].push_back(Vec3(0.,1.,0.));
    control_points_map[1].push_back(Vec3(0.,0.,0.));

    control_points_map[2].push_back(Vec3(0.,0.,0.));
    control_points_map[2].push_back(Vec3(0.,2.,0.));
    control_points_map[2].push_back(Vec3(0.,3.,0.));
    control_points_map[2].push_back(Vec3(0.,2.,0.));
    control_points_map[2].push_back(Vec3(0.,0.,0.));

    control_points_map[3].push_back(Vec3(0.,0.,0.));
    control_points_map[3].push_back(Vec3(0.,1.,0.));
    control_points_map[3].push_back(Vec3(0.,2.,0.));
    control_points_map[3].push_back(Vec3(0.,1.,0.));
    control_points_map[3].push_back(Vec3(0.,0.,0.));

    control_points_map[4].push_back(Vec3(0.,0.,0.));
    control_points_map[4].push_back(Vec3(0.,0.,0.));
    control_points_map[4].push_back(Vec3(0.,0.,0.));
    control_points_map[4].push_back(Vec3(0.,0.,0.));
    control_points_map[4].push_back(Vec3(0.,0.,0.));


    glutMainLoop();

    return EXIT_SUCCESS;
}
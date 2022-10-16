#ifndef MESH_H
#define MESH_H


#include <vector>
#include <string>
#include "Vec3.h"
#include "Skeleton.h"

#include <cmath>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#else
#include <GL/glut.h>
#endif

// -------------------------------------------
// Basic Mesh class
// -------------------------------------------

struct MeshVertex {
    inline MeshVertex () {
        w.clear();
    }
    inline MeshVertex (const Vec3 & _p, const Vec3 & _n) : p (_p), n (_n) {
        w.clear();
    }
    inline MeshVertex (const MeshVertex & vertex) : p (vertex.p), n (vertex.n) , w(vertex.w) {
    }
    inline virtual ~MeshVertex () {}
    inline MeshVertex & operator = (const MeshVertex & vertex) {
        p = vertex.p;
        n = vertex.n;
        w = vertex.w;
        return (*this);
    }
    // membres :
    Vec3 p; // une position
    Vec3 n; // une normale
    std::vector< double > w; // skinning weights
};

struct MeshTriangle {
    inline MeshTriangle () {
        v[0] = v[1] = v[2] = 0;
    }
    inline MeshTriangle (const MeshTriangle & t) {
        v[0] = t.v[0];   v[1] = t.v[1];   v[2] = t.v[2];
    }
    inline MeshTriangle (unsigned int v0, unsigned int v1, unsigned int v2) {
        v[0] = v0;   v[1] = v1;   v[2] = v2;
    }
    inline virtual ~MeshTriangle () {}
    inline MeshTriangle & operator = (const MeshTriangle & t) {
        v[0] = t.v[0];   v[1] = t.v[1];   v[2] = t.v[2];
        return (*this);
    }
    // membres :
    unsigned int v[3];
};




class Mesh {
public:
    std::vector<MeshVertex> V;
    std::vector<MeshTriangle> T;

    void loadOFF (const std::string & filename);
    void recomputeNormals ();


    void computeSkinningWeights( Skeleton const & skeleton ) {
        //---------------------------------------------------//
        //---------------------------------------------------//
        // code to change :

        // Indications:
        // you should compute weights for each vertex w.r.t. the skeleton bones
        // so each vertex will have B weights (B = number of bones)
        // these weights shoud be stored in vertex.w:

        unsigned int n = 5;

        for(int j = 0; j < V.size(); j++) {
            
            MeshVertex& vertex = V[j];

            vertex.w.resize(skeleton.bones.size());

            float weightSum = 0.;

            for(int i = 0; i < skeleton.bones.size(); i++) {

                // Bone : a-b, Vertex : v
                const Bone& bone = skeleton.bones[i];

                Vec3 a = skeleton.articulations[bone.joints[0]].p;
                Vec3 b = skeleton.articulations[bone.joints[1]].p;
                Vec3 v = vertex.p;
             
                Vec3 av = v - a;
                Vec3 ab = b - a;

                ab.normalize();

                float leftProjectionScalar = Vec3::dot(av,ab);

                // Le sommet est projeté avant l'os
                if(leftProjectionScalar <= 0) {

                    vertex.w[i] = pow(1 / av.length(),n);
                }

                else {

                    Vec3 bv = v - b;
                    Vec3 ba = a - b;

                    float rightProjectionScalar = Vec3::dot(bv,ba);

                    // Le sommet est projeté après l'os
                    if(rightProjectionScalar <= 0) {

                        vertex.w[i] = pow(1 / bv.length(),n);
                    }

                    // Le sommet est projeté sur l'os
                    else {

                        Vec3 projection = a + leftProjectionScalar * ab; 
                        vertex.w[i] = pow(1 / Vec3::euclideanDistance(v,projection),n);
                    }
                }

                weightSum += vertex.w[i];
            }

            // Normalisation du poids
            for(int i = 0; i < skeleton.bones.size(); i++) {

                vertex.w[i] /= weightSum;
            }
        }

        //---------------------------------------------------//
        //---------------------------------------------------//
        //---------------------------------------------------//
    }

    Vec3 scalarToRGB(float scalar_value) const // Scalar_value ∈ [0, 1]
    {
        Vec3 rgb;
        float H = scalar_value * 360., S = 1., V = 0.85,
              P, Q, T,
              fract;

        (H == 360.) ? (H = 0.) : (H /= 60.);
        fract = H - floor(H);

        P = V * (1. - S);
        Q = V * (1. - S * fract);
        T = V * (1. - S * (1. - fract));

        if (0. <= H && H < 1.) {
            rgb[0] = V;
            rgb[1] = T;
            rgb[2] = P;
        }
        else if (1. <= H && H < 2.) {
            rgb[0] = Q;
            rgb[1] = V;
            rgb[2] = P;
        }
        else if (2. <= H && H < 3.) {
            rgb[0] = P;
            rgb[1] = V;
            rgb[2] = T;
        }
        else if (3. <= H && H < 4.) {
            rgb[0] = P;
            rgb[1] = Q;
            rgb[2] = V;
        }
        else if (4. <= H && H < 5.) {
            rgb[0] = T;
            rgb[1] = P;
            rgb[2] = V;
        }
        else if (5. <= H && H < 6.) {
            rgb[0] = V;
            rgb[1] = P;
            rgb[2] = Q;
        }
        else {
            rgb[0] = 0.0;
            rgb[1] = 0.0;
            rgb[2] = 0.0;
        }

        return rgb;
    }

    void draw(int displayedBone) const {
        glEnable(GL_LIGHTING);
        glBegin (GL_TRIANGLES);
        for (unsigned int i = 0; i < T.size (); i++)
            for (unsigned int j = 0; j < 3; j++) {
                const MeshVertex & v = V[T[i].v[j]];
                glNormal3f (v.n[0], v.n[1], v.n[2]);
                glVertex3f (v.p[0], v.p[1], v.p[2]);
                Vec3 color = scalarToRGB(v.w[displayedBone]);
                glColor3f(color[0],color[1],color[2]);
            }
        glEnd ();
    }

    void drawTransformedMesh( SkeletonTransformation & transfo ) const {
        std::vector< Vec3 > newPositions( V.size() );

        //---------------------------------------------------//
        //---------------------------------------------------//
        // code to change :
        for( unsigned int i = 0 ; i < V.size() ; ++i ) {

            Vec3 p = V[i].p;
            Vec3 pp = Vec3(0.,0.,0.);

            // Indications:
            // you should use the skinning weights to blend the transformations of the vertex position by the bones.

            for(int j = 0; j < transfo.boneTransformations.size(); j++) {

                pp += V[i].w[j] * (transfo.boneTransformations[j].worldSpaceRotation * p + transfo.boneTransformations[j].worldSpaceTranslation);
            }

            newPositions[i] = pp;
        }
        //---------------------------------------------------//
        //---------------------------------------------------//
        //---------------------------------------------------//

        glEnable(GL_LIGHTING);
        glBegin (GL_TRIANGLES);
        for (unsigned int i = 0; i < T.size (); i++)
            for (unsigned int j = 0; j < 3; j++) {
                const MeshVertex & v = V[T[i].v[j]];
                Vec3 p = newPositions[ T[i].v[j] ];
                glNormal3f (v.n[0], v.n[1], v.n[2]);
                glVertex3f (p[0], p[1], p[2]);
            }
        glEnd ();
    }
};



#endif

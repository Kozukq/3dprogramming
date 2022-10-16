#version 450 core // Minimal GL version support expected from the GPU

#define NB_LIGHTS 2

struct LightSource {
    int isActive;
    vec3 position;
    vec3 color;
    float intensity;
    mat4 depthMVP;
};

struct Material {
    vec3 albedo;
    float shininess;
};

uniform mat4 projectionMat, modelViewMat, normalMat;
uniform LightSource lightSources[NB_LIGHTS];
uniform sampler2D shadowMap[NB_LIGHTS];
uniform Material material;

in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fPositionWorldSpace;
in vec3 fNormal;
in vec2 fTexCoord;

out vec4 colorResponse; // Shader output: the color response attached to this fragment

float computeShadow(vec4 shadowCoordinates, int index) {

    float bias = 0.01;

    // Perspective divide
    vec3 projectionCoordinates = shadowCoordinates.xyz / shadowCoordinates.w;

    // Normalisation dans [0,1]
    projectionCoordinates = projectionCoordinates * 0.5 + 0.5;

    // Depuis la lumière, valeur de profondeur la plus proche 
    float closestDepth = texture(shadowMap[index],projectionCoordinates.xy).r;

    // Depuis la lumière, valeur de profondeur du fragment
    float currentDepth = projectionCoordinates.z;

    // Vérification si la position du fragment est dans l'ombre
    float shadow = currentDepth <= (closestDepth + bias) ? 1.0 : 0.0;

    return shadow;
}

void main() {

    float pi = 3.1415927;
    float shadow = 0.0;

    vec3 n = normalize(fNormal);

    // Linear barycentric interpolation does not preserve unit vectors
    vec3 wo = normalize(-fPosition); // unit vector pointing to the camera
    vec3 radiance = vec3(0,0,0);

    if(dot(n,wo) >= 0.0) {

        for(int i = 0; i < NB_LIGHTS; i++) {

            if(lightSources[i].isActive == 1) { // WE ONLY CONSIDER LIGHTS THAT ARE SWITCHED ON

                vec3 wi = normalize(vec3((modelViewMat * vec4(lightSources[i].position,1)).xyz) - fPosition ); // unit vector pointing to the light source (change if you use several light sources!!!)
               
                if(dot(wi,n) >= 0.0) { // WE ONLY CONSIDER LIGHTS THAT ARE ON THE RIGHT HEMISPHERE (side of the tangent plane)
                    
                    vec3 wh = normalize(wi + wo); // half vector (if wi changes, wo should change as well)
                    
                    vec3 Li = lightSources[i].color * lightSources[i].intensity;

                    // Calcul des coordonnées d'ombrage
                    vec4 shadowCoordinates = lightSources[i].depthMVP * vec4(fPositionWorldSpace,1.0);

                    // Calcul de l'ombrage
                    shadow += computeShadow(shadowCoordinates,i);

                    radiance = radiance + Li * material.albedo * (max(dot(n,wi),0.0) + pow(max(dot(n,wh),0.0),material.shininess));
                }
            }
        }
    }

    colorResponse = shadow / NB_LIGHTS * vec4(radiance,1.0); // Building an RGBA value from an RGB one.
}

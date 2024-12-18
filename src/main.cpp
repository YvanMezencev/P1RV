// P1RV.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <Windows.h>
#include <iostream>

//#include <glew.h>
#include <GL/gl.h>
#include <GLUT.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image/stb_image_write.h"

using namespace std;

// Rotations autour de X et Y
float cameraX = 0.0f;
float cameraY = 200.0f;  // Height of the camera
float cameraZ = 0.0f;
GLfloat angleX = 0.0f;     // Horizontal angle
GLfloat angleY = 0.0f;     // Vertical angle
float cameraDistance = 200.0f;

GLfloat cameraLookAt[3] = { 0.0f, 0.0f, 0.0f }; // Camera looks at the center
GLfloat cameraPosition[3] = { cameraX, cameraY, cameraZ }; // Initial camera position
GLint oldX = 0;
GLint oldY = 0;
GLboolean boutonClick = false;

float cameraSpeed = 5.0f;  // Movement speed

int windowW = 640;
int windowH = 480;
float focale = 65.0f;
float near_p = 0.1f;
float far_p = 1000.0f;

const float PI = 3.14159265359f;

vector<float> load_triangle_vertices(const char* file_path, int& height, int& width, int& channels) {
    //Chargement de l'image heightmap
    unsigned char* img = stbi_load(file_path, &width, &height, &channels, 1);

    if (img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    cout << "Image chargee.\nWidth: " << width << "px, Height: " << height << "px, Channels: " << channels << endl;

    //Creation du mesh
    vector<float> vertices;
    float yScale = 512.0f / 256.0f, yShift = 0.0f;
    float min_hauteur = 1e9;
    float max_hauteur = -1e9;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {

            unsigned char* texel = img + (width * i) + j;
            float hauteur = *texel;
            hauteur = hauteur * yScale - yShift;

            vertices.push_back(-(float)width / 2 + j); //Coordonnée X
            vertices.push_back(hauteur); //Coordonnée Y
            vertices.push_back(-(float)height / 2 + i); //Coordonnée Z

            //Mise à jour des valeurs minimales et maximales
            if (hauteur > max_hauteur) {
                max_hauteur = hauteur;
            }
            if (hauteur < min_hauteur) {
                min_hauteur = hauteur;
            }
        }
    }
    //Affichage des informations sur le mesh
    cout << "\nVecteur Vertices acquis.\nTaille du vecteur: " << vertices.size() << " (On est cense obtenir width*height*3)" << endl;
    cout << "Altitude minimale: " << min_hauteur << endl;
    cout << "Altitude maximale: " << max_hauteur << endl;

    stbi_image_free(img); //On libère la mémoire
    return vertices;
}

vector<unsigned int> generate_indices(const int& width, const int& height) {
    // Génération des indices
    vector<unsigned int> indices;
    for (int i = 0; i < height - 1; i++)       // Pour chaque rangée
    {
        for (int j = 0; j < width; j++)      // Pour chaque colonne
        {
            for (int k = 0; k < 2; k++)      // Pour chaque cotée de la rangée (haut ou bas)
            {
                indices.push_back(j + width * (i + k));
            }
        }
    }
    return indices;
}

void calculateCameraDirections(float& forwardX, float& forwardY, float& forwardZ,
    float& rightX, float& rightY, float& rightZ) {
    // Convert angles to radians
    float radAngleX = angleX * 3.14159265359f / 180.0f; // Horizontal rotation (yaw)
    float radAngleY = angleY * 3.14159265359f / 180.0f; // Vertical rotation (pitch)

    // Forward vector (based on pitch and yaw)
    forwardX = cos(radAngleY) * sin(radAngleX);
    forwardY = -sin(radAngleY);
    forwardZ = -cos(radAngleY) * cos(radAngleX);

    // Right vector (perpendicular to the forward vector, ignoring pitch)
    rightX = sin(radAngleX - 3.14159265359f / 2.0f);
    rightY = 0.0f;
    rightZ = -cos(radAngleX - 3.14159265359f / 2.0f);
}

// Fonction de rappel de la souris
GLvoid souris(int bouton, int etat, int x, int y) {
    if (bouton == GLUT_LEFT_BUTTON && etat == GLUT_DOWN) {
        boutonClick = true;
        oldX = x;
        oldY = y;
    }
    else if (bouton == GLUT_LEFT_BUTTON && etat == GLUT_UP) {
        boutonClick = false;
    }
}

GLvoid deplacementSouris(int x, int y) {
    if (boutonClick) {
        float sensitivity = 0.1f; // Adjust sensitivity as needed
        angleX += (x - oldX) * sensitivity;
        angleY += (y - oldY) * sensitivity;

        // Clamp pitch to avoid flipping
        if (angleY > 89.0f) angleY = 89.0f;
        if (angleY < -89.0f) angleY = -89.0f;

        // Update the old mouse position
        oldX = x;
        oldY = y;

        // Update the camera look-at vector
        float forwardX, forwardY, forwardZ;
        calculateCameraDirections(forwardX, forwardY, forwardZ, forwardX, forwardY, forwardZ);
        cameraLookAt[0] = cameraX + forwardX;
        cameraLookAt[1] = cameraY + forwardY;
        cameraLookAt[2] = cameraZ + forwardZ;

        glutPostRedisplay();
    }
}

void clavier(unsigned char key, int x, int y) {
    float moveSpeed = 5.0f; // Adjust speed as needed

    // Calculate direction vectors
    float forwardX, forwardY, forwardZ;
    float rightX, rightY, rightZ;
    calculateCameraDirections(forwardX, forwardY, forwardZ, rightX, rightY, rightZ);

    // Move based on key
    if (key == 'z') { // Move forward
        cameraX += forwardX * moveSpeed;
        cameraY += forwardY * moveSpeed;
        cameraZ += forwardZ * moveSpeed;
    }
    if (key == 's') { // Move backward
        cameraX -= forwardX * moveSpeed;
        cameraY -= forwardY * moveSpeed;
        cameraZ -= forwardZ * moveSpeed;
    }
    if (key == 'q') { // Move left
        cameraX += rightX * moveSpeed;
        cameraY += rightY * moveSpeed;
        cameraZ += rightZ * moveSpeed;
    }
    if (key == 'd') { // Move right
        cameraX -= rightX * moveSpeed;
        cameraY -= rightY * moveSpeed;
        cameraZ -= rightZ * moveSpeed;
    }

    // Update the camera look-at vector to maintain orientation
    cameraLookAt[0] = cameraX + forwardX;
    cameraLookAt[1] = cameraY + forwardY;
    cameraLookAt[2] = cameraZ + forwardZ;

    // Redraw the scene
    glutPostRedisplay();
}

// Callback de redimensionnement de la fenêtre
GLvoid redimensionner(int w, int h) {
    // Garde les valeurs
    windowW = w;
    windowH = h;
    // eviter une division par 0
    if (windowH == 0)
        windowH = 1;

    float ratio = (float)windowW / (float)windowH;
    std::cout << "Ratio : " << ratio << std::endl;

    // Projection
    glMatrixMode(GL_PROJECTION);

    // Resetting matrix
    glLoadIdentity();

    // Viewport
    // // TODO Essayez de modifier l'appel à glViewport
    // en changeant les parametre d'appel a la fonction mais
    // tout en obtenant le meme resultat
    glViewport(0, 0, windowW, windowH);

    // Mise en place de la perspective
    // TODO : peut-on changerle ratio ici pour un meilleur resultat ?
    gluPerspective(focale, (float)windowW / (float)windowH, near_p, far_p);

   

    // Placement de la caméra
    gluLookAt(0, 200, 200, 0, 0, 0, 0, 1, 0);

    // Retourne a la pile modelview
    glMatrixMode(GL_MODELVIEW);
}

GLvoid draw_map() {
    //static const char* file_path = "heightmaps/heightmap_1.png";
    static const char* file_path = "C:/Users/Eleve/source/repos/P1RV_Project/heightmaps/peppers_128.png"; //Change this path to your path to the heightmap you want to load
    static int width, height, channels;
    static vector<float> vertices = load_triangle_vertices(file_path, width, height, channels);
    static vector<unsigned int> indices = generate_indices(width, height);

    // Effacement du frame buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Calculate the camera direction vector
    float radPitch = angleY * 3.14159265359f / 180.0f; // Convert pitch to radians
    float radYaw = angleX * 3.14159265359f / 180.0f;   // Convert yaw to radians

    float dirX = cos(radPitch) * sin(radYaw);
    float dirY = sin(radPitch);
    float dirZ = -cos(radPitch) * cos(radYaw);

    gluLookAt(
        cameraX, cameraY, cameraZ,                 // Camera position
        cameraX + dirX, cameraY + dirY, cameraZ + dirZ, // Look at position
        0.0f, 1.0f, 0.0f                          // Up vector
    );

    for (unsigned int i = 0; i < indices.size() - 2; i = i + 3) {
        glBegin(GL_TRIANGLES);
        for (int j = 0; j < 3; j++) {
            float x = vertices[indices[i + j] * 3];
            float y = vertices[(indices[i + j] * 3) + 1];
            float z = vertices[(indices[i + j] * 3) + 2];
            glColor3f(1.0, 1.0, 1.0);
            glVertex3f(x, y, z);
            //cout << "Vertex drawn at " << x <<" " << y << " " << z << endl;
        }
        glEnd();
    }
    glFlush();
    glutSwapBuffers();
    cout << "Map drawn; AngleX: " << angleX << "; AngleY: " << angleY << endl;
}

int main(int argc, char** argv)
{
    // Initialisation de GLUT
    glutInit(&argc, argv);
    // Choix du mode d'affichage (ici RVB)
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    // Position initiale de la fenetre GLUT
    glutInitWindowPosition(200, 200);
    // Taille initiale de la fenetre GLUT
    glutInitWindowSize(windowW, windowH);
    // Creation de la fenetre GLUT
    glutCreateWindow("HeightMap");
    // Définition de la couleur d'effacement du framebuffer
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Définition des fonctions de callbacks
    glutDisplayFunc(draw_map);
    glutKeyboardFunc(clavier);
    glutMouseFunc(souris);
    glutMotionFunc(deplacementSouris);
    glutReshapeFunc(redimensionner);

    //const unsigned int NUM_STRIPS = height - 1;     //Nombre de rangées de triangles
    //const unsigned int NUM_VERTS_PER_STRIP = width * 2;     //Nombre de vertex par rangée

    //cout << "Nombre de rangees de triangles a afficher: " << NUM_STRIPS << endl;
    //cout << "Nombre de sommet par rangee de triangles: " << NUM_VERTS_PER_STRIP << endl;

    glutMainLoop();



    return 0;
}

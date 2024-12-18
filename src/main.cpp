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

GLfloat angleX = 0.0f;     // Horizontal angle
GLfloat angleY = 0.0f;     // Vertical angle
float cameraDistance = 200.0f;

float posX = 200.0f, posY = 200.0f, posZ = 5.0f; // Position de la caméra
bool boutonClick = false;
int oldX = 0, oldY = 0; // Anciennes positions de la souris

float movementSpeed = 5.0f;  // Movement speed
float rotationSpeed = 0.2f; //Rotation speed

int windowW = 640;
int windowH = 480;
float focale = 65.0f;
float near_p = 0.1f;
float far_p = 1000.0f;

const float M_PI = 3.14159265359f;

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
        int deltaX = x - oldX;
        int deltaY = y - oldY;

        angleX += deltaX * rotationSpeed; // Sensibilité de la souris
        angleY += deltaY * rotationSpeed;

        // Limiter l'angle Y pour éviter de regarder complètement en haut/bas
        if (angleY > 89.0f) angleY = 89.0f;
        if (angleY < -89.0f) angleY = -89.0f;

        oldX = x;
        oldY = y;
    }
    glutPostRedisplay();
}

GLvoid clavier(unsigned char key, int x, int y) {
    float radAngleX = angleX * M_PI / 180.0f; // Conversion en radians
    float radAngleY = angleY * M_PI / 180.0f;

    switch (key) {
    case 'z': // Avancer
        posX += sin(radAngleX) * movementSpeed;
        posZ -= cos(radAngleX) * movementSpeed;
        break;
    case 's': // Reculer
        posX -= sin(radAngleX) * movementSpeed;
        posZ += cos(radAngleX) * movementSpeed;
        break;
    case 'q': // Aller à gauche
        posX -= cos(radAngleX) * movementSpeed;
        posZ -= sin(radAngleX) * movementSpeed;
        break;
    case 'd': // Aller à droite
        posX += cos(radAngleX) * movementSpeed;
        posZ += sin(radAngleX) * movementSpeed;
        break;
    case 'e': // Monter
        posY += movementSpeed;
        break;
    case 'c': // Descendre
        posY -= movementSpeed;
        break;
    case 27: // Échap pour quitter
        exit(0);
        break;
    }
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

    float radAngleX = angleX * M_PI / 180.0f;
    float radAngleY = angleY * M_PI / 180.0f;

    // Calcul de la direction de vue
    float dirX = cos(radAngleY) * sin(radAngleX);
    float dirY = sin(radAngleY);
    float dirZ = -cos(radAngleY) * cos(radAngleX);

    // Placement de la caméra
    gluLookAt(
        posX, posY, posZ,           // Position de la caméra
        posX + dirX, posY + dirY, posZ + dirZ, // Direction de la vue
        0.0f, 1.0f, 0.0f            // Vecteur "up" (Y positif)
    );

    // Retourne a la pile modelview
    glMatrixMode(GL_MODELVIEW);
}

GLvoid draw_map() {
    //static const char* file_path = "heightmaps/heightmap_1.png";
    static const char* file_path = "C:/Users/Eleve/source/repos/P1RV_Project/heightmaps/heightmap_2.png"; //Change this path to your path to the heightmap you want to load
    static int width, height, channels;
    static vector<float> vertices = load_triangle_vertices(file_path, width, height, channels);
    static vector<unsigned int> indices = generate_indices(width, height);

    // Effacement du frame buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float radAngleX = angleX * M_PI / 180.0f;
    float radAngleY = angleY * M_PI / 180.0f;

    // Calcul de la direction de vue
    float dirX = cos(radAngleY) * sin(radAngleX);
    float dirY = -sin(radAngleY);
    float dirZ = -cos(radAngleY) * cos(radAngleX);

    // Calcul de la position de "lookAt"
    gluLookAt(
        posX, posY, posZ,           // Position de la caméra
        posX + dirX, posY + dirY, posZ + dirZ, // Direction de la vue
        0.0f, 1.0f, 0.0f            // Vecteur "up" (Y positif)
    );

    glBegin(GL_TRIANGLES);
    for (unsigned int i = 0; i < indices.size() - 2; i = i + 3) {
        for (int j = 0; j < 3; j++) {
            float x = vertices[indices[i + j] * 3];
            float y = vertices[(indices[i + j] * 3) + 1];
            float z = vertices[(indices[i + j] * 3) + 2];
            glColor3f(1.0, 1.0, 1.0);
            glVertex3f(x, y, z);
            //cout << "Vertex drawn at " << x <<" " << y << " " << z << endl;
        }
    }
    glEnd();
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

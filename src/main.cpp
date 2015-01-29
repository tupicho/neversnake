#ifdef __APPLE__
#include <GLUT/glut.h>
#else

#include <GL/glut.h>

#endif

#include <iostream>
#include <sstream>
#include <string.h>
#include <math.h>

#include "../cabeceras/image.h"
#include "../cabeceras/snake.h"

Snake *player;

using namespace std;

/*
   Windows
******************************/

// Límites del viewport
double minX = -1.0, maxX = 1.0;
double minY = -1.0, maxY = 1.0;

int nivel = 1;

// Ancho y alto de la ventana
double width = 640;
double height = 480;

//struct para obstaculos
struct atributos {
    int f = 0; //fila 48
    int c = 0; //colunma 64
    int s = 0; //select 2
    int l = 0; //largo 5
};
//n matriz que se usara para crear y mantener las coordenadas y atributos de los obstaculos
atributos n[9];
int i = 0, j = 0, k = 0, aux1 = 0, z = 0, k1 = 0, aux2 = 0, aux3 = 0;
/*
 Unidades de movimiento
 ******************************/

// Tamaño de cada unidad de espacio
double unitSize = 10;

// Unidades por fila y por columna
int unitsPerRow = width / unitSize;
int unitsPerCol = height / unitSize;

int red, green, blue = 0;
double radio = 0;
time_t start = time(0);
time_t traspasaTimeIni = time(0);

int isTime = 1;
int traspasarParedes = 1;
/*
 Variables compartidas 2D/3D
 ******************************/

// Marcador
int score = 0;
int scoreMultiplier = 1;

// Timer
double timerTick = 65;
double speed = 1;

// Dirección de movimiento
int dirX = 1; // X=1 Derecha, X=-1 Izq.
int dirY = 0; // Y=1 Arriba,  Y=-1 Abajo

// Flag para desplegar el mapa
bool showMap = false;

// Flag para tipo de serpiente
int snakeShape = 0;

// Flag para manzana
int appleFlag = 0;
int specialApple = 0;

// Guarda nombre de la textura
static GLuint texName[36];

// Menu
typedef enum {
    SPEED1, SPEED2, SPEED3,
    PERSP1, PERSP2, SNAKE1, SNAKE2, SALIR
} opcionesMenu;

bool showSplashScreen = true;

/*
 Manzana
 ******************************/

// Posición
int appleX, appleY;
int specX, specY;

// Angulo de rotación
int appleAngle = 0;


/*
 Serpiente
 ******************************/

// Flag para el evento "crece"
int crece = 0;
int mScore = 100;

/** -- Fin de las variables -- **/


/********************************************************************************
********************************************************************************
**                                                                            **
** Snake 3D                                                                   **
**                                                                            **
** Juego clásico de Snake, pero ahora con vista en 3D.                        **
**                                                                            **
** Controles:                                                                 **
**   - Flechas: Movimiento                                                    **
**   - P: cambiar de perspectiva (2D/3D)                                      **
**   - M: mostrar/esconder minimapa (sólo aplica para perspectiva 3D)         **
**                                                                            **
** El tablero mide 64x48 unidades, la serpiente se mueve una unidad           **
** por cada *tick* del timer. Se usa esta posición para mostrar la serpiente  **
** tanto en 2D como en 3D.                                                    **
**                                                                            **
** Hay dos vistas del juego:                                                  **
**   1. 2D (clásica)                                                          **
**   2. 3D (con minimapa)                                                     **
**                                                                            **
** Las coordenadas (x, y) de la ventana están en el rango                     **
** de -1.0 a 1.0, respectivamente.                                            **
**                                                                            **
** 2D: Se divide el total de unidades horizontales entre el rango del ancho,  **
** y las verticales entre el rango del alto, para obtener las coordenadas.    **
**                                                                            **
** 3D: Se utilizan las mismas coordenadas de la serpiente,                    **
** pero se dibuja una serie de cubos que la componen.                         **
**                                                                            **
********************************************************************************
********************************************************************************/

void loadTexture(Image *image, int k) {
    glBindTexture(GL_TEXTURE_2D, texName[k]); // Tell OpenGL which texture to edit

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Map the image to the texture
    glTexImage2D(GL_TEXTURE_2D,               // Always GL_TEXTURE_2D
            0,                           // 0 for now
            GL_RGB,                      // Format OpenGL uses for image
            image->width, image->height, // Width and height
            0,                           // The border of the image
            GL_RGB,                      // GL_RGB, because pixels are stored in RGB format
            GL_UNSIGNED_BYTE,            // GL_UNSIGNED_BYTE, because pixels are stored
            //   as unsigned numbers
            image->pixels);              // The actual pixel data
}

// Reads a bitmap image from a file
Image *loadBMP(const char *filename);

void onMenu(int opcion) {
    switch (opcion) {

        // Casos Velocidad
        case SPEED1:
            speed = 1.0;
            break;
        case SPEED2:
            speed = 2.0;
            break;
        case SPEED3:
            speed = 3.0;
            break;

            // Casos Perspectiva
        case PERSP1:
            showMap = false;
            break;
        case PERSP2:
            showMap = true;
            break;

            // Casos Forma Serpiente
        case SNAKE1:
            snakeShape = 0;
            break;
        case SNAKE2:
            snakeShape = 1;
            break;

            // Salir
        case SALIR:
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void initMenu(void) {
    int menuVelocidad, menuPerspectiva, menuSerpiente, menuPrincipal;

    menuVelocidad = glutCreateMenu(onMenu);
    glutAddMenuEntry("Principiante", SPEED1);
    glutAddMenuEntry("Intermedio", SPEED2);
    glutAddMenuEntry("Avanzado", SPEED3);

    menuPerspectiva = glutCreateMenu(onMenu);
    glutAddMenuEntry("3D", PERSP1);
    //glutAddMenuEntry("2D", PERSP2);

    menuSerpiente = glutCreateMenu(onMenu);
    glutAddMenuEntry("Esférica", SNAKE1);
    glutAddMenuEntry("Cubica", SNAKE2);

    glutCreateMenu(onMenu);
    glutAddSubMenu("Nivel", menuVelocidad);
    glutAddSubMenu("Perspectiva (3D)", menuPerspectiva);
    glutAddSubMenu("Serpiente", menuSerpiente);

    glutAddMenuEntry("Salir", SALIR);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}


/*
 * public: init()
 *
 * Inicialización de:
 *  - Modos de openGL
 *  - Estado del juego
 */
static void init() {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_FLAT);

    player = new Snake(unitsPerRow / 2, // initial position in X
            unitsPerCol / 2, // initial position in y
            100);            // maximum length of tail

    // Random seed
    srand((unsigned int) time(NULL));

    // Genera la Manzana por primera vez
    appleX = rand() % (unitsPerRow - 1) + 3;
    specX = rand() % (unitsPerRow - 1) + 3;
    specY = rand() % (unitsPerCol - 1) + 3;
    appleY = rand() % (unitsPerCol - 1) + 3;

    // Crea menu
    initMenu();

    // Texturas
    glGenTextures(2, texName);
    Image *image;

    //image = loadBMP("/home/luifer99/ClionProjects/neversnake/texturas/snake.bmp");
    image = loadBMP("/home/luifer99/ClionProjects/neversnake/texturas/snake.bmp");
    loadTexture(image, 0);

//    image = loadBMP("/home/luifer99/ClionProjects/neversnake/texturas/apple.bmp");
//    loadTexture(image, 1);

    delete image;
}


/*
 *
 * public: xPos2d(double x)
 *
 * x - posición en x del tablero
 * Regresa la posición x para desplegarse en 2D
 *
 *  Ejemplos:
 *
 *    - cuando x = 64, regresa 1.0
 *    - cuando x = 0, regresa -1.0
 *    - cuando x = 32, regresa 0.0
 *
 */
double xPos2d(int x) {
    double wide = maxX - minX;
    double mappedX = x * (wide / unitsPerRow);

    return minX + mappedX;
}

/*
 *
 * public: yPos2d(double x)
 *
 * y - posición en y del tablero
 * Regresa la posición y para desplegarse en 2D
 *
 *  Ejemplos:
 *
 *    - cuando y = 48, regresa 1.0
 *    - cuando y = 0, regresa -1.0
 *    - cuando y = 24, regresa 0.0
 *
 */
double yPos2d(int y) {
    double tall = maxY - minY;
    double mappedY = y * (tall / unitsPerCol);

    return minY + mappedY;
}

// Dibuja un String
void drawString(void *font, const char *s, float x, float y) {
    unsigned int i;
    glRasterPos2f(x, y);

    for (i = 0; i < strlen(s); i++)
        glutBitmapCharacter(font, s[i]);
}

void draw3dString(void *font, const char *s, float x, float y, float z) {
    unsigned int i;

    glPushMatrix();

    glTranslatef(x, y, z);
    glScaled(0.0005, 0.0005, 0.0005);

    for (i = 0; i < strlen(s); i++)
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, s[i]);

    glPopMatrix();
}


void instrucciones() {
//se apaga para que se vea mas iluminado cuando pierde
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(0.0, 1.0, 0.0);
    glLineWidth(1);

    std::stringstream ss; // Helper para desplegar el marcador

    glPushMatrix();

    ss << "Snake 3D Instrucciones";
    drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 1.08);
    ss.str("");
    ss.clear();

    ss <<   "En el plano del juego se encuentran alimentos, paredes y obstáculos, estos últimos deben\n"
            "ser evitados, mediante la maniobra adecuada. Los obstáculos y paredes tiene que tener\n"
            "textura sólidas (no transparentes)\n"
            "Los tipos de alimentos son:\n"
            "\uF0B7 Esfera de color blanco dan 25 pts, tamaño mediano (Probabilidad 50%).\n"
            "\uF0B7 Esfera de color azul, tamaño pequeño, reducen en un factor la velocidad\n"
            "(Probabilidad 20%).\n"
            "\uF0B7 Esfera de color rojo, tamaño mediano, permite por 10 segundos atropellar todo\n"
            "tipo de obstáculos sin perder la vida (Probabilidad 15%) OBS.: solamente\n"
            "obstáculos se puede atropellar, las paredes no.\n"
            "\uF0B7 Esferas de color amarillo dan 10 pts, tamaño pequeño (Probabilidad 60%)\n"
            "\uF0B7 Esferas de color verde dan 100 pts, tamaño grande, aparición (Probabilidad 10%)";
    drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 1.08);
    ss.str("");
    ss.clear();

    aux3=9;
}

void drawSplashScreen() {
//se apaga para que se vea mas iluminado cuando pierde
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(0.0, 1.0, 0.0);
    glLineWidth(1);

    std::stringstream ss; // Helper para desplegar el marcador

    glPushMatrix();

    ss << "Snake 3D version 1.00";
    drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 1.08);
    ss.str("");
    ss.clear();

    ss << "Copyright (C) 2014 Team Pro";
    drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 1.0);
    ss.str("");
    ss.clear();

    ss << "Opciones";
    drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.60);
    ss.str("");
    ss.clear();

    if(aux2 == 0){
        ss << "-> Jugar";
        drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.50);
        ss.str("");
        ss.clear();
        ss << "   Ranking";
        drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.40);
        ss.str("");
        ss.clear();
        ss << "   Instrucciones";
        drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.30);
        ss.str("");
        ss.clear();
    }


    if(aux2 == 1){
        ss << "   Jugar";
        drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.50);
        ss.str("");
        ss.clear();
        ss << "-> Ranking";
        drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.40);
        ss.str("");
        ss.clear();
        ss << "   Instrucciones";
        drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.30);
        ss.str("");
        ss.clear();
    }


    if(aux2 == 2){
        ss << "   Jugar";
        drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.50);
        ss.str("");
        ss.clear();
        ss << "   Ranking";
        drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.40);
        ss.str("");
        ss.clear();
        ss << "-> Instrucciones";
        drawString(GLUT_BITMAP_8_BY_13, ss.str().c_str(), -1.5, 0.30);
        ss.str("");
        ss.clear();
    }
    glPopMatrix();

}

void drawApple() {
    if (appleFlag == 1) {
        glColor3f(red, green, blue);
        glPushMatrix();
        glTranslated(xPos2d(specX), yPos2d(specY), 0.025);
        glutSolidSphere(radio, 10, 10);
        glPopMatrix();
    }
    glColor3f(0.5, 0.2, 0.7);
    glPushMatrix();
    glTranslated(xPos2d(appleX), yPos2d(appleY), 0.025);
    glRotated(appleAngle, 0.3, 1.0, 0.0);
    glutSolidSphere(0.04, 10, 10);
    glPopMatrix();
}

static void crearobstaculos (void) {

    if (aux1 == 0){
        aux1 = 1;

        for(j=0;j<i +1;j++) {
            n[j].f = 0;
            n[j].c = 0;
            n[j].s = 0;
            n[j].l = 0;
        }

        i = 3*nivel;
        printf("(obs: %d)",i);
        for(j=0;j<i +1;j++) {
            n[j].f = rand() % unitsPerRow + 1;
            n[j].c = rand() % unitsPerCol + 1;
            n[j].s = rand() % 2;
            n[j].l = rand() % 5 + 1;
            printf("(%d,%d,%d,%d)\n", n[j].f, n[j].c, n[j].s, n[j].l);
        }
    }

}

static void drawPerspective(void) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLfloat ambientLight[] = {0.5f, 0.4f, 0.4f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

    GLfloat directedLight[] = {0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat directedLightPos[] = {-10.0f, 15.0f, 20.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, directedLight);
    glLightfv(GL_LIGHT0, GL_POSITION, directedLightPos);

    // Dibuja la Cuadrícula
    glColor3f(0.2, 0.0, 0.2);
    glLineWidth(1);

    glBegin(GL_LINES);

    for (double i = 0; i <= unitsPerRow; i += 2) {
        glVertex2d(minX, xPos2d(i));
        glVertex2f(maxX, xPos2d(i));
        glVertex2d(xPos2d(i), minY);
        glVertex2f(xPos2d(i), maxY);
    }

    glEnd();

    // Dibuja el Marco
    glColor3f(1, 1, 1);
    //derecha

    glPushMatrix();
    glTranslated(maxX + 0.05, 0.0, 0.0);
    glScaled(1.0, 1.0 * 41, 1.0);
    glutSolidCube(0.05);
    glPopMatrix();


    //obstaculos aleatorios una funcion aparte? o solo el rand

    crearobstaculos();

//tengo que poner un if para ver si sale del campo los obstaculos cuando son muy largos
    for(j=0;j<i+1 ;j++) { //posiciones de los obstaculos en el vector de atributos
        printf("(%d,%d,%d,%d)p%d\n", n[j].f, n[j].c, n[j].s, n[j].l,j);
        if(n[j].l < 7 && (n[j].s == 0 || n[j].s == 1)){
        for(k=0;k <= n[j].l ;k++) {//largo del obstaculo
            if( n[j].s == 1 ){ //crece en y
                if( (n[j].c +k) <= unitsPerCol){
                    glPushMatrix();
                    glTranslated(xPos2d(n[j].f), yPos2d(n[j].c +k), 0.0); //translada con parametros -1 a 1
                    glScaled(1.0, 1.0, 1.0); //x 1 a 42 e y de 1 a 42 escala que tan largo sera en x o y
                    glutSolidCube(0.05);
                    glPopMatrix();
                    printf("(%d,%d)cd,",n[j].f,n[j].c +k );
                }
            }
            if( n[j].s == 0 ){
                if( (n[j].f +k) <= unitsPerRow){
                    glPushMatrix();
                    glTranslated(xPos2d(n[j].f +k), yPos2d(n[j].c), 0.0); //translada con parametros -1 a 1
                    glScaled(1.0, 1.0, 1.0); //x 1 a 42 e y de 1 a 42 escala que tan largo sera en x o y
                    glutSolidCube(0.05);
                    glPopMatrix();
                    printf("(%d,%d)fd,",n[j].f +k ,n[j].c);
                }
            }
        }
        printf("\n...........\n");
        }
    }

    // izquierda
    glPushMatrix();
    glTranslated(minX - 0.05, 0.0, 0.0);
    glScaled(1.0, 1.0 * 41, 1.0);
    glutSolidCube(0.05);
    glPopMatrix();

    // adelante
    glPushMatrix();
    glTranslated(0.0, minY - 0.05, 0.0);
    glScaled(1.0 * 43, 1.0, 1.0);
    glutSolidCube(0.05);
    glPopMatrix();
    // atras
    glPushMatrix();
    glTranslated(0.0, maxY + 0.05, 0.0);
    glScaled(1.0 * 43, 1.0, 1.0);
    glutSolidCube(0.05);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);

    // Dibuja la Manzana
    glBindTexture(GL_TEXTURE_2D, texName[1]);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    drawApple();

    // Dibuja la Serpientedr
    glColor3f(1.0, 1.0, 1.0);

    glBindTexture(GL_TEXTURE_2D, texName[0]);

    for (int i = player->length - 1; i >= 0; i--) {
        glPushMatrix();
        glTranslated(xPos2d(player->xAt(i)), yPos2d(player->yAt(i)), 0.025);
        glutSolidSphere(0.03, 10, 10);

        glPopMatrix();
    }

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_2D);

    // Dibuja el Marcador
    glColor3f(1.0, 1.0, 1.0);

    std::stringstream ss; // Helper para desplegar el marcador
    std::stringstream sp;
    sp << "Speed: " << std::to_string(speed);
    draw3dString(GLUT_STROKE_MONO_ROMAN, sp.str().c_str(), -0.93, -0.85, 0.0);
    ss << "Score: " << std::to_string(score);
    draw3dString(GLUT_STROKE_MONO_ROMAN, ss.str().c_str(), 0.5, -0.85, 0.0);
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat) w / (GLfloat) h, 1.0, 20.0);
}

static void display(void) {
    double snakeX, snakeY;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (showSplashScreen) {

        gluLookAt(0.0, 0.0, 2.0,
                0.0, 0.0, 0.0,
                0.0, 1.0, 0.0);

        drawSplashScreen();

    } else {

        snakeX = xPos2d(player->x());
        snakeY = yPos2d(player->y());

        snakeY = snakeY <= -0.5 ? -0.5 : snakeY;
        snakeY = snakeY >= 1.5 ? 1.5 : snakeY;

        gluLookAt(0.0, -2.0, 1,
                0.0, snakeY, 0.0,
                0.0, 0.5, 0.0);

        drawPerspective();

    }
    if (aux3 == 2){
        instrucciones();

    }


    glutSwapBuffers();

}

void generateApple(int appleValue) {
    switch (appleValue) {
        case 1: //blanco
            red = green = blue = 1;
            radio = 0.04;
            break;
        case 2: //azul
            red = green = 0;
            blue = 1;
            radio = 0.02;
            break;
        case 3: //rojo
            red = 1;
            green = blue = 0;
            radio = 0.04;
            break;
        case 4: //amarillo
            red = green = 1;
            blue = 0;
            radio = 0.02;
            break;
        case 5: //verde
            red = blue = 0;
            green = 1;
            radio = 0.06;
            break;
        default: //no genera nada
            appleFlag = 0;
            break;
    }

}

int specialAppleValue() {
    //aca debería calcular la posibilidad de que salga cada manzana
    int probApple = (rand() % 300) + 1;
    if (probApple <= 50) {
        return 1; //blanco
    } else if (probApple <= 70) {
        return 2; //azul
    } else if (probApple <= 85) {
        return 3; //rojo
    } else if (probApple <= 145) {
        return 4; //amarillo
    } else if (probApple <= 155) {
        return 5; //verde
    } else {
        return 0; //nada
    }
}

void snakessj(int val) {
    Image *image;
    if(val){ //1 -> SI
        image = loadBMP("/home/hector/ClionProjects/neversnake/texturas/apple.bmp");
    }else{
        image = loadBMP("/home/hector/ClionProjects/neversnake/texturas/snake.bmp");
    }
    loadTexture(image, 0);
    delete image;
}

// Regresa verdadero si la serpiente colisiona con un par de puntos
bool snakeHits(float x, float y) {
    double nextX, nextY;

    nextX = player->x();
    nextY = player->y();

    return nextX == x && nextY == y;
}

void resetGame() {
    if (dirX == -1 && dirY == 0)
        dirX = 1;
    player->reset();
    start = time(0);
    speed = 1;
    appleFlag = 0;
    appleX = rand() % (unitsPerRow - 1) + 2;
    appleY = rand() % (unitsPerCol - 1) + 2;
    score = 0;
    aux1 = 0;
    snakessj(0);
    crearobstaculos();
}

void myTimer(int valor) {

    double secondsSinceStart = difftime(time(0), start);

    if(difftime(time(0), traspasaTimeIni) > 10){
        traspasarParedes = 1;
        snakessj(0);
    }

    if (fmod(secondsSinceStart, 100) == 0) {
        if (isTime) {
            speed = speed*1.1;
            isTime = 0;
        }
    } else {
        isTime = 1;
    }

    if (showSplashScreen) {
        glutTimerFunc((unsigned int) (timerTick / speed), myTimer, 1);
        return;
    }

    //compara la posicion de los obstaculos para ver si choca
    //hay que mirar bien porque suele chocar un punto x o y despues -- arreglado :D
    if(traspasarParedes)
    for (z = 0; z < i + 1; z++) {
        for (k1 = 0; k1 < n[z].l + 1; k1++) {//largo del obstaculo
            if (n[z].s == 1) { //crece en y
                if (player->x() == n[z].f && player->y() == (n[z].c + k1)) { //choca con un obstaculo
                    printf("(%d,%d)c\n", player->x(), player->y());
                    showSplashScreen = true;
                    resetGame();
                }
            }
            if (n[z].s == 0) {
                if (player->x() == (n[z].f + k1) && player->y() == n[z].c) { //choca con un obstaculo
                    printf("(%d,%d)f\n", player->x(), player->y());
                    showSplashScreen = true;
                    resetGame();
                }
            }

        }
    }

    // Revisa si la Serpiente colisiona con el marco
    // y cambia la dirección cuando sea necesario
    if ((dirX == 1 && player->x() >= unitsPerRow) || (dirX == -1 && player->x() <= 0) ||
            (dirY == 1 && player->y() >= unitsPerCol) ||
            (dirY == -1 && player->y() <= 0)) {
        showSplashScreen = true;
        resetGame();
    }
    if (dirX == 1 && player->x() >= unitsPerRow) {
        resetGame();
    } else if (dirX == -1 && player->x() <= 0) {
        resetGame();
    } else if (dirY == 1 && player->y() >= unitsPerCol) {
        resetGame();
    } else if (dirY == -1 && player->y() <= 0) {
        resetGame();
    }

    appleAngle = (appleAngle >= 360) ? 0 : appleAngle + 5;

    // Crece la cola primero para que el jugador tenga mejor control
    if (crece == 1 || snakeHits(appleX, appleY) || snakeHits(specX, specY)) {
        // Incrementa el score si choca con una manzana normal
        if (snakeHits(appleX, appleY)) {
            appleX = rand() % (unitsPerRow - 1) + 1;
            appleY = rand() % (unitsPerCol - 1) + 1;
            score += 1;
        }

        /*
        * 1 Manzana blanca: 25pts -> mediano
        * 2 Manzana azul: reduce velocidad -> pequeño
        * 3 Manzana rojo: por 10seg atropella obstaculos -> mediano
        * 4 Manzana amarilla: 10pts -> pequeño
        * 5 Manzana verde: 100pts -> grande
         */

        // Si choca contra una especial
        if (snakeHits(specX, specY)) {
            appleFlag = 0;
            specX = rand() % (unitsPerRow - 1) + 3;
            specY = rand() % (unitsPerCol - 1) + 3;
            scoreMultiplier = 0;
            if (specialApple == 1) {
                scoreMultiplier = 25;
            } else if (specialApple == 2) {
                speed *= 0.5; //reduce la mitad
            } else if (specialApple == 3) {
                //FLAG DE OBSTACULOS
                snakessj(1);
                traspasarParedes = 0;
                traspasaTimeIni = time(0);
            } else if (specialApple == 4) {
                scoreMultiplier = 10;
            } else if (specialApple == 5) {
                scoreMultiplier = 100;
            }
            score += scoreMultiplier;
        }

        if (appleFlag == 0) {
            appleFlag = 1;
            specialApple = specialAppleValue();
        }

        if (!player->full()) {
            player->eat();
        }

        crece = 0;
        generateApple(specialApple);
    }

    if (!player->moveTo(dirX, dirY)) {
        resetGame();
        score = 0;
        scoreMultiplier = 1;
    }

    if(mScore > score%200){
        aux1 = 0;
        nivel++;
    }
    mScore = score%200;

    glutPostRedisplay();
    glutTimerFunc(timerTick / speed, myTimer, 1);
}

void myKeyboard(int key, int x, int y) {
    // Cambia el valor de dirX y dirY dependiendo de la tecla que oprima el usuario.
    // Activa la bandera de crecer para que la funcion `myTimer` crezca la serpiente en una unidad.
    // Debe funcionar para mayuscula y minuscula.
    switch (key) {
        // Mueve la serpiente
        case GLUT_KEY_LEFT:
            if (dirX == 1 && dirY == 0) {
                //va a la derecha
                dirX = 0;
                dirY = 1;
            } else if (dirX == 0 && dirY == 1) {
                //si se va arriba
                dirX = -1;
                dirY = 0;
            } else if (dirX == -1 && dirY == 0) {
                dirX = 0;
                dirY = -1;
            } else if (dirX == 0 && dirY == -1) {
                dirX = 1;
                dirY = 0;
            }
            break;
        case GLUT_KEY_RIGHT:
            if (dirX == 1 && dirY == 0) {
                //va a la derecha
                dirX = 0;
                dirY = -1;
            } else if (dirX == 0 && dirY == 1) {
                //si se va arriba
                dirX = 1;
                dirY = 0;
            } else if (dirX == -1 && dirY == 0) {
                dirX = 0;
                dirY = 1;
            } else if (dirX == 0 && dirY == -1) {
                dirX = -1;
                dirY = 0;
            }
            break;
        // Salir
        case 27:
            exit(-1);
    }

    glutPostRedisplay();

}

void myKey(unsigned char key, int x, int y) {
    // Cambia el valor de dirX y dirY dependiendo de la tecla que oprima el usuario.
    // Activa la bandera de crecer para que la funcion `myTimer` crezca la serpiente en una unidad.
    // Debe funcionar para mayuscula y minuscula.
    switch (key) {
        // Mueve la serpiente
        case 'a':
            if (dirX == 1 && dirY == 0) {
                //va a la derecha
                dirX = 0;
                dirY = 1;
            } else if (dirX == 0 && dirY == 1) {
                //si se va arriba
                dirX = -1;
                dirY = 0;
            } else if (dirX == -1 && dirY == 0) {
                dirX = 0;
                dirY = -1;
            } else if (dirX == 0 && dirY == -1) {
                dirX = 1;
                dirY = 0;
            }
            break;
        case 'd':
            if (dirX == 1 && dirY == 0) {
                //va a la derecha
                dirX = 0;
                dirY = -1;
            } else if (dirX == 0 && dirY == 1) {
                //si se va arriba
                dirX = 1;
                dirY = 0;
            } else if (dirX == -1 && dirY == 0) {
                dirX = 0;
                dirY = 1;
            } else if (dirX == 0 && dirY == -1) {
                dirX = -1;
                dirY = 0;
            }
            break;
        case 13:
            if(aux2 == 0) {
                showSplashScreen = false; //press enter
            }
            if(aux2 == 2){
                aux3 = 2;
            }
            if(aux3 == 9){
                display();
            }

            break;

            // Salir
        case 27:
            exit(-1);
        case 'n'://hay que cambiar por las flechas
            printf("asd1\n");
            if (aux2 > 0){
                aux2 = aux2 -1;
                //display();
                break;
            }else {
                aux2 = 2;
                //display();
                break;
            }
        case 'm'://hay que cambiar por las flechas
            printf("asd\n");
            if (aux2 < 2){
                aux2 = aux2 + 1;
                display();//donde se crea la pantalla inicial
                break;
            }else{
                aux2 = 0;
                display();
                break;
            }
    }
    glutPostRedisplay();

}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitWindowSize(width, height);
    glutInitWindowPosition(120, 120);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Snake 3D - Amarilla/Villalba");

    init();

   // glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(myKey);
    glutSpecialFunc(myKeyboard);
    glutTimerFunc(2000, myTimer, 1);

    glutMainLoop();
    return EXIT_SUCCESS;
}

/*

Niniejszy program jest wolnym oprogramowaniem; mo¿esz go
rozprowadzaæ dalej i / lub modyfikowaæ na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundacjê Wolnego
Oprogramowania - wed³ug wersji 2 tej Licencji lub(wed³ug twojego
wyboru) którejœ z póŸniejszych wersji.

Niniejszy program rozpowszechniany jest z nadziej¹, i¿ bêdzie on
u¿yteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyœlnej
gwarancji PRZYDATNOŒCI HANDLOWEJ albo PRZYDATNOŒCI DO OKREŒLONYCH
ZASTOSOWAÑ.W celu uzyskania bli¿szych informacji siêgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnoœci¹ wraz z niniejszym programem otrzyma³eœ te¿ egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeœli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <lodepng.h>
#include <string>
#include <time.h>
#include "constants.h"
#include "struct.h"
#include "allmodels.h"

#include "move.h"
#include "loaderOBJ.h"
#include <string>

#define szerokoscOkna 500
#define wysokoscOkna 500

// tekstury dla obiektow
#define twall "tekstury/wood.png"
#define tfloor "tekstury/floor.png"
#define tplayer "tekstury/pacman.png"
#define tcoin "tekstury/coin.png"
#define tworld "tekstury/world.png"
#define tlicznik "tekstury/licznik.png"
#define tghost1 "tekstury/ghost1.png"
#define tghost2 "tekstury/ghost2.png"
#define tghost3 "tekstury/ghost3.png"
#define tghost4 "tekstury/ghost4.png"
#define tghostX "tekstury/ghostX.png"
#define tstart "tekstury/start.png"
#define tend "tekstury/end.png"
#define twin "tekstury/win.png"

// definicja klawiszy (latwa zmiana sterowania w kodzie)

#define kup  GLFW_KEY_W
#define kdown  GLFW_KEY_S
#define kleft  GLFW_KEY_A
#define kright  GLFW_KEY_D
#define klook_back  GLFW_KEY_J

#define K_CAMERA_PACMAN GLFW_KEY_1
#define K_CAMERA_UP GLFW_KEY_2

#define KEY_SECOND true
#define K_UP_SECOND GLFW_KEY_UP
#define K_DOWN_SECOND GLFW_KEY_DOWN
#define K_RIGHT_SECOND GLFW_KEY_RIGHT
#define K_LEFT_SECOND GLFW_KEY_LEFT

// asocjacja klawiszy z indeksami dla tablicy klawiszy...
#define max_key 32
#define nop 0
#define up  1
#define down  2
#define left  3
#define right  4
#define look_back 5

// ustawienia gry
#define big_coin_number 2

bool CAMERA_PACMAN = true;

using namespace glm;

// tablica kolizji (aby nie trzymiac wymiarow w poszczegolnych obiektach tylko ogolnie dla danego typu obiektu
colision_length colision_table[MAX_MODEL_ON_MAP];
std::vector <glm::vec3> coin_position;
// potrzbne modele
struct colision_length unvalue;
Wall *world;
Floor *podloga;
Map *mapa;
Player *player;
Coin *coin;
Licznik *licznik;
Ghost *ghost_1;
Ghost *ghost_2;
Ghost *ghost_3;
Ghost *ghost_4;
Wall *ginfo = new Wall(unvalue);

bool fullscreen = false;

float aspect=1.0f; //Aktualny stosunek szerokoœci do wysokoœci okna
float speed_y=0; //Szybkoœæ k¹towa obrotu obiektu w radianach na sekundê wokó³ osi y
float camera_far = 1.235; // odleglosc kamery (promiec okregu po ktorym porusza sie kamera wokol PacMana)
float camera_angle = 0.4; // odleglosc camery nad PacManem w osi Y

bool ghost_run = false;
bool game_start = false;
bool game_end = false;
bool game_win = false;

int modulo_coin = 10;
// tablica dla kawiszy (aby pamietac jakie byly i bezproblemowo robic kombinajce klawiszy)
bool keys[max_key];

// definicja uchwytow na tekstury
GLuint texSciana;
GLuint texPodloga;
GLuint texPlayer;
GLuint texCoin;
GLuint texWorld;
GLuint texLicznik;
GLuint texGhost1;
GLuint texGhost2;
GLuint texGhost3;
GLuint texGhost4;
GLuint texGhostX;
GLuint texStart;
GLuint texEnd;
GLuint texWin;

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

//Procedura obługi zmiany rozmiaru bufora ramki
void windowResize(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height); //Obraz ma być generowany w oknie o tej rozdzielczości
    ///(u nas ma byc staly!)
    aspect=(float)szerokoscOkna/(float)wysokoscOkna; //Stosunek szerokości do wysokości okna
}

void configGame() {
    coin_position.clear();
    for(int i=0; i<max_key; i++) keys[i]=false;
    game_end = false;
    game_start = true;
    game_win = false;
    ghost_run = false;
    world = new Wall(unvalue);
    podloga = new Floor(colision_table[mFLOR]);
    mapa = new Map(colision_table[mWALL],coin_position);
    player = new Player(mapa, colision_table[mPMAN]);
    coin = new Coin(colision_table[mCOIN]);
    licznik = new Licznik(unvalue);
    ghost_1 = new Ghost(mapa,colision_table[mGHO1],mGHO1);
    ghost_2 = new Ghost(mapa,colision_table[mGHO2],mGHO2);
    ghost_3 = new Ghost(mapa,colision_table[mGHO3],mGHO3);
    ghost_4 = new Ghost(mapa,colision_table[mGHO4],mGHO4);
    world->position = vec3(SZEROKOSC_MAPY/2,0.0,WYSOKOSC_MAPY/2);
    world->scale = vec3(40.0,40.0,40.0);
    ghost_1->speed = 0.5;
    ghost_2->speed = 1.0;
    ghost_3->speed = 1.5;
    ghost_4->speed = 2.0;
    modulo_coin = coin_position.size() / (big_coin_number+1);
}

// funkcja ktora powoduje ruch w kazdym kierunku (po skosie dwa klawisze tez) ogolnie od klawiszy jest
void doMove(Map* &mapa,colision_length colision_table[],std::vector <glm::vec3> &coin_position) {
    if(keys[up]) goSTRAIGHT(player,mapa,colision_table,coin_position);  // do przodu
    else if(keys[down]) goBACK(player, mapa, colision_table,coin_position);  // do tylu
    else if(keys[right]) rotateRIGHT(player);  // obrot w prawo
    else if(keys[left]) rotateLEFT(player);  // obrot w lewo
    if(keys[left] && keys[up]) {  // po skosie gora/lewo
        rotateLEFT(player);
        goSTRAIGHT(player,mapa,colision_table,coin_position);
    } else if(keys[right] && keys[up]) { // po skosie gora/prawo
        rotateRIGHT(player);
        goSTRAIGHT(player,mapa,colision_table,coin_position);
    } else if(keys[right] && keys[down]) { // po skosie dol/prawo
        rotateLEFT(player);
        goBACK(player, mapa, colision_table,coin_position);
    } else if(keys[left] && keys[down]) { // po skosie dol/lewo
        rotateRIGHT(player);
        goBACK(player, mapa, colision_table,coin_position);
    }
}

//Procedura obsługi klawiatury
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action == GLFW_PRESS && key == GLFW_KEY_F) {
        const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        int window_width = mode->width;
        int window_height = mode->height;
        int refresh = mode->refreshRate;
        if(fullscreen) {
            glfwSetWindowMonitor(window,NULL,50,50,szerokoscOkna,wysokoscOkna,GLFW_DONT_CARE);
            glfwSetWindowSize(window, szerokoscOkna, wysokoscOkna);
            fullscreen = false;
        } else {
            int black = (window_width-window_height)/2;
            glfwSetWindowSize(window, window_height, window_height);
            glfwSetWindowMonitor(window,glfwGetPrimaryMonitor(),black,0,window_height,window_height,refresh);
            fullscreen = true;
        }
    }
    if(game_start==false) {
        if(key == GLFW_KEY_ENTER) game_start=true;
    } else {
        if(game_end==true) {
            if(key == GLFW_KEY_ENTER) glfwSetWindowShouldClose(window, 1);
            if(key == GLFW_KEY_SPACE) configGame();
        } else {
            if(action == GLFW_PRESS) {
                if(key == kleft) keys[left] = true;
                if(key == kright) keys[right] = true;
                if(key == kup) keys[up] = true;
                if(key == kdown) keys[down] = true;
                if(key == klook_back) keys[look_back] = true;
                if(key == K_CAMERA_PACMAN) CAMERA_PACMAN = true;
                if(key == K_CAMERA_UP) CAMERA_PACMAN = false;
                if(KEY_SECOND) {
                    if(key == K_LEFT_SECOND) keys[left] = true;
                    if(key == K_RIGHT_SECOND) keys[right] = true;
                    if(key == K_UP_SECOND) keys[up] = true;
                    if(key == K_DOWN_SECOND) keys[down] = true;
                }
            }
            if(action == GLFW_RELEASE) {
                if(key == kleft) keys[left] = false;
                if(key == kright) keys[right] = false;
                if(key == kup) keys[up] = false;
                if(key == kdown) keys[down] = false;
                if(key == klook_back) keys[look_back] = false;
                if(KEY_SECOND) {
                    if(key == K_LEFT_SECOND) keys[left] = false;
                    if(key == K_RIGHT_SECOND) keys[right] = false;
                    if(key == K_UP_SECOND) keys[up] = false;
                    if(key == K_DOWN_SECOND) keys[down] = false;
                }
            }
        }
    }
}
// inicjalizacja swiatla na mapie
void LetItBeLight() {
    GLfloat ambientLight[] = { 0.1f, 0.1f, 0.1f, 1.0f }; // otoczenia
    GLfloat diffuseLight[] = { 0.4f, 0.4f, 0.4, 1.0f }; // rozproszenia
    GLfloat specularLight[] = { 0.7f, 0.7f, 0.7f, 1.0f }; // odbicia
    GLfloat position0[] = { 2.0f, 2.0f, -2.0f, 1.0f }; // pozycja za pacmanem po prawej
    GLfloat position1[] = { -2.0, 2.0f, -2.0f, 1.0f }; // pozycja za pacmanem po lewej
    //glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position0);
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT1, GL_POSITION, position1);
}
void wczytajObraz(GLuint &tex, std::string path) {
    std::vector<unsigned char> image; //Alokuj wektor do wczytania obrazka
    unsigned width, height; //Zmienne do których wczytamy wymiary obrazka
    unsigned error = lodepng::decode(image, width, height, path);
    if(error != 0) {
        fprintf(stderr,"%s\n",lodepng_error_text(error)); // wypisanie bledu jak cos nie pojdzie
        exit(1);
    }
    //Import do pamiêci karty graficznej
    glGenTextures(1,&tex); //Zainicjuj jeden uchwyt
    glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
    //Wczytaj obrazek do pamiêci KG skojarzonej z uchwytem
    glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*) image.data());
    //Bilinear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);
}
//Procedura inicjuj¹ca
void initOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który nale¿y wykonaæ raz, na pocz¹tku programu************
    glfwSetFramebufferSizeCallback(window, windowResize); //Zarejestruj procedurê obs³ugi zmiany rozdzielczoœci bufora ramki
    glfwSetKeyCallback(window, key_callback); //Zarejestruj procedurê obs³ugi klawiatury
    LetItBeLight(); // swiatlo
    glEnable(GL_COLOR_MATERIAL); // wlaczenie kolorow w opengl
    glEnable(GL_DEPTH_TEST); //W³¹cz u¿ywanie budora g³êbokoœci
    /// -> Wczytaj obrazek - world
    wczytajObraz(texWorld, tworld);
    /// - > Wczytaj obrazek - sciana
    wczytajObraz(texSciana,twall);
    /// - > Wczytaj obrazek - podloga
    wczytajObraz(texPodloga,tfloor);
    /// - > Wczytaj obrazek - player
    wczytajObraz(texPlayer,tplayer);
    /// -> Wczytaj obrazek - coin
    wczytajObraz(texCoin, tcoin);
    /// -> Wczytaj obrazek - licznik
    wczytajObraz(texLicznik, tlicznik);
    /// -> Wczytaj obrazek - ghost1
    wczytajObraz(texGhost1, tghost1);
    /// -> Wczytaj obrazek - ghost2
    wczytajObraz(texGhost2, tghost2);
    /// -> Wczytaj obrazek - ghost3
    wczytajObraz(texGhost3, tghost3);
    /// -> Wczytaj obrazek - ghost4
    wczytajObraz(texGhost4, tghost4);
    /// -> Wczytaj obrazek - ghostX
    wczytajObraz(texGhostX, tghostX);
    /// -> Wczytaj obrazek - start
    wczytajObraz(texStart, tstart);
    /// -> Wczytaj obrazek - end
    wczytajObraz(texEnd, tend);
    /// -> Wczytaj obrazek - win
    wczytajObraz(texWin, twin);
}
void drawInfo(GLFWwindow* window,GLuint tex) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //Wyczyœæ bufor kolorów (czyli przygotuj "p³ótno" do rysowania)
    mat4 P=perspective(35.0f*PI/180.0f,aspect,1.0f,50.0f); //Wylicz macierz rzutowania P
    mat4 V; // macierz widoku
    V = lookAt(vec3(0.0,0.0,-2.3),
               vec3(0.0,0.0,0.0),
               vec3(0.0,1.0,0.0));
    glMatrixMode(GL_PROJECTION); //W³¹cz tryb modyfikacji macierzy rzutowania
    glLoadMatrixf(value_ptr(P)); //Za³aduj macierz rzutowania
    glMatrixMode(GL_MODELVIEW);  //W³¹cz tryb modyfikacji macierzy model-widok
    /// Rysowanie
    ginfo->position = vec3(0.0f,0.0f,0.0f);
    ginfo->rotation = vec3(0.0f,0.0f,PI/2);
    ginfo->drawSolid(tex,V);
    glfwSwapBuffers(window);
}
void drawEnd(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //Wyczyœæ bufor kolorów (czyli przygotuj "p³ótno" do rysowania)
    mat4 P=perspective(35.0f*PI/180.0f,aspect,1.0f,50.0f); //Wylicz macierz rzutowania P
    mat4 V; // macierz widoku
    V = lookAt(vec3(0.0,0.0,-2.3),
               vec3(0.0,0.0,0.0),
               vec3(0.0,1.0,0.0));
    glMatrixMode(GL_PROJECTION); //W³¹cz tryb modyfikacji macierzy rzutowania
    glLoadMatrixf(value_ptr(P)); //Za³aduj macierz rzutowania
    glMatrixMode(GL_MODELVIEW);  //W³¹cz tryb modyfikacji macierzy model-widok
    /// Rysowanie
    ginfo->position = vec3(0.0f,0.0f,0.0f);
    ginfo->rotation = vec3(0.0f,0.0f,PI/2);
    ginfo->drawSolid(texStart,V);
    glfwSwapBuffers(window);
}
void drawGame(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //Wyczyœæ bufor kolorów (czyli przygotuj "p³ótno" do rysowania)
    vec3 licznik_1_pos;
    vec3 licznik_2_pos;
    vec3 camera_position;
    vec3 camera_look_at;
    vec3 camera_up = vec3(0.0f,1.0f,0.0);
    if(CAMERA_PACMAN && !keys[look_back]) { /// widok do przodu
        camera_position = vec3(player->position.x-camera_far*cos(player->rotation.y),
                               player->position.y+camera_angle,
                               player->position.z+camera_far*sin(player->rotation.y));
        camera_look_at = vec3(player->position.x+camera_far*cos(player->rotation.y),
                              player->position.y+0.3,
                              player->position.z-camera_far*sin(player->rotation.y));
        licznik_1_pos = vec3(player->position.x+(camera_far/6)*cos(player->rotation.y),
                             player->position.y+camera_angle*1.6,
                             player->position.z-(camera_far/6)*sin(player->rotation.y));
        licznik_2_pos = vec3(player->position.x+(camera_far/6)*cos(player->rotation.y+camera_far/5),
                             player->position.y+camera_angle*1.6,
                             player->position.z-(camera_far/6)*sin(player->rotation.y+camera_far/5));
        licznik->rotation.y=player->rotation.y-90.0*PI/180.0;
        licznik->zgory = false;
    } else if(!CAMERA_PACMAN) { /// widok z góry
        camera_position = vec3(player->position.x - camera_far*cos(player->rotation.y),
                               player->position.y + 10,
                               player->position.z + camera_far*sin(player->rotation.y));
        camera_look_at = vec3(player->position.x,
                              player->position.y,
                              player->position.z);
        licznik_2_pos = vec3(player->position.x-(camera_far/2.5)*cos(player->rotation.y),
                             player->position.y+8,
                             player->position.z+(camera_far/2.5)*sin(player->rotation.y));
        licznik_1_pos = vec3(player->position.x-(camera_far/2.5)*cos(player->rotation.y+camera_far/2.5/5),
                             player->position.y+8,
                             player->position.z+(camera_far/2.5)*sin(player->rotation.y+camera_far/2.5/5));
        licznik->rotation.y=player->rotation.y-90.0*PI/180.0;
        licznik->zgory = true;
    } else { /// widok do tyłu
        camera_position = vec3(player->position.x+camera_far*cos(player->rotation.y),
                               player->position.y+camera_angle,
                               player->position.z-camera_far*sin(player->rotation.y));
        camera_look_at = vec3(player->position.x-camera_far*cos(player->rotation.y),
                              player->position.y+0.3,
                              player->position.z+camera_far*sin(player->rotation.y));
        licznik_1_pos = vec3(player->position.x-(camera_far/6)*cos(player->rotation.y),
                             player->position.y+camera_angle*1.6,
                             player->position.z+(camera_far/6)*sin(player->rotation.y));
        licznik_2_pos = vec3(player->position.x-(camera_far/6)*cos(player->rotation.y+camera_far/5),
                             player->position.y+camera_angle*1.6,
                             player->position.z+(camera_far/6)*sin(player->rotation.y+camera_far/5));
        licznik->rotation.y=player->rotation.y+90.0*PI/180.0;
        licznik->zgory = false;
    }
    mat4 P=perspective(35.0f*PI/180.0f,aspect,1.0f,50.0f); //Wylicz macierz rzutowania P
    mat4 V; // macierz widoku
    V = lookAt(camera_position,camera_look_at,camera_up);
    glMatrixMode(GL_PROJECTION); //W³¹cz tryb modyfikacji macierzy rzutowania
    glLoadMatrixf(value_ptr(P)); //Za³aduj macierz rzutowania
    glMatrixMode(GL_MODELVIEW);  //W³¹cz tryb modyfikacji macierzy model-widok
    /// Rysowanie
    world->drawSolid(texWorld,V);
    podloga->drawSolid(texPodloga,V);
    mapa->drawSolid(texSciana,V);
    player->drawSolid(texPlayer,V);
    coin->drawAll(texCoin,V,coin_position);
    if(ghost_run) {
        ghost_1->drawSolid(texGhostX,V);
        ghost_2->drawSolid(texGhostX,V);
        ghost_3->drawSolid(texGhostX,V);
        ghost_4->drawSolid(texGhostX,V);
    } else {
        ghost_1->drawSolid(texGhost1,V);
        ghost_2->drawSolid(texGhost2,V);
        ghost_3->drawSolid(texGhost3,V);
        ghost_4->drawSolid(texGhost4,V);
    }
    licznik->drawAll(texLicznik,V,licznik_1_pos,licznik_2_pos,coin_position.size());
    glfwSwapBuffers(window);
}
//usuwanie obiektow
void usunObiekty() {
    delete player;
    delete mapa;
    delete podloga;
    delete coin;
    delete world;
    delete licznik;
    delete ghost_1;
    delete ghost_2;
    delete ghost_3;
    delete ghost_4;
}
bool checkLooser(Map* &mapa,colision_length colision_table[]) {
    //GHO1
    float odleglosc_krytyczna = colision_table[mPMAN].toX + colision_table[mGHO1].toX;
    float odleglosc = sqrt(pow(player->position.x + colision_table[mPMAN].toX - ghost_1->position.x - colision_table[mGHO1].toX, 2)
                           + pow(player->position.z + colision_table[mPMAN].toZ - ghost_1->position.z - colision_table[mGHO1].toZ, 2));
    if(odleglosc < odleglosc_krytyczna) {
        if(ghost_run) {
            ghost_1->position = ghost_1->default_position;
        } else return true;
    }
    //GHO2
    odleglosc_krytyczna = colision_table[mPMAN].toX + colision_table[mGHO2].toX;
    odleglosc = sqrt(pow(player->position.x + colision_table[mPMAN].toX - ghost_2->position.x - colision_table[mGHO1].toX, 2)
                     + pow(player->position.z + colision_table[mPMAN].toZ - ghost_2->position.z - colision_table[mGHO1].toZ, 2));
    if(odleglosc < odleglosc_krytyczna) {
        if(ghost_run) {
            ghost_2->position = ghost_2->default_position;
        } else return true;
    }
    //GHO3
    odleglosc_krytyczna = colision_table[mPMAN].toX + colision_table[mGHO3].toX;
    odleglosc = sqrt(pow(player->position.x + colision_table[mPMAN].toX - ghost_3->position.x - colision_table[mGHO1].toX, 2)
                     + pow(player->position.z + colision_table[mPMAN].toZ - ghost_3->position.z - colision_table[mGHO1].toZ, 2));
    if(odleglosc < odleglosc_krytyczna) {
        if(ghost_run) {
            ghost_3->position = ghost_3->default_position;
        } else return true;
    }
    //GHO4
    odleglosc_krytyczna = colision_table[mPMAN].toX + colision_table[mGHO4].toX;
    odleglosc = sqrt(pow(player->position.x + colision_table[mPMAN].toX - ghost_4->position.x - colision_table[mGHO1].toX, 2)
                     + pow(player->position.z + colision_table[mPMAN].toZ - ghost_4->position.z - colision_table[mGHO1].toZ, 2));
    if(odleglosc < odleglosc_krytyczna) {
        if(ghost_run) {
            ghost_4->position = ghost_4->default_position;
        } else return true;
    }
    return false;
}
int main(void) {
    configGame();
    game_start = false; // pierwsze rozpoczecie gry
    mapa->drawMapInConsole(true);
    srand(time(NULL));
    GLFWwindow* window; //WskaŸnik na obiekt reprezentuj¹cy okno
    glfwSetErrorCallback(error_callback);//Zarejestruj procedurê obs³ugi b³êdów
    if(!glfwInit()) {  //Zainicjuj bibliotekê GLFW
        fprintf(stderr, "Nie mo¿na zainicjowaæ GLFW.\n");
        exit(EXIT_FAILURE);
    }
    window = glfwCreateWindow(szerokoscOkna, wysokoscOkna, "Paca-Mana", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.
    if(!window) {  //Je¿eli okna nie uda³o siê utworzyæ, to zamknij program
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje siê aktywny i polecenia OpenGL bêd¹ dotyczyæ w³aœnie jego.
    glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora
    GLenum err;
    if((err=glewInit()) != GLEW_OK) {  //Zainicjuj bibliotekê GLEW
        fprintf(stderr, "Nie mo¿na zainicjowaæ GLEW: %s\n", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }
    initOpenGLProgram(window); //Operacje inicjuj¹ce
    glfwSetTime(0); //Wyzeruj timer
    //G³ówna pêtla
    // ustawienie duszkow
    while(!glfwWindowShouldClose(window)) {  //Tak d³ugo jak okno nie powinno zostaæ zamkniête
        if(game_start==false) {
            drawInfo(window,texStart);
        } else {
            if(game_end==false) {
                if(coin_position.size() % modulo_coin == 0) {
                    ghost_run = true;
                } else {
                    ghost_run = false;
                }
                if(coin_position.empty()) {
                    ghost_run = true;
                    game_win = true;
                    game_end = true;
                }
                if(checkLooser(mapa, colision_table)) {
                    game_end = true;
                    game_win = false;
                }
                doMove(mapa, colision_table, coin_position);
                ghost_1->doGhostMove(mapa, colision_table);
                ghost_2->doGhostMove(mapa, colision_table);
                ghost_3->doGhostMove(mapa, colision_table);
                ghost_4->doGhostMove(mapa, colision_table);
                coin->rotation_temp += (float)(glfwGetTime()*coin->speed);
                coin->rotation_temp = (float)(coin->rotation_temp - 2*PI*(ceil(coin->rotation_temp/(2*PI))));
                coin->rotation.y = coin->rotation_temp;
                glfwSetTime(0); //Wyzeruj timer
                drawGame(window); //Wykonaj procedurê rysuj¹c¹
            } else {
                if(game_win) {
                    drawInfo(window,texWin);
                } else {
                    drawInfo(window,texEnd);
                }
            }
        }
        glfwPollEvents(); //Wykonaj procedury callback w zaleznoœci od zdarzeñ jakie zasz³y.
    }
    //Usuniecie obiektow
    usunObiekty();
    //Usuniêcie tekstury z pamiêci karty graficznej – po g³ownej pêtli
    glDeleteTextures(1,&texSciana);
    glDeleteTextures(1,&texPodloga);
    glDeleteTextures(1,&texPlayer);
    glDeleteTextures(1,&texCoin);
    glDeleteTextures(1,&texWorld);
    glDeleteTextures(1,&texLicznik);
    glDeleteTextures(1,&texGhost1);
    glDeleteTextures(1,&texGhost2);
    glDeleteTextures(1,&texGhost3);
    glDeleteTextures(1,&texGhost4);
    glDeleteTextures(1,&texGhostX);
    glfwDestroyWindow(window); //Usuñ kontekst OpenGL i okno
    glfwTerminate(); //Zwolnij zasoby zajête przez GLFW
    exit(EXIT_SUCCESS);
}

#include <stdio.h>
#include <cstdlib>

#include "ghost.h"
#include "loaderOBJ.h"
#include "move.h"

#define mGhost1 "modeleBlend/ghost1.obj"
#define mGhost2 "modeleBlend/ghost2.obj"

using namespace glm;

Ghost::Ghost(Map* &mapa,colision_length &colision_length,int id) {
    bool res = loadOBJ(mGhost1, this->TEMPvertices, this->TEMPuvs, this->TEMPnormals,this->TEMPvCount,colision_length);
    if(!res) {
        fprintf(stderr,"Nie uda³o siê wczytaæ!");
        exit(1);
    }
    position = vec3(1.0,1.0,1.0);
    rotation = vec3(0.0,0.0,0.0);
    scale = vec3(1.0,1.0,1.0);
    speed = 2;
    obrot = abs((int)(rotation.y/((PI-0.1f)/2)));
    rotation_speed = PI;
    struct colision_length unusable;
    res = loadOBJ(mGhost2, this->TEMPvertices2, this->TEMPuvs2, this->TEMPnormals2,this->TEMPvCount2,unusable);
    if(!res) {
        fprintf(stderr,"Nie uda³o siê wczytaæ!");
        exit(1);
    }
    position2 = position;
    rotation2 = rotation;
    scale2 = scale;
    speed2 = 6;
    rotation_temp = rotation2.y;
    findPosition(mapa,id);
    ID = id;
    oldPZ = -1;
    oldPX = -1;
}

Ghost::~Ghost() {
    TEMPvertices.clear();
    TEMPuvs.clear();
    TEMPnormals.clear();
    TEMPvertices2.clear();
    TEMPuvs2.clear();
    TEMPnormals2.clear();
}

// znajduje poczatkowa pozycje pacmana na mapie
void Ghost::findPosition(Map* &mapa,int id) {
    for(int i=0; i<WYSOKOSC_MAPY; i++) {
        for(int j=0; j<SZEROKOSC_MAPY; j++) {
            if(mapa->mapa[i][j] == id) {
                this->position = vec3((float)i,0.75,(float)j);
                this->default_position = this->position;
                break;
            }
        }
    }
}

void Ghost::drawSolid(GLuint &tex,mat4 &V) {
    this->drawSolid_1(tex,V);
    this->drawSolid_2(tex,V);
}

void Ghost::drawSolid_1(GLuint &tex,mat4 &V) {
    glEnable(GL_NORMALIZE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindTexture(GL_TEXTURE_2D,tex);
    mat4 M=mat4(1.0f);
    M=translate(M,this->position);
    M=rotate(M,this->rotation.y,vec3(0.0,1.0,0.0));
    glLoadMatrixf(value_ptr(V*M));
    glVertexPointer(3,GL_FLOAT,0,&(this->TEMPvertices[0]));
    glNormalPointer(GL_FLOAT,sizeof(float)*3,&(this->TEMPnormals[0]));
    glTexCoordPointer(2,GL_FLOAT,0,&(this->TEMPuvs[0]));
    float ambient[] = {0,0,0,1};
    float emision[] = {0,0,0,1};
    float diffuse[] = {0.7,0.5,0.5,1};
    float specular[] = {0.5,0.5,0.5,1};
    float shininess = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emision);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    glDrawArrays(GL_TRIANGLES,0,this->TEMPvCount);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Ghost::drawSolid_2(GLuint &tex,mat4 &V) {
    glEnable(GL_NORMALIZE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindTexture(GL_TEXTURE_2D,tex);
    mat4 M=mat4(1.0f);
    M=translate(M,this->position);
    M=rotate(M,this->rotation.y,vec3(0.0,1.0,0.0));
    this->rotation_temp -= (float)(glfwGetTime()*speed2);
    this->rotation2.y = (float)(this->rotation_temp - 2*PI*(ceil(this->rotation_temp/(2*PI))));
    M=rotate(M,this->rotation2.y,vec3(0.0,1.0,0.0));
    glLoadMatrixf(value_ptr(V*M));
    glVertexPointer(3,GL_FLOAT,0,&(this->TEMPvertices2[0]));
    glNormalPointer(GL_FLOAT,sizeof(float)*3,&(this->TEMPnormals2[0]));
    glTexCoordPointer(2,GL_FLOAT,0,&(this->TEMPuvs2[0]));
    float ambient[] = {0,0,0,1};
    float emision[] = {0,0,0,1};
    float diffuse[] = {0.7,0.5,0.5,1};
    float specular[] = {0.5,0.5,0.5,1};
    float shininess = 50;
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emision);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    glDrawArrays(GL_TRIANGLES,0,this->TEMPvCount2);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Ghost::doGhostMove(Map* &mapa,colision_length colision_table[]) {
    int px = (int)(this->position.x+0.5f);
    int pz = (int)(this->position.z+0.5f);

    // mamy nowy kafelek
    if(fabs(this->position.x - oldPX) > 1.0 || fabs(this->position.z - oldPZ) > 1.0) {
        // Zaznaczenie zmiany położenia
        oldPX = px;
        oldPZ = pz;
        oldRotation = rotation.y;
        // UP - RIGHT - DOWN - LEFT
        bool kierunki[4] = {false, false, false, false};
        int mozliwosci = 0;
        if(mapa->mapa[px+1][pz] != mWALL) { // UP
            kierunki[0] = true;
            mozliwosci++;
        }
        if(mapa->mapa[px-1][pz] != mWALL) { // DOWN
            kierunki[2] = true;
            mozliwosci++;
        }
        if(mapa->mapa[px][pz+1] != mWALL) { // RIGHT
            kierunki[1] = true;
            mozliwosci++;
        }
        if(mapa->mapa[px][pz-1] != mWALL) { // LEFT
            kierunki[3] = true;
            mozliwosci++;
        }
        // Obliczenie obrotu
        obrot = abs((int)(rotation.y/((PI-0.1f)/2)));
        // Przegląd
        // proby aby nie szedl do tylu za czesto
        int proby = 3;
        while(proby>0) {
            int losowanie = rand() % mozliwosci ; // losowanie numeru kierunku
            int iter = 0;
            while(losowanie >= 0) { // poki mamy numer ;p
                while(kierunki[iter]==false) { // po tablicy poki falsz (nie mozna tam isc)
                    iter=(iter+1)%4;
                }
                losowanie--; // jak mozna to odhaczamy ze bylismy
                przemieszczenieID = iter; // wskauzjemy na ta droge
                iter=(iter+1)%4;
            }
            przemieszczenieID = (przemieszczenieID +4- obrot) % 4; // uzupelniamy o obrot
            proby--;
            if(przemieszczenieID != 2) { // gdy nie do tylu to wychodzimy z petli
                break;
            }
            // a jak nie to jeszcze raz moze sie uda nie 2 wylosowac
        }
    } else {
        // Kontynuacja ruchu
        int temp_obrot;
        switch(przemieszczenieID) {
        case 0: {
            // ustalenie kierunku aby po ilus tam obrotach nie chodzil po skosie
            this->rotation.y = -(abs((int)(rotation.y/((PI-0.2f)/2))))*(PI/2);
            goGhostSTRAIGHT(this, mapa, colision_table);
            break;
        }
        case 1: {
            // dodatkowy warunek w ifie bo jak byl obrot o  z 1st na 350 stopni to sie sypalo bo roznica byla wieksza niz 90 st...
            if((fabs(fabs(oldRotation) - fabs(rotation.y)) < ((PI-0.01f)/2))
                    || (fabs(fabs(oldRotation) - fabs(rotation.y)) > (2*PI-((PI-0.01f)/2)))) {
                rotateGhostRIGHT(this);
            } else {
                // ustalenie kierunku aby po ilus tam obrotach nie chodzil po skosie
                this->rotation.y = -(abs((int)(rotation.y/((PI-0.2f)/2))))*(PI/2);
                goGhostSTRAIGHT(this, mapa, colision_table);
            }
            break;
        }
        case 2: {
            // ustalenie kierunku aby po ilus tam obrotach nie chodzil po skosie
            this->rotation.y = -(abs((int)(rotation.y/((PI-0.2f)/2))))*(PI/2);
            goGhostBACK(this, mapa, colision_table);
            break;
        }
        case 3: {
            // dodatkowy warunek w ifie bo jak byl obrot o  z 1st na 350 stopni to sie sypalo bo roznica byla wieksza niz 90 st...
            if((fabs(fabs(oldRotation) - fabs(rotation.y)) < ((PI-0.01f)/2))
                    || (fabs(fabs(oldRotation) - fabs(rotation.y)) > (2*PI-((PI-0.01f)/2)))) {
                rotateGhostLEFT(this);
            } else {
                // ustalenie kierunku aby po ilus tam obrotach nie chodzil po skosie
                this->rotation.y = -(abs((int)(rotation.y/((PI-0.2f)/2))))*(PI/2);
                goGhostSTRAIGHT(this, mapa, colision_table);
            }
            break;
        }
        }
        WallDetect(mapa,colision_table);
    }
}

void Ghost::WallDetect(Map* &mapa,colision_length colision_table[]) {
    /*
        SCHEMAT DETEKCJI SCIAN
    X ^
    |   ________   ________   ________
    |  |        | |        | |        |
    |  |        | |        | |        |
    |  |   8   b| |   1    | |b  2    |
    |  |        | |        | |        |
    |  |___a____| |________| |___a____|
    |   ________   ________   ________
    |  |        | |        | |        |
    |  |        | |        | |        |
    |  |   7    | | GHOST  | |   3    |
    |  |        | |        | |        |
    |  |________| |________| |________|
    |   ________   ________   ________
    |  |   a    | |        | |   a    |
    |  |        | |        | |        |
    |  |   6   b| |   5    | |b  4    |
    |  |        | |        | |        |
    |  |________| |________| |________|
    |
    |------------------------------------->
                                        Z
    */
    int px = (int)(this->position.x+0.5f); // znajduje srodek pola w x
    int pz = (int)(this->position.z+0.5f); // znajduje srodek pola w z
    // dodawanie po 0.5 poniewaz tyle ma nasze pole w grze np. (0,0) pozycja to srodki bedzie na (0.5,0.5)
    /// Kolizja ze sciana nr 1
    if(mapa->mapa[px+1][pz]==1 && px+1>=0 && px+1<WYSOKOSC_MAPY && pz>=0 && pz <SZEROKOSC_MAPY) {
        if(colision_table[mapa->mapa[px+1][pz]].toX + colision_table[ID].toX
                >= fabs((px+1)-this->position.x)) {
            // zderzenie !
            //printf("Sciana 1\n");
            position.x = px+1 - (colision_table[mapa->mapa[px+1][pz]].toX + colision_table[ID].toX);
        }
    }
    /// Kolizja ze sciana nr 3
    if(mapa->mapa[px][pz+1]==1 && px>=0 && px<WYSOKOSC_MAPY && pz+1>=0 && pz+1 <SZEROKOSC_MAPY) {
        if(colision_table[mapa->mapa[px][pz+1]].toZ + colision_table[ID].toZ
                >= fabs((pz+1)-this->position.z)) {
            // zderzenie !
            //printf("Sciana 3\n");
            position.z = pz+1 - (colision_table[mapa->mapa[px][pz+1]].toZ + colision_table[ID].toZ);
        }
    }
    /// Kolizja ze sciana nr 5
    if(mapa->mapa[px-1][pz]==1 && px-1>=0 && px-1<WYSOKOSC_MAPY && pz>=0 && pz <SZEROKOSC_MAPY) {
        if(colision_table[mapa->mapa[px-1][pz]].toX + colision_table[ID].toX
                >= fabs((px-1)-this->position.x)) {
            // zderzenie !
            //printf("Sciana 5\n");
            position.x = px-1 + (colision_table[mapa->mapa[px-1][pz]].toX + colision_table[ID].toX);
        }
    }
    /// Kolizja ze sciana nr 7
    if(mapa->mapa[px][pz-1]==1 && px>=0 && px<WYSOKOSC_MAPY && pz-1>=0 && pz-1 <SZEROKOSC_MAPY) {
        if(colision_table[mapa->mapa[px][pz-1]].toZ + colision_table[ID].toZ
                >= fabs((pz-1)-this->position.z)) {
            // zderzenie !
            //printf("Sciana 7\n");
            position.z = pz-1 + (colision_table[mapa->mapa[px][pz-1]].toZ + colision_table[ID].toZ);
        }
    }
    /// Kolizja ze sciana nr 2
    if(mapa->mapa[px+1][pz+1]==1 && px+1>=0 && px+1<WYSOKOSC_MAPY && pz+1>=0 && pz+1 <SZEROKOSC_MAPY) {
        if(colision_table[mapa->mapa[px+1][pz+1]].toX + colision_table[ID].toX
                >= fabs((px+1)-this->position.x)
                && colision_table[mapa->mapa[px+1][pz+1]].toZ + colision_table[ID].toZ
                >= fabs((pz+1)-this->position.z)) {
            // zderzenie !
            if(fabs((pz+1)-this->position.z) < colision_table[mapa->mapa[px+1][pz+1]].toZ + colision_table[ID].toZ
                    && fabs((pz+1)-this->position.z) < fabs((px+1)-this->position.x)) {
                /// Kolizja ze sciana nr 2a
                //printf("Sciana 2a\n");
                position.x = px+1 - (colision_table[mapa->mapa[px+1][pz+1]].toX + colision_table[ID].toX);
            } else if(fabs((px+1)-this->position.x) < colision_table[mapa->mapa[px+1][pz+1]].toX + colision_table[ID].toX
                      && fabs((px+1)-this->position.x) < fabs((pz+1)-this->position.z)) {
                /// Kolizja ze sciana nr 2b
                //printf("Sciana 2b\n");
                position.z = pz+1 - (colision_table[mapa->mapa[px+1][pz+1]].toZ + colision_table[ID].toZ);
            }
        }
    }
    /// Kolizja ze sciana nr 4
    if(mapa->mapa[px-1][pz+1]==1 && px-1>=0 && px-1<WYSOKOSC_MAPY && pz+1>=0 && pz+1 <SZEROKOSC_MAPY) {
        if(colision_table[mapa->mapa[px-1][pz+1]].toX + colision_table[ID].toX
                >= fabs((px-1)-this->position.x)
                && colision_table[mapa->mapa[px-1][pz+1]].toZ + colision_table[ID].toZ
                >= fabs((pz+1)-this->position.z)) {
            // zderzenie !
            if(fabs((pz+1)-this->position.z) < colision_table[mapa->mapa[px-1][pz+1]].toZ + colision_table[ID].toZ
                    && fabs((pz+1)-this->position.z) < fabs((px-1)-this->position.x)) {
                /// Kolizja ze sciana nr 4a
                //printf("Sciana 4a\n");
                position.x = px-1 + (colision_table[mapa->mapa[px-1][pz+1]].toX + colision_table[ID].toX);
            } else if(fabs((px-1)-this->position.x) < colision_table[mapa->mapa[px-1][pz+1]].toX + colision_table[ID].toX
                      && fabs((px-1)-this->position.x) < fabs((pz+1)-this->position.z)) {
                /// Kolizja ze sciana nr 4b
                //printf("Sciana 4b\n");
                position.z = pz+1 - (colision_table[mapa->mapa[px-1][pz+1]].toZ + colision_table[ID].toZ);
            }
        }
    }
    /// Kolizja ze sciana nr 6
    if(mapa->mapa[px-1][pz-1]==1 && px-1>=0 && px-1<WYSOKOSC_MAPY && pz-1>=0 && pz-1 <SZEROKOSC_MAPY) {
        if(colision_table[mapa->mapa[px-1][pz-1]].toX + colision_table[ID].toX
                >= fabs((px-1)-this->position.x)
                && colision_table[mapa->mapa[px-1][pz-1]].toZ + colision_table[ID].toZ
                >= fabs((pz-1)-this->position.z)) {
            // zderzenie !
            if(fabs((pz-1)-this->position.z) < colision_table[mapa->mapa[px-1][pz-1]].toZ + colision_table[ID].toZ
                    && fabs((pz-1)-this->position.z) < fabs((px-1)-this->position.x)) {
                /// Kolizja ze sciana nr 6a
                //printf("Sciana 6a\n");
                position.x = px-1 + (colision_table[mapa->mapa[px-1][pz-1]].toX + colision_table[ID].toX);
            } else if(fabs((px-1)-this->position.x) < colision_table[mapa->mapa[px-1][pz-1]].toX + colision_table[ID].toX
                      && fabs((px-1)-this->position.x) < fabs((pz-1)-this->position.z)) {
                /// Kolizja ze sciana nr 6b
                //printf("Sciana 6b\n");
                position.z = pz-1 + (colision_table[mapa->mapa[px-1][pz-1]].toZ + colision_table[ID].toZ);
            }
        }
    }
    /// Kolizja ze sciana nr 8
    if(mapa->mapa[px+1][pz-1]==1 && px+1>=0 && px+1<WYSOKOSC_MAPY && pz-1>=0 && pz-1 <SZEROKOSC_MAPY) {
        if(colision_table[mapa->mapa[px+1][pz-1]].toX + colision_table[ID].toX
                >= fabs((px+1)-this->position.x)
                && colision_table[mapa->mapa[px+1][pz-1]].toZ + colision_table[ID].toZ
                >= fabs((pz-1)-this->position.z)) {
            // zderzenie !
            if(fabs((pz-1)-this->position.z) < colision_table[mapa->mapa[px+1][pz-1]].toZ + colision_table[ID].toZ
                    && fabs((pz-1)-this->position.z) < fabs((px+1)-this->position.x)) {
                /// Kolizja ze sciana nr 8a
                //printf("Sciana 8a\n");
                position.x = px+1 - (colision_table[mapa->mapa[px+1][pz-1]].toX + colision_table[ID].toX);
            } else if(fabs((px+1)-this->position.x) < colision_table[mapa->mapa[px+1][pz-1]].toX + colision_table[ID].toX
                      && fabs((px+1)-this->position.x) < fabs((pz-1)-this->position.z)) {
                /// Kolizja ze sciana nr 8b
                //printf("Sciana 8b\n");
                position.z = pz-1 + (colision_table[mapa->mapa[px+1][pz-1]].toZ + colision_table[ID].toZ);
            }
        }
    }
}


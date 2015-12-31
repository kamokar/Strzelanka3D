#ifndef __MD2_H
#define __MD2_H

#include <windows.h>			// standardowy plik nag��wkowy Windows
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gl/gl.h>						// standardowy plik nag��wkowy OpenGL
#include <gl/glu.h>						// plik nag��wkowy biblioteki GLU

#define MAX_FRAMES 512

// typy plik�w tekstur
enum texTypes_t
{
	PCX,
	BMP,
	TGA
};

// stany animacji modelu
enum modelState_t
{
	MODEL_IDLE,		// bezczynno��
	MODEL_CROUCH,		// kucanie
	MODEL_RUN,		// bieg
	MODEL_JUMP,		// skok
	MODEL_DIE,	//umiera
	MODEL_SHOT, //strzal
	MODEL_RESPAWN, //oznacza, ze gracz ma sie zrodzic w nowej pozycji
};

typedef struct 
{
	// w j�zyku C: enum texTypes textureType;
	// dalej b�dziemy u�ywa� tylko C++ 
	texTypes_t textureType;

	int width;					// szeroko�� tekstury
	int height;					// wysoko�� tekstury
	long int scaledWidth;
	long int scaledHeight;

	unsigned int texID;			// identyfikator obiektu tekstury
	unsigned char *data;		// dane tekstury
	unsigned char *palette;
} texture_t;

// cz�ciowy nag��wek pliku pcx
typedef struct
{
	unsigned char manufacturer;
	unsigned char version;
	unsigned char encoding;
	unsigned char bits;
	unsigned char xMin;
	unsigned char yMin;
	unsigned char xMax;
	unsigned char yMax;
	unsigned char *palette;
} PCXHEADER;

texture_t *LoadTexture(char *filename);

/*
	Wektory
*/
// pojedynczy wierzcho�ek
typedef struct
{
   float point[3];
} vector_t;

vector_t operator-(vector_t a, vector_t b);
vector_t operator*(float f, vector_t b);
vector_t operator/(vector_t a, vector_t b);
vector_t operator+(vector_t a, vector_t b);



/* 
	Funkcjonalno�� modelu MD2 
*/


// wsp�rz�dne tekstury
typedef struct
{
   float s;
   float t;
} texCoord_t;

// indeks wsp�rz�dnych tekstury
typedef struct
{
   short s;
   short t;
} stIndex_t;

// opis pojedynczego punktu klatki
typedef struct
{
   unsigned char v[3];
   unsigned char normalIndex;	// nie u�ywane
} framePoint_t;

// opis pojedynczej klatki
typedef struct
{
   float scale[3];
   float translate[3];
   char name[16];
   framePoint_t fp[1];
} frame_t;

// dane pojedynczego tr�jk�ta
typedef struct
{
   unsigned short meshIndex[3];		// indeksy wierzcho�k�w
   unsigned short stIndex[3];		// indeksy wsp�rz�dnych tekstury
} mesh_t;

typedef struct
{
   int ident;		// identyfikator formatu MD2: "IDP2"
   int version;		// wersja: 8
   int skinwidth;    // szeroko�� tekstury
   int skinheight;   // wysoko�� tekstury
   int framesize;    // liczba bajt�w klatki
   int numSkins;     // liczba tekstur
   int numXYZ;       // liczba wierzcho�k�w
   int numST;        // numer tekstury
   int numTris;      // liczba tr�jk�t�w
   int numGLcmds;
   int numFrames;    // ca�kowita liczba klatek
   int offsetSkins;  // pocz�tek nazw tekstur (64 bajty ka�da)
   int offsetST;     // pocz�tek wsp�rz�dnych tekstury
   int offsetTris;   // pocz�tek siatki tr�jk�t�w
   int offsetFrames; // pocz�tek danych klatki (wierzcho�k�w)
   int offsetGLcmds; // typ komend OpenGL
   int offsetEnd;    // koniec pliku
} modelHeader_t;


class CMD2Model
{
private:

     int numFrames;			// liczba klatek animacji
     int numVertices;         // liczba wierzcho�k�w modelu
     int numTriangles;        // liczba tr�jk�t�w modelu
     int numST;               // liczba tekstur modelu
     int frameSize;           // rozmiar klatki w bajtach
     int currentFrame;        // bie��ca klatka animacji
     int nextFrame;           // kolejna klatka animacji
     float interpol;          // wsp�czynnik interpolacji
     mesh_t *triIndex;        // lista tr�jk�t�w
     texCoord_t *st;          // lista wsp�rz�dnych tekstury
     vector_t *vertexList;    // lista wierzcho�k�w
     texture_t *modelTex;     // dane tekstury

	modelState_t modelState;	// stan animacji modelu

     void SetupSkin(texture_t *thisTexture);

public:

     CMD2Model();        // konstruktor
     ~CMD2Model();       // destruktor

     // �aduje model i jego tekstur�
     int Load(char *modelFile, char *skinFile);

     // �aduje tylko model
     int LoadModel(char *modelFile);

     // ��duje tylko tekstur�
     int LoadSkin(char *skinFile);

     // okre�la tekstur� modelu
     int SetTexture(texture_t *texture);

     // animuje model metod� interpolacji klatek kluczowych
     int Animate(int startFrame, int endFrame, float percent);

     // wy�wietla wybran� klatk� modelu
     int RenderFrame(int keyFrame);

     // zwalnia pami�� wykorzystywan� przez model
     int Unload();

	// okre�la stan modelu
	int SetState(modelState_t state);	

	// pobiera stan modelu
	modelState_t GetState();
};

#endif
#ifndef __MD2_H
#define __MD2_H

#include <windows.h>			// standardowy plik nag³ówkowy Windows
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gl/gl.h>						// standardowy plik nag³ówkowy OpenGL
#include <gl/glu.h>						// plik nag³ówkowy biblioteki GLU

#define MAX_FRAMES 512

// typy plików tekstur
enum texTypes_t
{
	PCX,
	BMP,
	TGA
};

// stany animacji modelu
enum modelState_t
{
	MODEL_IDLE,		// bezczynnoœæ
	MODEL_CROUCH,		// kucanie
	MODEL_RUN,		// bieg
	MODEL_JUMP,		// skok
	MODEL_DIE,	//umiera
	MODEL_SHOT, //strzal
	MODEL_RESPAWN, //oznacza, ze gracz ma sie zrodzic w nowej pozycji
};

typedef struct 
{
	// w jêzyku C: enum texTypes textureType;
	// dalej bêdziemy u¿ywaæ tylko C++ 
	texTypes_t textureType;

	int width;					// szerokoœæ tekstury
	int height;					// wysokoœæ tekstury
	long int scaledWidth;
	long int scaledHeight;

	unsigned int texID;			// identyfikator obiektu tekstury
	unsigned char *data;		// dane tekstury
	unsigned char *palette;
} texture_t;

// czêœciowy nag³ówek pliku pcx
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
// pojedynczy wierzcho³ek
typedef struct
{
   float point[3];
} vector_t;

vector_t operator-(vector_t a, vector_t b);
vector_t operator*(float f, vector_t b);
vector_t operator/(vector_t a, vector_t b);
vector_t operator+(vector_t a, vector_t b);



/* 
	Funkcjonalnoœæ modelu MD2 
*/


// wspó³rzêdne tekstury
typedef struct
{
   float s;
   float t;
} texCoord_t;

// indeks wspó³rzêdnych tekstury
typedef struct
{
   short s;
   short t;
} stIndex_t;

// opis pojedynczego punktu klatki
typedef struct
{
   unsigned char v[3];
   unsigned char normalIndex;	// nie u¿ywane
} framePoint_t;

// opis pojedynczej klatki
typedef struct
{
   float scale[3];
   float translate[3];
   char name[16];
   framePoint_t fp[1];
} frame_t;

// dane pojedynczego trójk¹ta
typedef struct
{
   unsigned short meshIndex[3];		// indeksy wierzcho³ków
   unsigned short stIndex[3];		// indeksy wspó³rzêdnych tekstury
} mesh_t;

typedef struct
{
   int ident;		// identyfikator formatu MD2: "IDP2"
   int version;		// wersja: 8
   int skinwidth;    // szerokoœæ tekstury
   int skinheight;   // wysokoœæ tekstury
   int framesize;    // liczba bajtów klatki
   int numSkins;     // liczba tekstur
   int numXYZ;       // liczba wierzcho³ków
   int numST;        // numer tekstury
   int numTris;      // liczba trójk¹tów
   int numGLcmds;
   int numFrames;    // ca³kowita liczba klatek
   int offsetSkins;  // pocz¹tek nazw tekstur (64 bajty ka¿da)
   int offsetST;     // pocz¹tek wspó³rzêdnych tekstury
   int offsetTris;   // pocz¹tek siatki trójk¹tów
   int offsetFrames; // pocz¹tek danych klatki (wierzcho³ków)
   int offsetGLcmds; // typ komend OpenGL
   int offsetEnd;    // koniec pliku
} modelHeader_t;


class CMD2Model
{
private:

     int numFrames;			// liczba klatek animacji
     int numVertices;         // liczba wierzcho³ków modelu
     int numTriangles;        // liczba trójk¹tów modelu
     int numST;               // liczba tekstur modelu
     int frameSize;           // rozmiar klatki w bajtach
     int currentFrame;        // bie¿¹ca klatka animacji
     int nextFrame;           // kolejna klatka animacji
     float interpol;          // wspó³czynnik interpolacji
     mesh_t *triIndex;        // lista trójk¹tów
     texCoord_t *st;          // lista wspó³rzêdnych tekstury
     vector_t *vertexList;    // lista wierzcho³ków
     texture_t *modelTex;     // dane tekstury

	modelState_t modelState;	// stan animacji modelu

     void SetupSkin(texture_t *thisTexture);

public:

     CMD2Model();        // konstruktor
     ~CMD2Model();       // destruktor

     // ³aduje model i jego teksturê
     int Load(char *modelFile, char *skinFile);

     // ³aduje tylko model
     int LoadModel(char *modelFile);

     // ³¹duje tylko teksturê
     int LoadSkin(char *skinFile);

     // okreœla teksturê modelu
     int SetTexture(texture_t *texture);

     // animuje model metod¹ interpolacji klatek kluczowych
     int Animate(int startFrame, int endFrame, float percent);

     // wyœwietla wybran¹ klatkê modelu
     int RenderFrame(int keyFrame);

     // zwalnia pamiêæ wykorzystywan¹ przez model
     int Unload();

	// okreœla stan modelu
	int SetState(modelState_t state);	

	// pobiera stan modelu
	modelState_t GetState();
};

#endif
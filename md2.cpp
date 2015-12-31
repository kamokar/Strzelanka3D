#include "md2.h"

// odejmowanie wektor�w
vector_t operator-(vector_t a, vector_t b)
{
	vector_t c;

	c.point[0] = a.point[0] - b.point[0];
	c.point[1] = a.point[1] - b.point[1];
	c.point[2] = a.point[2] - b.point[2];

	return c;
}

// mno�enie wektora przez skalar
vector_t operator*(float f, vector_t b)
{
	vector_t c;

	c.point[0] = f * b.point[0];
	c.point[1] = f * b.point[1];
	c.point[2] = f * b.point[2];

	return c;
}

// dzielenie wektor�w
vector_t operator/(vector_t a, vector_t b)
{
	vector_t c;

	c.point[0] = a.point[0] / b.point[0];
	c.point[1] = a.point[1] / b.point[1];
	c.point[2] = a.point[2] / b.point[2];

	return c;
}

// dodawanie wektor�w
vector_t operator+(vector_t a, vector_t b)
{
	vector_t c;

	c.point[0] = a.point[0] + b.point[0];
	c.point[1] = a.point[1] + b.point[1];
	c.point[2] = a.point[2] + b.point[2];

	return c;
}

// konstruktor klasy CMD2Model 
CMD2Model::CMD2Model()
{
     numVertices = 0;    // wierzcho�ki
     numTriangles = 0;   // tr�jk�ty
     numFrames = 0;      // klatki
     numST = 0;          // wsp�rz�dne tekstury
     frameSize = 0;      // rozmiar klatki
     currentFrame = 0;   // bie��ca klatka
     nextFrame = 1;      // nast�pna klatka
     interpol = 0.0;     // wsp�czynnik interpolacji
     triIndex = NULL;    // indeksy tr�jk�t�w
     st = NULL;          // indeksy wsp�rz�dnych tekstury
     vertexList = NULL;  // lista wierzcho�k�w
     modelTex = NULL;    // tekstura
	modelState = MODEL_IDLE;
}    

// destruktor klasy CMD2Model
CMD2Model::~CMD2Model()
{
}

// CMD2Model::SetupSkin()
// dost�p: prywatny
// opis: konfiguruje tekstur� modelu MD2
void CMD2Model::SetupSkin(texture_t *thisTexture)
{
     // konfiguruje tekstur� modelu MD2
     glGenTextures(1, &thisTexture->texID);
     glBindTexture(GL_TEXTURE_2D, thisTexture->texID);
     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
     
     switch (thisTexture->textureType)
     {
     case BMP:
          gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, thisTexture->width, thisTexture->height, 
               GL_RGB, GL_UNSIGNED_BYTE, thisTexture->data);
          break;
     case PCX:
          gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, thisTexture->width, thisTexture->height,
               GL_RGBA, GL_UNSIGNED_BYTE, thisTexture->data);
     case TGA:
          break;
     default:
          break;
     }
}

// CMD2Model::Load()
// dost�p: publiczny
// opis: �aduje model i tekstur�
int CMD2Model::Load(char *modelFile, char *skinFile)
{
     FILE *filePtr;						// wska�nik pliku
	int fileLen;						// d�ugo�� pliku
    char *buffer;						// bufor pliku
     
     modelHeader_t *modelHeader;             // nag��wek modelu
     
     stIndex_t *stPtr;                       // dane tekstury
     frame_t *frame;                              // dane klatki
     vector_t *vertexListPtr;                // zmienna indeksu
     mesh_t *bufIndexPtr;          // zmienna indeksu
     int i, j;                               // zmienne indeksu
     
     // otwiera plik modelu
     filePtr = fopen(modelFile, "rb");
     if (filePtr == NULL)
          return FALSE;
     
     // ustala d�ugo�� pliku
     fseek(filePtr, 0, SEEK_END);
     fileLen = ftell(filePtr);
     fseek(filePtr, 0, SEEK_SET);
     
     // wczytuje ca�y plik do bufora
     buffer = new char [fileLen+1];
     fread(buffer, sizeof(char), fileLen, filePtr);
     
     // wyodr�bnia nag��wek
     modelHeader = (modelHeader_t*)buffer;
     
     vertexList = new vector_t [modelHeader->numXYZ * modelHeader->numFrames];
     
     numVertices = modelHeader->numXYZ;
     numFrames = modelHeader->numFrames;
     frameSize = modelHeader->framesize;
     
     for (j = 0; j < numFrames; j++)
     {
          frame = (frame_t*)&buffer[modelHeader->offsetFrames + frameSize * j];
          
          vertexListPtr = (vector_t*)&vertexList[numVertices * j];
          for (i = 0; i < numVertices; i++)
          {
               vertexListPtr[i].point[0] = frame->scale[0] * frame->fp[i].v[0] + frame->translate[0];
               vertexListPtr[i].point[1] = frame->scale[1] * frame->fp[i].v[1] + frame->translate[1];
               vertexListPtr[i].point[2] = frame->scale[2] * frame->fp[i].v[2] + frame->translate[2];
          }
     }
     
     modelTex = LoadTexture(skinFile);
     if (modelTex != NULL)
          SetupSkin(modelTex);
     else
          return FALSE;
     
     numST = modelHeader->numST;
     st = new texCoord_t [numST];
     
     stPtr = (stIndex_t*)&buffer[modelHeader->offsetST];
     for (i = 0; i < numST; i++)
     {
          st[i].s = (float)stPtr[i].s / (float)modelTex->width;
          st[i].t = (float)stPtr[i].t / (float)modelTex->height;
     }
     
     numTriangles = modelHeader->numTris;
     triIndex = new mesh_t [numTriangles];
     
     // tymczasowy wska�nik bufora tr�jk�t�w
     bufIndexPtr = (mesh_t*)&buffer[modelHeader->offsetTris];
     
     // wype�nia list� tr�jk�t�w
     for (j = 0; j < numFrames; j++)         
     {
          // dla wszystkich tr�jk�t�w danej klatki
          for(i = 0; i < numTriangles; i++)
          {
               triIndex[i].meshIndex[0] = bufIndexPtr[i].meshIndex[0];
               triIndex[i].meshIndex[1] = bufIndexPtr[i].meshIndex[1];
               triIndex[i].meshIndex[2] = bufIndexPtr[i].meshIndex[2];
               triIndex[i].stIndex[0] = bufIndexPtr[i].stIndex[0];
               triIndex[i].stIndex[1] = bufIndexPtr[i].stIndex[1];
               triIndex[i].stIndex[2] = bufIndexPtr[i].stIndex[2];
          }
     }
     
     // zamyka plik i zwalnia jego bufor
     fclose(filePtr);
     free(buffer);
     
     currentFrame = 0;
     nextFrame = 1;
     interpol = 0.0;
     
     return TRUE;
}

// CMD2Model::LoadModel()
// dost�p: publiczny
// opis: �aduje model z pliku
int CMD2Model::LoadModel(char *modelFile)
{
     FILE *filePtr;                          // wska�nik pliku
     int fileLen;                            // d�ugo�� pliku
     char *buffer;                           // bufor pliku
     
     modelHeader_t *modelHeader;             // nag��wek modelu 
     
     stIndex_t *stPtr;                       // dane tekstury
     frame_t *frame;                              // dane klatki
     vector_t *vertexListPtr;                // zmienna indeksu
     mesh_t *triIndex, *bufIndexPtr;         // zmienne indeksu
     int i, j;                               // zmienne indeksu
     
     // otwiera plik modelu
     filePtr = fopen(modelFile, "rb");
     if (filePtr == NULL)
          return FALSE;
     
     // ustala d�ugo�� pliku
     fseek(filePtr, 0, SEEK_END);
     fileLen = ftell(filePtr);
     fseek(filePtr, 0, SEEK_SET);
     
     // wczytuje ca�y plik do bufora
     buffer = new char [fileLen+1];
     fread(buffer, sizeof(char), fileLen, filePtr);
     
     // wyodr�bnia nag��wek
     modelHeader = (modelHeader_t*)buffer;
     
     // przydziela pami�� na list� wierzcho�k�w
     vertexList = new vector_t [modelHeader->numXYZ * modelHeader->numFrames];
     
     numVertices = modelHeader->numXYZ;
     numFrames = modelHeader->numFrames;
     frameSize = modelHeader->framesize;
     
     for (j = 0; j < numFrames; j++)
     {
          frame = (frame_t*)&buffer[modelHeader->offsetFrames + frameSize * j];
          
          vertexListPtr = (vector_t*)&vertexList[numVertices * j];
          for (i = 0; i < numVertices; i++)
          {
               vertexListPtr[i].point[0] = frame->scale[0] * frame->fp[i].v[0] + frame->translate[0];
               vertexListPtr[i].point[1] = frame->scale[1] * frame->fp[i].v[1] + frame->translate[1];
               vertexListPtr[i].point[2] = frame->scale[2] * frame->fp[i].v[2] + frame->translate[2];
          }
     }
     
     numST = modelHeader->numST;
     
     st = new texCoord_t [numST];
     
     stPtr = (stIndex_t*)&buffer[modelHeader->offsetST];
     for (i = 0; i < numST; i++)
     {
          st[i].s = 0.0;
          st[i].t = 0.0;
     }
     
     numTriangles = modelHeader->numTris;
     triIndex = new mesh_t [numTriangles];
     
     // tymczasowy wska�nik bufora tr�jk�t�w
     bufIndexPtr = (mesh_t*)&buffer[modelHeader->offsetTris];
     
     // wype�nia list� tr�jk�t�w
     for (j = 0; j < numFrames; j++)         
     {
          // dla wszystkich tr�jk�t�w danej klatki
          for(i = 0; i < numTriangles; i++)
          {
               triIndex[i].meshIndex[0] = bufIndexPtr[i].meshIndex[0];
               triIndex[i].meshIndex[1] = bufIndexPtr[i].meshIndex[1];
               triIndex[i].meshIndex[2] = bufIndexPtr[i].meshIndex[2];
               triIndex[i].stIndex[0] = bufIndexPtr[i].stIndex[0];
               triIndex[i].stIndex[1] = bufIndexPtr[i].stIndex[1];
               triIndex[i].stIndex[2] = bufIndexPtr[i].stIndex[2];
          }
     }
     
     // zamyka plik
     fclose(filePtr);
     
     modelTex = NULL;
     currentFrame = 0;
     nextFrame = 1;
     interpol = 0.0;
     
     return 0;
}

// CMD2Model::LoadSkin()
// dost�p: publiczny
// opis: �aduje tekstur� z pliku
int CMD2Model::LoadSkin(char *skinFile)
{
     int i;
     
     modelTex = LoadTexture(skinFile);
     
     if (modelTex != NULL)
          SetupSkin(modelTex);
     else
          return -1;
     
     for (i = 0; i < numST; i++)
     {
          st[i].s /= (float)modelTex->width;
          st[i].t /= (float)modelTex->height;
     }
     
     return 0;
}

// CMD2Model::SetTexture()
// dost�p: publiczny
// opis: wybiera nowy obiekt tekstury
int CMD2Model::SetTexture(texture_t *texture)
{
     int i;
     
     if (texture != NULL)
	{
		free(modelTex);
          modelTex = texture;
	}
     else
          return -1;
     
     SetupSkin(modelTex);
     
     for (i = 0; i < numST; i++)
     {
          st[i].s /= (float)modelTex->width;
          st[i].t /= (float)modelTex->height;
     }
     
     return 0;
}

// CMD2Model::Animate()
// dost�p: publiczny
// opis: animuje model
int CMD2Model::Animate(int startFrame, int endFrame, float percent)
{
     vector_t *vList;              // wierzcho�ki bie��cej klatki
     vector_t *nextVList;          // wierzcho�ki nast�pnej klatki
     int i;                                  // zmienna indeksu
     float x1, y1, z1;                  // wsp�rz�dne wierzcho�ka bie��cej klatki
     float x2, y2, z2;                  // wsp�rz�dne wierzcho�ka nast�pnej klatki

     
     vector_t vertex[3]; 
     
     if ((startFrame > currentFrame))
          currentFrame = startFrame;
     
     if ((startFrame < 0) || (endFrame < 0))
          return -1;
     
     if ((startFrame >= numFrames) || (endFrame >= numFrames))
          return -1;
     
     if (interpol >= 1.0)
     {
          interpol = 0.0f;
          currentFrame++;
          if (currentFrame >= endFrame)
               currentFrame = startFrame;
          
          nextFrame = currentFrame + 1;
          
          if (nextFrame >= endFrame)
               nextFrame = startFrame;
          
     }
     
     vList = &vertexList[numVertices*currentFrame];
     nextVList = &vertexList[numVertices*nextFrame];
     
     glBindTexture(GL_TEXTURE_2D, modelTex->texID);
     glBegin(GL_TRIANGLES);
     for (i = 0; i < numTriangles; i++)
     {
          // wsp�rz�dne pierwszego wierzcho�ka tr�jk�ta w obu kluczowych klatkach
          x1 = vList[triIndex[i].meshIndex[0]].point[0];
          y1 = vList[triIndex[i].meshIndex[0]].point[1];
          z1 = vList[triIndex[i].meshIndex[0]].point[2];
          x2 = nextVList[triIndex[i].meshIndex[0]].point[0];
          y2 = nextVList[triIndex[i].meshIndex[0]].point[1];
          z2 = nextVList[triIndex[i].meshIndex[0]].point[2];
          
          // interpolowane wsp�rz�dne pierwszego wierzcho�ka 
          vertex[0].point[0] = x1 + interpol * (x2 - x1);
          vertex[0].point[1] = y1 + interpol * (y2 - y1);
          vertex[0].point[2] = z1 + interpol * (z2 - z1);
          
          // wsp�rz�dne drugiego wierzcho�ka tr�jk�ta
          x1 = vList[triIndex[i].meshIndex[2]].point[0];
          y1 = vList[triIndex[i].meshIndex[2]].point[1];
          z1 = vList[triIndex[i].meshIndex[2]].point[2];
          x2 = nextVList[triIndex[i].meshIndex[2]].point[0];
          y2 = nextVList[triIndex[i].meshIndex[2]].point[1];
          z2 = nextVList[triIndex[i].meshIndex[2]].point[2];
          
          // interpolowane wsp�rz�dne drugiego wierzcho�ka
          vertex[2].point[0] = x1 + interpol * (x2 - x1);
          vertex[2].point[1] = y1 + interpol * (y2 - y1);
          vertex[2].point[2] = z1 + interpol * (z2 - z1);   
          
          // wsp�rz�dne trzeciego wierzcho�ka tr�jk�ta
          x1 = vList[triIndex[i].meshIndex[1]].point[0];
          y1 = vList[triIndex[i].meshIndex[1]].point[1];
          z1 = vList[triIndex[i].meshIndex[1]].point[2];
          x2 = nextVList[triIndex[i].meshIndex[1]].point[0];
          y2 = nextVList[triIndex[i].meshIndex[1]].point[1];
          z2 = nextVList[triIndex[i].meshIndex[1]].point[2];
          
          // interpolowane wsp�rz�dne trzeciego wierzcho�ka
          vertex[1].point[0] = x1 + interpol * (x2 - x1);
          vertex[1].point[1] = y1 + interpol * (y2 - y1);
          vertex[1].point[2] = z1 + interpol * (z2 - z1);
          
         
          // rysuje tr�jk�t pokryty tekstur�
          glTexCoord2f(st[triIndex[i].stIndex[0]].s,
               st[triIndex[i].stIndex[0]].t);
          glVertex3fv(vertex[0].point);
          
          glTexCoord2f(st[triIndex[i].stIndex[2]].s ,
               st[triIndex[i].stIndex[2]].t);
          glVertex3fv(vertex[2].point);
          
          glTexCoord2f(st[triIndex[i].stIndex[1]].s,
               st[triIndex[i].stIndex[1]].t);
          glVertex3fv(vertex[1].point);
     }
     glEnd();
     
     interpol += percent;  // zwi�ksza wsp�czynnik interpolacji
     
     return 0;
}

// RenderFrame()
// opis: wy�wietla pojedyncz� klatk� kluczow�
int CMD2Model::RenderFrame(int keyFrame)
{
     vector_t *vList;
     int i;
     
     // wska�nik wy�wietlanej klatki
     vList = &vertexList[numVertices * keyFrame];
     
	if (modelTex != NULL)
		// wybiera tekstur�
		glBindTexture(GL_TEXTURE_2D, modelTex->texID);
     
     // wy�wietla klatk� modelu
     glBegin(GL_TRIANGLES);
     for(i = 0; i < numTriangles; i++)
     {

		if (modelTex != NULL)
			glTexCoord2f(st[triIndex[i].stIndex[0]].s,
				st[triIndex[i].stIndex[0]].t);

          glVertex3fv(vList[triIndex[i].meshIndex[0]].point);
          
		if (modelTex != NULL)
			glTexCoord2f(st[triIndex[i].stIndex[2]].s ,
				st[triIndex[i].stIndex[2]].t);

          glVertex3fv(vList[triIndex[i].meshIndex[2]].point);
          
		if (modelTex != NULL)
			glTexCoord2f(st[triIndex[i].stIndex[1]].s,
				st[triIndex[i].stIndex[1]].t);

          glVertex3fv(vList[triIndex[i].meshIndex[1]].point);
     }
     glEnd();
     
     return 0;
}

// Unload()
// opis: zwalnia zasoby modelu
int CMD2Model::Unload()
{
     if (triIndex != NULL)
          free(triIndex);
     if (vertexList != NULL)
          free(vertexList);
     if (st != NULL)
          free(st);
     
     return 0;
}

// SetState()
// opis: okre�la stan modelu
int CMD2Model::SetState(modelState_t state)
{
	modelState = state;
	return 0;
}

// GetState()
// opis: pobiera stan modelu
modelState_t CMD2Model::GetState()
{
	return modelState;
}

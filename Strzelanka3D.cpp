 #define WIN32_LEAN_AND_MEAN // nie includuje niepotrzebnych libow windowsa
#include <ws2tcpip.h>
#include <cmath>
#include <time.h>
#include <gl\glaux.h>
#include "sound.h"
#include "md2.h"

//bool fullscreen=FALSE;	// pelny ekran?? 
bool fullscreen=TRUE;
const int SZER = 1920;
const int WYS = 1080;			//rozdzielczosc ekranu
const int maxCzastek=50;
const int maxStrzalow=50;

#define SERWER "kamok"		//tutaj podac nazwe hosta lub IP serwera
#define PORT "55555"
bool jestSerwerem;			//aplikacja dziala w trybie serwer czy klient?
bool przeciwnikTrafiony = FALSE;
bool zostalesZabity = FALSE;
int wynik;
SOCKET socketSluchacz = INVALID_SOCKET;
SOCKET socketLaczeniowy = INVALID_SOCKET;
float liczba, przecX, przecZ, przecObrot;
float infoOtrz, infoWyslij;
modelState_t stanModeluGracz, stanModeluPrzec, temp;	// stan modelu otrzymany z sieci oraz tymczasowy
char* bufor = (char *) &liczba;			//bufor uzywany do wysylania i odbierania danych z sieci
char* bufor2 = (char *) &temp;
int frame_counter = 0, frame_counter2 = 0;
int punktyGracza = 0, punktyPrzeciwnika = 0;



HDC		hDC=NULL;			// Private GDI Device Context
HGLRC	hRC=NULL;			// Permanent Rendering Context
HWND	hWnd=NULL;			// Holds Our Window Handle
DirectSoundBuffer Dzwiek1[maxStrzalow];	//do pracy wystarcza 2 bufory dzwiekowe
DirectSoundBuffer Dzwiek2[maxStrzalow];
DirectSoundManager SoundManager; 

double mouse_x, mouse_y;
double myszX, myszY;
int mouse_wrap_x = 0;
int mouse_wrap_y = 0;
double czulosc = 4;			// czulosc myszki

bool klawisze[256];			// tablica trzymajaca kolejke wcisnietych klawiszy
bool fp;					// tymczasowa do obliczania fps'ow
bool active=TRUE;			// okno domyslnie aktywne
bool zliczFPSy=FALSE;

GLuint texture[28];					// ile mamy tekstur do wczytania?
GLuint fonty;						// Base Display List For The Font

//static GLuint ROOM;					// Storage For The Room Display List
const double piprzez180 = 0.0174532925f;	// obliczone pi/180
const int maxKomorek = 1000;

double pozX;// = rand()%maxKomorek*2-maxKomorek;	// poz x poczatkowa
double pozZ;					// poz x poczatkowa
double pozY = 40;					// poz y poczatkowa
double xtra = 0;					// poz -x
double ztra = 0;					// poz -z
double XP=0;						// przyspieszenie gracza w kierunku osi X
double ZP=0;						// przyspieszenie gracza w kierunku osi Z
double sc_obrY;					// kat obrotu sceny
double obrot;						// kat obrotu gracza
double _heading = 0;
double zprot;						// biezaca predkosc obrotu klawiszami kursora

double czas1;
double czas2;
double roznCzasu;
double startCzas;
double klatekNaSek = 0;				// klatki na sekunde
int klatki = 0;						// ogolny licznik klatek

bool jestKarabin=FALSE;
float karabin_kop = 0;
float karabin_kopi = 0;
bool jestLaser=TRUE;
bool jestLaser_blast=FALSE;
bool isFire = FALSE;
bool isFireComplete = TRUE;
bool jestRykoszet = FALSE;
double pustaZmienna = 0;			// do wywolywania funkcji kolizji
bool done=FALSE;					// jesli 1 to wyjscie z programu
double laser_blast_life = 1;
double laser_blast_life2 = 1;
double laser_blast_life3 = .5f;
double laser_blast_life3i = .015f;
double laser_blast_size = .1f;
int laser_blast_delay = 0;
double fire_x = 0;
double fire_y = 10;
double fire_xp, fire_yp;
double fire_z = 0;
double fire_zp = 0;
double fire_delay = 0;
int shots_fired = 0;
int is_Fired = 0; 
float q;


CMD2Model *model_gracza;		// model postaci
CMD2Model *model_broni;			// model broni
unsigned char *LoadPCXFile(char *filename, PCXHEADER *pcxHeader)
{
     int idx = 0;                  // licznik indeksu
     int c;                             // pobiera znak z pliku
     int i;                             // licznik indeksu
     int numRepeat;      
     FILE *filePtr;                // wskaŸnik pliku
     int width;                         // szerokoœæ obrazka PCX
     int height;                        // wysokoœæ obrazka PCX
     unsigned char *pixelData;     // dane obrazka PCX
     unsigned char *paletteData;   // dane palety PCX

     // otwiera plik PCX
     filePtr = fopen(filename, "rb");
     if (filePtr == NULL)
          return NULL;

     // pobiera pierwszy znak z pliku, który powinien mieæ wartoœæ 10
     c = getc(filePtr);
     if (c != 10)
     {
          fclose(filePtr);
          return NULL;
     }

     // pobiera nastêpny znak, który powinien mieæ wartoœæ 5
     c = getc(filePtr);
     if (c != 5)
     {
          fclose(filePtr);
          return NULL;
     }

     // ustawia wskaŸnik pliku na jego pocz¹tek
     rewind(filePtr);

     // pomija pierwsze 4 znaki
     fgetc(filePtr);
     fgetc(filePtr);
     fgetc(filePtr);
     fgetc(filePtr);

     // pobiera wspó³rzêdn¹ x lewej krawêdzi obrazka PCX
     pcxHeader->xMin = fgetc(filePtr);       // mniej znacz¹ce s³owo
     pcxHeader->xMin |= fgetc(filePtr) << 8; // bardziej znacz¹ce s³owo

     // pobiera wspó³rzêdn¹ y dolnej krawêdzi obrazka PCX
     pcxHeader->yMin = fgetc(filePtr);       // mniej znacz¹ce s³owo
     pcxHeader->yMin |= fgetc(filePtr) << 8; // bardziej znacz¹ce s³owo

     // pobiera wspó³rzêdn¹ x prawej krawêdzi obrazka PCX
     pcxHeader->xMax = fgetc(filePtr);       // mniej znacz¹ce s³owo
     pcxHeader->xMax |= fgetc(filePtr) << 8; // bardziej znacz¹ce s³owo

     // pobiera wspó³rzêdn¹ y górnej krawêdzi obrazka PCX
     pcxHeader->yMax = fgetc(filePtr);       // mniej znacz¹ce s³owo
     pcxHeader->yMax |= fgetc(filePtr) << 8; // bardziej znacz¹ce s³owo

     // oblicza szerokoœæ i wysokoœæ obrazka PCX
     width = pcxHeader->xMax - pcxHeader->xMin + 1;
     height = pcxHeader->yMax - pcxHeader->yMin + 1;

     // przydziela pamiêæ na dane obrazka PCX
     pixelData = (unsigned char*)malloc(width*height);

     // ustwaia wskaŸnik pliku na 128. bajt, gdzie zaczynaj¹ siê dane obrazka
     fseek(filePtr, 128, SEEK_SET);
     
     // dekoduje i przechowuje piksele obrazka
     while (idx < (width*height))
     {
          c = getc(filePtr);
          if (c > 0xbf)
          {
               numRepeat = 0x3f & c;
               c = getc(filePtr);

               for (i = 0; i < numRepeat; i++)
               {
                    pixelData[idx++] = c;
               }
          }
          else
               pixelData[idx++] = c;

          fflush(stdout);
     }

     // przydziela pamiêæ na paletê obrazka PCX
     paletteData = (unsigned char*)malloc(768);

     // paleta zajmuje 769 koñcowych bajtów pliku PCX
     fseek(filePtr, -769, SEEK_END);

     // weryfikuje paletê; pierwszy znak powinien mieæ wartoœæ 12
     c = getc(filePtr);
     if (c != 12)
     {
          fclose(filePtr);
          return NULL;
     }

     // wczytuje paletê
     for (i = 0; i < 768; i++)
     {
          c = getc(filePtr);
          paletteData[i] = c;
     }

     // zamyka plik i umieszcza wskaŸnik palety w nag³ówku
     fclose(filePtr);
     pcxHeader->palette = paletteData;

     // zwraca wskaŸnik do danych obrazka
     return pixelData;
}
texture_t *LoadPCXTexture(char *filename)
{
     PCXHEADER texInfo;            // nag³ówek
     texture_t *thisTexture;       // tekstura
     unsigned char *unscaledData;
     int i;                             // zmienna indeksowa
     int j;                             // zmienna indeksowa
     int width;                         // szerokoœæ tekstury
     int height;                        // wysokoœæ tekstury

     // przydziela pamiêæ strukturze opisuj¹cej teksturê
     thisTexture = (texture_t*)malloc(sizeof(texture_t));
     if (thisTexture == NULL)
          return NULL;

     // ³aduje plik PCX
     thisTexture->data = LoadPCXFile(filename, &texInfo);
     if (thisTexture->data == NULL)
     {
          free(thisTexture->data);
          return NULL;
     }

     // zapamiêtuje informacje o teksturze
     thisTexture->palette = texInfo.palette;
     thisTexture->width = texInfo.xMax - texInfo.xMin + 1;
     thisTexture->height = texInfo.yMax - texInfo.yMin + 1;
     thisTexture->textureType = PCX;

     // przydziela bufor dla nieprzeskalowanych danych
     unscaledData = (unsigned char*)malloc(thisTexture->width*thisTexture->height*4);

     // zapamietuje nieprzeskalowane dane
     for (j = 0; j < thisTexture->height; j++) 
     {
          for (i = 0; i < thisTexture->width; i++) 
          {
               unscaledData[4*(j*thisTexture->width+i)+0] = (unsigned char)thisTexture->palette[3*thisTexture->data[j*thisTexture->width+i]+0];
               unscaledData[4*(j*thisTexture->width+i)+1] = (unsigned char)thisTexture->palette[3*thisTexture->data[j*thisTexture->width+i]+1];
               unscaledData[4*(j*thisTexture->width+i)+2] = (unsigned char)thisTexture->palette[3*thisTexture->data[j*thisTexture->width+i]+2];
               unscaledData[4*(j*thisTexture->width+i)+3] = (unsigned char)255;
          }
     }

     // szerokoœæ i wysokoœæ jako najbli¿sze potêgi 2
     width = thisTexture->width;
     height = thisTexture->height;

     // szerokoœæ
     i = 0;
     while (width)
     {
          width /= 2;
          i++;
     }
     thisTexture->scaledHeight = (long)pow(2.0, i-1);

     // wysokoœæ
     i = 0;
     while (height)
     {
          height /= 2;
          i++;
     }
     thisTexture->scaledWidth = (long)pow(2.0, i-1);

     // zwalnia bufor
     if (thisTexture->data != NULL)
     {
          free(thisTexture->data);
          thisTexture->data = NULL;
     }

     
     thisTexture->data = (unsigned char*)malloc(thisTexture->scaledWidth*thisTexture->scaledHeight*4);
     
     // skaluje teksturê
     gluScaleImage (GL_RGBA, thisTexture->width, thisTexture->height, GL_UNSIGNED_BYTE, unscaledData, thisTexture->scaledWidth, thisTexture->scaledHeight, GL_UNSIGNED_BYTE, thisTexture->data);

     return thisTexture;
}
texture_t *LoadTexture(char *filename)
{
	texture_t *thisTexture;
	char *extStr;

	// wyodrêbnia rozszerzenie nazwy pliku
	extStr = strchr(filename, '.');
	extStr++;

	// w oparciu o rozszerzenie ³aduje teksturê z pliku w danym formacie
	thisTexture = LoadPCXTexture(filename);
	/*
	else if ((strcmp(extStr, "TGA") == 0) || (strcmp(extStr, "tga") == 0) )
		thisTexture = LoadTGATexture(filename);
		//texType = TGA;
	*/
	return thisTexture;
}
void CleanUp()		// opis: zwalnia zasoby modeli
{

	model_gracza->Unload();
	delete model_gracza;
	model_broni->Unload();
	delete model_broni;
}
class WIERZCHOLEK
{
public:
	float x, y, z;
	float u, v;
};
class TROJKAT
{
public:
	int numer_tekstury;
	WIERZCHOLEK wierzcholek[3];
};
class SEKTOR
{
public:
	int iloscTrojkatow;
	TROJKAT* trojkat;
};
SEKTOR sektor1;
class tekstura
{
public:
	GLubyte	*imageData;							// Image Data (Up To 32 Bits)
	GLuint	bpp;								// Image Color Depth In Bits Per Pixel.
	GLuint	width;								// Image Width
	GLuint	height;								// Image Height
	GLuint	texID;								// Texture ID Used To Select A Texture
};
tekstura textures[2];							// miejsce na 1 teksture
class czasteczki
{
public:
	GLdouble	life;					// Particle Life
	GLdouble	fade;					// Fade Speed
	GLdouble	size;					// Particle size
	GLdouble	triangle_x1;						// X vertex
	GLdouble	triangle_x2;						// X vertex
	GLdouble	triangle_x3;						// X vertex
	GLdouble	triangle_y1;						// Y vertex
	GLdouble	triangle_y2;						// Y vertex
	GLdouble	triangle_y3;						// Y vertex
	GLdouble	triangle_z1;						// Z vertex
	GLdouble	triangle_z2;						// Z vertex
	GLdouble	triangle_z3;						// Z vertex
	GLdouble	triangle_rotate_x;					// X Rotate
	GLdouble	triangle_rotate_y;					// X Rotate
	GLdouble	triangle_rotate_z;					// X Rotate
	GLdouble	triangle_rotate_xi;					// X Rotate
	GLdouble	triangle_rotate_yi;					// X Rotate
	GLdouble	triangle_rotate_zi;					// X Rotate
	GLdouble	x;						// X Position
	GLdouble	y;						// Y Position
	GLdouble	z;						// Z Position
	GLdouble	xi;						// X Direction
	GLdouble	yi;						// Y Direction
	GLdouble	zi;						// Z Direction
};
czasteczki czastki[maxCzastek][maxStrzalow];	// klasa i macierz dla czasteczek
void readstr(FILE *f, char *string)
{
	do
	{
		fgets(string, 255, f);
	} while ((string[0] == '/') || (string[0] == '\n'));
}
void wczytajSwiat()
{
	float x, y, z, u, v;
	int iloscTrojkatow, tmp;
	FILE *filein;
	char linijka[255];
	filein = fopen("dane/poziom1.txt", "rt");

	readstr(filein, linijka);
	sscanf(linijka, "ILOSC_TROJK %d\n", &iloscTrojkatow);

	sektor1.trojkat = new TROJKAT[iloscTrojkatow];
	sektor1.iloscTrojkatow = iloscTrojkatow;
	for (int i = 0; i < iloscTrojkatow; i++)
	{
		readstr(filein,linijka);
		sscanf(linijka, "%d", &tmp);
		sektor1.trojkat[i].numer_tekstury=tmp;

		for (int j = 0; j < 3; j++)
		{
			readstr(filein,linijka);
			sscanf(linijka, "%f %f %f %f %f", &x, &y, &z, &u, &v);
			sektor1.trojkat[i].wierzcholek[j].x = x;
			sektor1.trojkat[i].wierzcholek[j].y = y;
			sektor1.trojkat[i].wierzcholek[j].z = z;
			sektor1.trojkat[i].wierzcholek[j].u = u;
			sektor1.trojkat[i].wierzcholek[j].v = v;
		}
	}
	fclose(filein);
	return;
}
void mapka()
{
	GLfloat x_m, y_m, z_m, u_m, v_m;
	int iloscTrojkatow;
	iloscTrojkatow = sektor1.iloscTrojkatow;
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	for (int i = 0; i < iloscTrojkatow; i++)
	{
		glBindTexture(GL_TEXTURE_2D, texture[sektor1.trojkat[i].numer_tekstury]);
		glBegin(GL_TRIANGLES);
			x_m = sektor1.trojkat[i].wierzcholek[0].x;
			y_m = sektor1.trojkat[i].wierzcholek[0].y;
			z_m = sektor1.trojkat[i].wierzcholek[0].z;
			u_m = sektor1.trojkat[i].wierzcholek[0].u;
			v_m = sektor1.trojkat[i].wierzcholek[0].v;
			glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);
			
			x_m = sektor1.trojkat[i].wierzcholek[1].x;
			y_m = sektor1.trojkat[i].wierzcholek[1].y;
			z_m = sektor1.trojkat[i].wierzcholek[1].z;
			u_m = sektor1.trojkat[i].wierzcholek[1].u;
			v_m = sektor1.trojkat[i].wierzcholek[1].v;
			glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);
			
			x_m = sektor1.trojkat[i].wierzcholek[2].x;
			y_m = sektor1.trojkat[i].wierzcholek[2].y;
			z_m = sektor1.trojkat[i].wierzcholek[2].z;
			u_m = sektor1.trojkat[i].wierzcholek[2].u;
			v_m = sektor1.trojkat[i].wierzcholek[2].v;
			glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);
		glEnd();
	}
}
double pitagoras(double a, double b)		// pitagoras a*a + b*b = c*c
{
	return sqrt((a*a)+(b*b));
}
bool LoadTGA(tekstura *texture, char *filename)			// Loads A TGA File Into Memory
{    
	GLubyte		TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
	GLubyte		TGAcompare[12];								// Used To Compare TGA Header
	GLubyte		header[6];									// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;								// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;									// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;										// Temporary Variable
	GLuint		type=GL_RGBA;								// Set The Default GL Mode To RBGA (32 BPP)

	FILE *file = fopen(filename, "rb");						// Open The TGA File

	if(	file==NULL ||										// Does File Even Exist?
		fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0				||	// Does The Header Match What We Want?
		fread(header,1,sizeof(header),file)!=sizeof(header))				// If So Read Next 6 Header Bytes
	{
		if (file == NULL)									// Did The File Even Exist? *Added Jim Strong*
			return false;									// Return False
		else
		{
			fclose(file);									// If Anything Failed, Close The File
			return false;									// Return False
		}
	}

	texture->width  = header[1] * 256 + header[0];			// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];			// Determine The TGA Height	(highbyte*256+lowbyte)

	if(	texture->width	<=0	||								// Is The Width Less Than Or Equal To Zero
		texture->height	<=0	||								// Is The Height Less Than Or Equal To Zero
		(header[4]!=24 && header[4]!=32))					// Is The TGA 24 or 32 Bit?
	{
		fclose(file);										// If Anything Failed, Close The File
		return false;										// Return False
	}

	texture->bpp	= header[4];							// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel	= texture->bpp/8;						// Divide By 8 To Get The Bytes Per Pixel
	imageSize		= texture->width*texture->height*bytesPerPixel;	// Calculate The Memory Required For The TGA Data

	texture->imageData=(GLubyte *)malloc(imageSize);		// Reserve Memory To Hold The TGA Data

	if(	texture->imageData==NULL ||							// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file)!=imageSize)	// Does The Image Size Match The Memory Reserved?
	{
		if(texture->imageData!=NULL)						// Was Image Data Loaded
			free(texture->imageData);						// If So, Release The Image Data

		fclose(file);										// Close The File
		return false;										// Return False
	}

	for(GLuint i=0; i<int(imageSize); i+=bytesPerPixel)		// Loop Through The Image Data
	{														// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp=texture->imageData[i];							// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];	// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;					// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose (file);											// Close The File

	// Build A Texture From The Data
	glGenTextures(1, &texture[0].texID);					// Generate OpenGL texture IDs

	glBindTexture(GL_TEXTURE_2D, texture[0].texID);			// Bind Our Texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Filtered

	if (texture[0].bpp==24)									// Was The TGA 24 Bits
	{
		type=GL_RGB;										// If So Set The 'type' To GL_RGB
	}

	gluBuild2DMipmaps(GL_TEXTURE_2D, type, texture[0].width, texture[0].height, type, GL_UNSIGNED_BYTE, texture[0].imageData);           

	return true;											// Texture Building Went Ok, Return True
}
void BuildFont()									// Build Our Font Display List
{
	fonty=glGenLists(256);									// Creating 256 Display Lists
	glBindTexture(GL_TEXTURE_2D, textures[0].texID);		// Select Our Font Texture
	for (int loop1=0; loop1<256; loop1++)					// Loop Through All 256 Lists
	{
		float cx=float(loop1%16)/16.0f;						// X Position Of Current Character
		float cy=float(loop1/16)/16.0f;						// Y Position Of Current Character

		glNewList(fonty+loop1,GL_COMPILE);					// Start Building A List
		glBegin(GL_QUADS);								// Use A Quad For Each Character
		glTexCoord2f(cx,1.0f-cy-0.0625f);			// Texture Coord (Bottom Left)
		glVertex2d(0,16);							// Vertex Coord (Bottom Left)
		glTexCoord2f(cx+0.0625f,1.0f-cy-0.0625f);	// Texture Coord (Bottom Right)
		glVertex2i(16,16);							// Vertex Coord (Bottom Right)
		glTexCoord2f(cx+0.0625f,1.0f-cy-0.001f);	// Texture Coord (Top Right)
		glVertex2i(16,0);							// Vertex Coord (Top Right)
		glTexCoord2f(cx,1.0f-cy-0.001f);			// Texture Coord (Top Left)
		glVertex2i(0,0);							// Vertex Coord (Top Left)
		glEnd();										// Done Building Our Quad (Character)
		glTranslated(14,0,0);							// Move To The Right Of The Character
		glEndList();										// Done Building The Display List
	}														// Loop Until All 256 Are Built
}
void glPrint(GLint x, GLint y, int set, const char *fmt, ...)	// Where The Printing Happens
{
	char		text[1024];									// Holds Our String
	va_list		ap;											// Pointer To List Of Arguments

	if (fmt == NULL)										// If There's No Text
		return;												// Do Nothing

	va_start(ap, fmt);										// Parses The String For Variables
	vsprintf(text, fmt, ap);							// And Converts Symbols To Actual Numbers
	va_end(ap);												// Results Are Stored In Text

	if (set>1)												// Did User Choose An Invalid Character Set?
	{
		set=1;												// If So, Select Set 1 (Italic)
	}

	glEnable(GL_TEXTURE_2D);								// Enable Texture Mapping
	glLoadIdentity();										// Reset The Modelview Matrix
	glTranslated(x,y,-500);									// Position The Text (0,0 - Top Left)
	glRotatef(180,0,0,1);
	glRotatef(180,0,1,0);
	glListBase(fonty-32+(128*set));							// Choose The Font Set (0 or 1)

	glCallLists(strlen(text),GL_UNSIGNED_BYTE, text);		// Write The Text To The Screen
	glDisable(GL_TEXTURE_2D);								// Disable Texture Mapping
}
AUX_RGBImageRec *LoadBMP(char *Filename)				// Loads A Bitmap Image
{
	FILE *File=NULL;									// File Handle

	if (!Filename)										// Make Sure A Filename Was Given
	{
		return NULL;									// If Not Return NULL
	}

	File=fopen(Filename,"r");							// Check To See If The File Exists

	if (File)											// Does The File Exist?
	{
		fclose(File);									// Close The Handle
		return auxDIBImageLoad(Filename);				// Load The Bitmap And Return A Pointer
	}

	return NULL;										// If Load Failed Return NULL
}
struct									// Create A Structure For The Timer Information
{
	__int64	frequency;					// Timer Frequency
	GLdouble	resolution;				// Timer Resolution
	unsigned long mm_timer_start;		// Multimedia Timer Start Value    
	unsigned long mm_timer_elapsed;		// Multimedia Timer Elapsed Time
	bool   performance_timer;    

										// Using The Performance Timer?
	__int64 performance_timer_start;	// Performance Timer Start Value
	__int64 performance_timer_elapsed;	// Performance Timer Elapsed Time
} timer;
void TimerInit()						// Initialize Our Timer
{
	memset(&timer, 0, sizeof(timer));   
	// Clear Our Timer Structure
	// Check To See If A Performance Counter Is Available
	// If One Is Available The Timer Frequency Will Be Updated
	if (!QueryPerformanceFrequency((LARGE_INTEGER *) &timer.frequency))
	{
		// No Performace Counter Available
		timer.performance_timer = FALSE;                    // Set Performance Timer To FALSE
		timer.mm_timer_start = timeGetTime();               // Use timeGetTime()
		timer.resolution  = 1.0f/1000.0f;                   // Set Our Timer Resolution To .001f
		timer.frequency   = 1000;                           // Set Our Timer Frequency To 1000
		timer.mm_timer_elapsed = timer.mm_timer_start;		// Set The Elapsed Time
	}
	else
	{
		// Performance Counter Is Available, Use It Instead Of The Multimedia Timer
		// Get The Current Time And Store It In performance_timer_start
		QueryPerformanceCounter((LARGE_INTEGER *) &timer.performance_timer_start);
		timer.performance_timer   = TRUE;    // Set Performance Timer To TRUE
		// Calculate The Timer Resolution Using The Timer Frequency
		timer.resolution    = (GLdouble) (((double)1.0f)/((double)timer.frequency));
		// Set The Elapsed Time To The Current Time
		timer.performance_timer_elapsed = timer.performance_timer_start;
	}
}
double TimerGetTime()				// Get Time In Milliseconds
{
	__int64 time;					// 'time' Will Hold A 64 Bit Integer
	if (timer.performance_timer)	// Are We Using The Performance Timer?
	{
		QueryPerformanceCounter((LARGE_INTEGER *) &time); // Current Performance Time
		// Return The Time Elapsed since TimerInit was called
		return ( (GLdouble) ( time - timer.performance_timer_start) * timer.resolution)*1000.0f;
	}
	else
	{
		// Return The Time Elapsed since TimerInit was called
		return ( (GLdouble) ( timeGetTime() - timer.mm_timer_start) * timer.resolution)*1000.0f;
	}
}
int wczytajTekstury()									// Load Bitmap And Convert To A Texture
{
	int Status=FALSE;								// Status Indicator
	AUX_RGBImageRec *tekstura[1];				// Create Storage Space For The Textures
	memset(tekstura,0,sizeof(void *)*1);		// Set The Pointer To NULL

	if (tekstura[0]=LoadBMP("dane/sciana.bmp"))	// Load Particle Texture
	{
		Status=TRUE;								// Set The Status To TRUE
		glGenTextures(1, &texture[2]);				// Create One Texture

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);           
	}

	if (tekstura[0]=LoadBMP("dane/plotek.bmp"))	// Load Particle Texture
	{
		Status=TRUE;								// Set The Status To TRUE
		glGenTextures(1, &texture[3]);				// Create One Texture

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, texture[3]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);           
	}

	if (tekstura[0]=LoadBMP("dane/kolo.bmp"))	// Load Particle Texture
	{
		Status=TRUE;								// Set The Status To TRUE
		glGenTextures(1, &texture[10]);				// Create One Texture

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, texture[10]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);           
	}

	if (tekstura[0]=LoadBMP("dane/karabin.bmp"))	// Load Particle Texture
	{
		Status=TRUE;								// Set The Status To TRUE
		glGenTextures(1, &texture[11]);				// Create One Texture

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, texture[11]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);           
	}

	if (tekstura[0]=LoadBMP("dane/laser.bmp"))	// Load Particle Texture
	{
		Status=TRUE;								// Set The Status To TRUE
		glGenTextures(1, &texture[12]);				// Create One Texture

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, texture[12]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);           
	}

	if (tekstura[0]=LoadBMP("dane/laser_blast.bmp"))	// Load Particle Texture
	{
		Status=TRUE;								// Set The Status To TRUE
		glGenTextures(1, &texture[13]);				// Create One Texture

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, texture[13]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);           
	}

	if (tekstura[0]=LoadBMP("dane/laser_blast2.bmp"))	// Load Particle Texture
	{
		Status=TRUE;								// Set The Status To TRUE
		glGenTextures(1, &texture[14]);				// Create One Texture

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, texture[14]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);           
	}

	if (tekstura[0]=LoadBMP("dane/celownik.bmp"))	// Load Particle Texture
	{
		Status=TRUE;								// Set The Status To TRUE
		glGenTextures(1, &texture[15]);				// Create One Texture

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, texture[15]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);           
	}

	if (tekstura[0]=LoadBMP("dane/dymek.bmp"))	// Load Particle Texture
	{
		Status=TRUE;								// Set The Status To TRUE
		glGenTextures(1, &texture[16]);				// Create One Texture

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, texture[16]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);           
	}

	if (tekstura[0]=LoadBMP("dane/sufit.bmp"))	// Load Particle Texture
	{
		Status=TRUE;								// Set The Status To TRUE
		glGenTextures(1, &texture[17]);				// Create One Texture

		// Create MipMapped Texture
		glBindTexture(GL_TEXTURE_2D, texture[17]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB8, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);           
	}

	if (tekstura[0]=LoadBMP("dane/kamien.bmp"))
    {
            Status=TRUE;
            glGenTextures(1, &texture[18]);
			glBindTexture(GL_TEXTURE_2D, texture[18]);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);
    }

	if (tekstura[0]=LoadBMP("dane/podloga.bmp"))
    {
            Status=TRUE;
            glGenTextures(1, &texture[19]);
			glBindTexture(GL_TEXTURE_2D, texture[19]);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);
    }

	if (tekstura[0]=LoadBMP("dane/dach.bmp"))
    {
            Status=TRUE;
            glGenTextures(1, &texture[20]);
			glBindTexture(GL_TEXTURE_2D, texture[20]);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);
    }

	if (tekstura[0]=LoadBMP("dane/niebo.bmp"))
    {
            Status=TRUE;
            glGenTextures(1, &texture[23]);
			glBindTexture(GL_TEXTURE_2D, texture[23]);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);
    }

	if (tekstura[0]=LoadBMP("dane/trawa.bmp"))
    {
            Status=TRUE;
            glGenTextures(1, &texture[25]);
			glBindTexture(GL_TEXTURE_2D, texture[25]);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, tekstura[0]->sizeX, tekstura[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, tekstura[0]->data);
    }


	if (tekstura[0])							// If Texture Exists
	{
		if (tekstura[0]->data)					// If Texture Image Exists
		{
			free(tekstura[0]->data);			// Free The Texture Image Memory
		}
		free(tekstura[0]);						// Free The Image Structure
	}

	return Status;									// Return The Status
}
void przeskalujScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}


	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	glLoadIdentity();									// Reset The Modelview Matrix
	gluPerspective(60.0f,(GLdouble)width/(GLdouble)height,1.0f,22250.0f);	 //ostatni param. to wielkosc sceny

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}
int inicjujGL()													// All Setup For OpenGL Goes Here
{
	if (!LoadTGA(&textures[0],"dane/Font.TGA"))					// Load The Font Texture
	{
		return false;											// If Loading Failed, Return False
	}

	if (!SoundManager.Initialize(hWnd)) 
	{
		return false;
	} 

	for (int i = 0; i < maxStrzalow; i++)
	{
		SoundManager.CreateSound("dane/karabin.wav", Dzwiek1[i]);
		Dzwiek1[i].SetFrequency(44100);
	}

	for (int i = 0; i < maxStrzalow; i++)
	{
		SoundManager.CreateSound("dane/laser.wav", Dzwiek2[i]);
	}

	LPARAM lParam=0;
	SetCursorPos(322,340);						//poczatkowa pozycja celownika
	float temp_mouse_x = LOWORD(lParam);
	float temp_mouse_y = HIWORD(lParam);
	SetCursorPos(320,340);

	if (!wczytajTekstury())								// skok do wczytywania tekstur
		return false;  

	glEnable(GL_TEXTURE_2D);							// mapowanie tekstur
	glEnable(GL_DEPTH_TEST);							// bufor g³êbi
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// t³o w kolorze czarnym
	glShadeModel(GL_SMOOTH);							// cieniowanie g³adkie
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);

	// tworzy obiekty modeli
	model_gracza = new CMD2Model;
	model_broni = new CMD2Model;

	// ³aduje modele i ich tekstury
	if (jestSerwerem)
	{
		model_gracza->Load("models\\Sodf8\\tris.md2", "models\\Sodf8\\Abarlith.pcx");
		model_broni->Load("models\\Sodf8\\weapon.md2", "models\\Sodf8\\weapon.pcx");
	}
	else
	{
		model_gracza->Load("models\\Ogro\\tris.md2", "models\\Ogro\\Ogrobase.pcx");
		model_broni->Load("models\\Ogro\\weapon.md2", "models\\Ogro\\weapon.pcx");
	}

	wczytajSwiat();
	TimerInit();										//zainicjowanie licznika czasu
	BuildFont();   
	startCzas = TimerGetTime()/1000; 
	return TRUE;										//wszystko ok
}
bool kolizja(GLdouble &cx, GLdouble &cy, GLdouble &cz, GLdouble &cxi, GLdouble &cyi, GLdouble &czi, GLdouble padding, GLdouble bounce)
{
	// Detect collision and reflect movement if collision is detected
	// cx = x position; cy = y position; cz = z position;
	// cxi = x increment; cyi = y increment; czi = z increment;
	// padding = a square border around the object.  your value will be 1/2 the length of one side of the square
	// bounce = bounce coefficient -- 0 = no bounce; 1 = perfect bounce
	bool Status = false;
	if (cx > maxKomorek-padding)
	{	
		cx = maxKomorek-padding;
		cxi = -cxi;
		cxi *= bounce; 
		Status = true;
	}
	else if (cx < -maxKomorek+padding)
	{
		cx = -maxKomorek+padding;
		cxi = -cxi;
		cxi *= bounce; 
		Status = true;
	}

	if (cy > maxKomorek-padding)
	{
		cy = maxKomorek-padding;
		cyi = -cyi;
		cyi *= bounce; 
		Status = true;
	}
	else if (cy < 0+padding)		
	{
		cy = 0+padding;
		cyi = -cyi;
		cyi *= bounce; 
		Status = true;
	}

	if (cz > maxKomorek-padding)		
	{
		cz = maxKomorek-padding;
		czi = -czi;
		czi *= bounce; 
		Status = true;
	}
	else if (cz < -maxKomorek+padding)		
	{
		cz = -maxKomorek+padding;
		czi = -czi;
		czi *= bounce; 
		Status = true;
	}

	//if (cx < maxKomorek/2+26+padding && cx > maxKomorek/2-26-padding && cz < maxKomorek/2+26+padding && cz > maxKomorek/2-26-padding && cy < 8+padding && cy > 6-padding)
	//{
	//	if (cx < maxKomorek/2+26+padding && cx > maxKomorek/2+25.75f-padding)
	//	{
	//		cx = maxKomorek/2+26+padding;
	//		cxi = -cxi;
	//		cxi *= bounce; 
	//	}
	//	else if (cx > maxKomorek/2-26-padding && cx < maxKomorek/2-25.75f+padding)
	//	{
	//		cx = maxKomorek/2-26-padding;
	//		cxi = -cxi;
	//		cxi *= bounce; 
	//	}


	//	if (cy < 8+padding && cy > 7.75f-padding)
	//	{
	//		cyi = -cyi;
	//		cyi *= bounce; 
	//	}
	//	else if (cy > 6-padding && cy < 6.25f+padding)
	//	{
	//		cyi = -cyi;
	//		cyi *= bounce; 
	//	}

	//	if (cz < maxKomorek/2+26+padding && cz > maxKomorek/2+25.75f-padding)
	//	{
	//		cz = maxKomorek/2+26+padding;
	//		czi = -czi;
	//		czi *= bounce; 
	//	}
	//	else if (cz > maxKomorek/2-26-padding && cz < maxKomorek/2-25.75f+padding)
	//	{
	//		cz = maxKomorek/2-26-padding;
	//		czi = -czi;
	//		czi *= bounce;
	//	}

	//	Status = true;
	//}

	return Status;
}
void odlamki()
{
	for (int i = 0; i < maxCzastek; i++)
	{
		if (i == 0)
			czastki[i][shots_fired].size = 8;
		else
			czastki[i][shots_fired].size = GLdouble((rand()%25)+25)/50;

		czastki[i][shots_fired].triangle_x1 = GLdouble((rand()%50)-25)/75.f;	//tu wielkosc odlamkow ;)
		czastki[i][shots_fired].triangle_x2 = GLdouble((rand()%50)-25)/75.f;
		czastki[i][shots_fired].triangle_x3 = GLdouble((rand()%50)-25)/75.f;

		czastki[i][shots_fired].triangle_y1 = GLdouble((rand()%50)-25)/75.f;
		czastki[i][shots_fired].triangle_y2 = GLdouble((rand()%50)-25)/75.f;
		czastki[i][shots_fired].triangle_y3 = GLdouble((rand()%50)-25)/75.f;

		czastki[i][shots_fired].triangle_z1 = GLdouble((rand()%50)-25)/75.f;
		czastki[i][shots_fired].triangle_z2 = GLdouble((rand()%50)-25)/75.f;
		czastki[i][shots_fired].triangle_z3 = GLdouble((rand()%50)-25)/75.f;

		czastki[i][shots_fired].triangle_rotate_x = rand()%360;
		czastki[i][shots_fired].triangle_rotate_y = rand()%360;
		czastki[i][shots_fired].triangle_rotate_z = rand()%360;

		czastki[i][shots_fired].triangle_rotate_xi = GLdouble((rand()%50)-25)/5;
		czastki[i][shots_fired].triangle_rotate_yi = GLdouble((rand()%50)-25)/5;
		czastki[i][shots_fired].triangle_rotate_zi = GLdouble((rand()%50)-25)/5;

		czastki[i][shots_fired].life = 1;
		czastki[i][shots_fired].fade = GLdouble(rand()%100)/5000 + .02f;
		czastki[i][shots_fired].x = fire_x;
		czastki[i][shots_fired].y = fire_y;
		czastki[i][shots_fired].z = fire_z;
		GLdouble angle = rand()%360;
		GLdouble angle2 = rand()%360;
		GLdouble speed = GLdouble(rand()%100) / 500;
		GLdouble Hyp = pitagoras(angle,angle2);

		czastki[i][shots_fired].xi = sin(pitagoras(angle,Hyp))*speed + fire_xp;
		czastki[i][shots_fired].yi = cos(pitagoras(angle,Hyp))*speed + fire_yp;
		czastki[i][shots_fired].zi = cos(pitagoras(angle2,Hyp))*speed + fire_zp;
	}
}
int renderujScene()
{

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);	//czyszcznie sceny i bufora glebi
	glBlendFunc(GL_SRC_ALPHA,GL_ZERO); 

	XP *= 0.9f;				//zmniejszenie aktualnego przyspieszenia gracza do zera
	ZP *= 0.9f; 
	pozX += XP/10;		//do aktualnego przesuniecia sceny dodajemy aktualne przyspieszenie
	pozZ += ZP/10;
	xtra = -pozX;		//przesuniecie sceny w keirunku przeciwnym do ruchu gracza (symulacja ruchu)
	ztra = -pozZ;

	kolizja(pozX, pozY, pozZ, XP, pustaZmienna, ZP, 2.5f, 0);	//sprawdzanie kolizji gracza z otoczeniem

	zprot*=.9f;				//zmnieszenie aktualnego przyspieszenia katowego do zera (obracanie klawiszami kursora)
	_heading += zprot;		//do aktualnego obrotu sceny w poziomie dodanie przyspieszenia katowego
	obrot = myszX + _heading;	//uwzglednienie poleznia myszki w poziomie
	sc_obrY = 360.0f - obrot;		//obrot sceny w przeciwnym kierunku niz obrot gracz (symulacja obrotu)

	glLoadIdentity();

	karabin_kopi += -karabin_kop/50;			//odrzucenie gracza do tylu przez karabin
	karabin_kop += karabin_kopi/10;
	karabin_kopi *= .8f;
	karabin_kop *= .9f;
	if (karabin_kop != 0)
	{
		glRotated(-karabin_kop,1,0,0);
		glTranslatef(0,0,-karabin_kop);
	}

	glRotated(myszY,1.f,0,0);				//obrot kata kamery gora-dol
	glRotated(sc_obrY,0,1.f,0);				//obrot kamery prawo-lewo
	glTranslated(xtra, -pozY, ztra);		//przesuniecie kemery x, y, z

	mapka();									//rysuj wczytana mapke




	if (jestSerwerem)
	{
		recv( socketLaczeniowy, bufor, 4, 0 ); przecX=liczba;		//otrzymanie od klienta jego pozycji x, z i obrot
		recv( socketLaczeniowy, bufor, 4, 0 ); przecZ=liczba;		//przesylanie floata jako ciag danych 4-bajtowych
		recv( socketLaczeniowy, bufor, 4, 0 ); przecObrot=liczba;
		recv( socketLaczeniowy, bufor, 4, 0 ); infoOtrz=liczba;
		recv( socketLaczeniowy, bufor2, 4, 0 ); if ( stanModeluPrzec != MODEL_DIE) stanModeluPrzec=temp;

		liczba=pozX; send( socketLaczeniowy, bufor, 4, 0 );			//wysylanie do serwera poz x,z i obrot
		liczba=pozZ; send( socketLaczeniowy, bufor, 4, 0 );
		liczba=obrot; send( socketLaczeniowy, bufor, 4, 0 );
		liczba=infoWyslij; send( socketLaczeniowy, bufor, 4, 0 ); infoWyslij=0;
		temp=stanModeluGracz; send( socketLaczeniowy, bufor2, 4, 0 );
	}
	else
	{
		liczba=pozX; send( socketLaczeniowy, bufor, 4, 0 );			//wysylanie do serwera poz x,z i obrot
		liczba=pozZ; send( socketLaczeniowy, bufor, 4, 0 );
		liczba=obrot; send( socketLaczeniowy, bufor, 4, 0 );
		liczba=infoWyslij; send( socketLaczeniowy, bufor, 4, 0 ); infoWyslij=0;
		temp=stanModeluGracz; send( socketLaczeniowy, bufor2, 4, 0 );

		recv( socketLaczeniowy, bufor, 4, 0 ); przecX=liczba;		//otrzymanie od klienta jego pozycji x, z i obrot
		recv( socketLaczeniowy, bufor, 4, 0 ); przecZ=liczba;		//przesylanie floata jako ciag danych 4-bajtowych
		recv( socketLaczeniowy, bufor, 4, 0 ); przecObrot=liczba;
		recv( socketLaczeniowy, bufor, 4, 0 ); infoOtrz=liczba;
		recv( socketLaczeniowy, bufor2, 4, 0 ); if ( stanModeluPrzec != MODEL_DIE) stanModeluPrzec=temp;
	}

	if ( infoOtrz == 1.0 )		//jesli zostales zabity, wylosuj nowa pozycje
	{
		infoOtrz = 0.0;
		pozX = rand()%maxKomorek*2-maxKomorek;
		pozZ = rand()%maxKomorek*2-maxKomorek;
		punktyPrzeciwnika++;
		zostalesZabity = TRUE;
	}



	if (isFire)			// czy padl strzal?
	{
		if (jestKarabin)
		{
			Dzwiek1[shots_fired].Play ( 0 );
			karabin_kopi = 1;
		}
		else
			Dzwiek2[shots_fired].Play ( 0 );

		shots_fired++;
		if (shots_fired > maxStrzalow-1) shots_fired = 0;
		isFireComplete = false;
		isFire = false;

		fire_x = -xtra;
		fire_y = 10;
		fire_z = -ztra;

		fire_xp = -(GLdouble)sin(obrot*piprzez180) / 5;	
		fire_yp = -(GLdouble)tan(myszY*piprzez180) / 20;
		fire_zp = -(GLdouble)cos(obrot*piprzez180) / 5;

		// padl strzal ale czy trafil w przeciwnika? porownanie kata strzalu i kata przeciwnika
		if ( abs(( ( (pozX-przecX)/pitagoras((pozX-przecX),(pozZ-przecZ)) )   - sin(obrot*piprzez180) ) ) < 8 / pitagoras((pozX-przecX),(pozZ-przecZ)))
		{
			przeciwnikTrafiony = TRUE;
			stanModeluPrzec = MODEL_DIE;
		}
	}

	if (przeciwnikTrafiony)
	{
		frame_counter++;
		if (frame_counter > 37 )
		{
			frame_counter = 0;
			przeciwnikTrafiony = FALSE;
			stanModeluPrzec = MODEL_IDLE;
			punktyGracza++;
			infoWyslij=1.0;		//1=respawn przeciwnika
		}
	}

	
	glPushMatrix();												// renderowanie modelu przeciwnika
	glTranslated( przecX, 24, przecZ);  
	glRotatef( przecObrot+90, 0.0f, 1.0f, 0.0f);
	glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
	glColor3f(1.0, 1.0, 1.0);

	// wybiera stan modeli
	model_gracza->SetState(stanModeluPrzec);
	model_broni->SetState(stanModeluPrzec);

	// wykonuje animacje
	switch (model_gracza->GetState())
	{
	case MODEL_IDLE:
		model_gracza->Animate(0, 39, 0.2f);
		model_broni->Animate(0, 39, 0.2f);
		break;
	case MODEL_RUN:
		model_gracza->Animate(40, 46, 0.2f);
		model_broni->Animate(40, 46, 0.2f);
		break;
	case MODEL_SHOT:
		model_gracza->Animate(47, 54, 0.2f);
		model_broni->Animate(47, 54, 0.2f);
		break;
	case MODEL_CROUCH:
		model_gracza->Animate(136, 154, 0.2f);
		model_broni->Animate(136, 154, 0.2f);
		break;
	case MODEL_DIE:
		model_gracza->Animate(178, 185, 0.1f);
		model_broni->Animate(178, 185, 0.1f);
		break;
	default:
		model_gracza->Animate(0, 39, 0.2f);
		model_broni->Animate(0, 39, 0.2f);
		break;
	}
	glPopMatrix();

	//przeciwnikTrafiony = FALSE;
	stanModeluGracz = MODEL_IDLE;

	 


	if(zliczFPSy){
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINES);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);			//osie ukladu wspolrzednych
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(2200.0f, 0.0f, 0.0f);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 2200.0f, 0.0f);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);			
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 2200.0f);
	glEnd();
	glBegin(GL_QUADS);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);						// KWADRAT
		glVertex3f(0.0f, 0.0f, 0.0f);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 100.0f, 0.0f);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);			
		glVertex3f(100.0f, 100.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);			
		glVertex3f(100.0f, 0.0f, 0.0f);
	glEnd();

	q+=0.03;
	glPushMatrix();
	glTranslated(-200, 0.1, 500);
	glTranslated( 30*sin(q), 0.1, 30*cos(q));  
	glBegin(GL_QUADS);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);						// KWADRAT ruchomy
		glVertex3f(0.0f, 0.0f, 0.0f);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 100.0f, 0.0f);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);			
		glVertex3f(100.0f, 100.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);			
		glVertex3f(100.0f, 0.0f, 0.0f);
	glEnd();
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
	}





	int fire_counter = 0;

	// obliczenia wystrzalow
	while (!isFireComplete)
	{     
		fire_counter++;
		fire_x += fire_xp;
		fire_y += fire_yp;
		fire_z += fire_zp;

		if (kolizja(fire_x,  fire_y,  fire_z, pustaZmienna, pustaZmienna, pustaZmienna, 0, 0))
		{
			isFireComplete = true;
			if (jestKarabin)
			{
				odlamki();
				jestRykoszet = true;
			}
			else if (jestLaser)
			{
				if (fire_z > maxKomorek) 
					fire_z = maxKomorek; 
				else if (fire_z < -maxKomorek) 
					fire_z = -maxKomorek; 
				if (fire_x > maxKomorek) 
					fire_x = maxKomorek; 
				else if (fire_x < -maxKomorek) 
					fire_x = -maxKomorek; 
				jestLaser_blast = true; 
			}
		}
		else if (fire_counter > 10000)
		{
			isFireComplete = true;
		} 
	}

	if (laser_blast_delay > 0) laser_blast_delay--;

	// jesli wystrzelil laser
	if (jestLaser_blast)
	{
		laser_blast_delay = 5;
		laser_blast_life -= .015f;
		laser_blast_life2 -= .1f;
		laser_blast_life3 += laser_blast_life3i;
		laser_blast_life3i -= .0001f*laser_blast_size;
		laser_blast_size += .5f;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);

		GLdouble alpha;
		float r = .25f;
		float g = .4f;
		float b = 1;
		GLdouble light_size = 100;
		glBindTexture(GL_TEXTURE_2D, texture[10]);
		alpha = laser_blast_life3 - (fire_y/light_size);
		if (alpha < 0) alpha = 0;
		else
		{
			glColor4d(r,g,b,alpha);
			glBegin(GL_QUADS);
			glTexCoord2d(1,1); glVertex3d(fire_x + alpha*light_size, 0.1f, fire_z + alpha*light_size);
			glTexCoord2d(1,0); glVertex3d(fire_x + alpha*light_size, 0.1f, fire_z - alpha*light_size);
			glTexCoord2d(0,0); glVertex3d(fire_x - alpha*light_size, 0.1f, fire_z - alpha*light_size);
			glTexCoord2d(0,1); glVertex3d(fire_x - alpha*light_size, 0.1f, fire_z + alpha*light_size);
			glEnd();
		}

		alpha = laser_blast_life3 - ((maxKomorek-fire_y)/light_size);
		if (alpha < 0) alpha = 0;
		else
		{
			glColor4d(r,g,b,alpha);
			glBegin(GL_QUADS);
			glTexCoord2d(1,1); glVertex3d(fire_x + alpha*light_size, maxKomorek-.1f, fire_z + alpha*light_size);
			glTexCoord2d(1,0); glVertex3d(fire_x + alpha*light_size, maxKomorek-.1f, fire_z - alpha*light_size);
			glTexCoord2d(0,0); glVertex3d(fire_x - alpha*light_size, maxKomorek-.1f, fire_z - alpha*light_size);
			glTexCoord2d(0,1); glVertex3d(fire_x - alpha*light_size, maxKomorek-.1f, fire_z + alpha*light_size);
			glEnd();
		}

		//alpha = laser_blast_life3 - (fire_z/light_size);
		//if (alpha < 0) alpha = 0;
		//else
		//{
		//	glColor4d(r,g,b,alpha);
		//	glBegin(GL_QUADS);
		//	glTexCoord2d(1,1); glVertex3d(fire_x + alpha*light_size, fire_y + alpha*light_size, .1f);
		//	glTexCoord2d(1,0); glVertex3d(fire_x + alpha*light_size, fire_y - alpha*light_size, .1f);
		//	glTexCoord2d(0,0); glVertex3d(fire_x - alpha*light_size, fire_y - alpha*light_size, .1f);
		//	glTexCoord2d(0,1); glVertex3d(fire_x - alpha*light_size, fire_y + alpha*light_size, .1f);
		//	glEnd();
		//}

		alpha = laser_blast_life3 - ((maxKomorek-fire_z)/light_size);
		if (alpha < 0) alpha = 0;
		else
		{
			glColor4d(r,g,b,alpha);
			glBegin(GL_QUADS);
			glTexCoord2d(1,1); glVertex3d(fire_x + alpha*light_size, fire_y + alpha*light_size, maxKomorek-.1f);
			glTexCoord2d(1,0); glVertex3d(fire_x + alpha*light_size, fire_y - alpha*light_size, maxKomorek-.1f);
			glTexCoord2d(0,0); glVertex3d(fire_x - alpha*light_size, fire_y - alpha*light_size, maxKomorek-.1f);
			glTexCoord2d(0,1); glVertex3d(fire_x - alpha*light_size, fire_y + alpha*light_size, maxKomorek-.1f);
			glEnd();
		}

		//alpha = laser_blast_life3 - ((-maxKomorek+fire_x)/light_size);
		//if (alpha < 0) alpha = 0;
		//else
		//{
		//	glColor4d(r,g,b,alpha);
		//	glBegin(GL_QUADS);
		//	glTexCoord2d(1,1); glVertex3d(-maxKomorek+.1f, fire_y + alpha*light_size, fire_z + alpha*light_size);
		//	glTexCoord2d(1,0); glVertex3d(-maxKomorek+.1f, fire_y - alpha*light_size, fire_z + alpha*light_size);
		//	glTexCoord2d(0,0); glVertex3d(-maxKomorek+.1f, fire_y - alpha*light_size, fire_z - alpha*light_size);
		//	glTexCoord2d(0,1); glVertex3d(-maxKomorek+.1f, fire_y + alpha*light_size, fire_z - alpha*light_size);
		//	glEnd();
		//}

		alpha = laser_blast_life3 - ((maxKomorek-fire_x)/light_size);
		if (alpha < 0) alpha = 0;
		else
		{
			glColor4d(r,g,b,alpha);
			glBegin(GL_QUADS);
			glTexCoord2d(1,1); glVertex3d(maxKomorek-.1f, fire_y + alpha*light_size, fire_z + alpha*light_size);
			glTexCoord2d(1,0); glVertex3d(maxKomorek-.1f, fire_y - alpha*light_size, fire_z + alpha*light_size);
			glTexCoord2d(0,0); glVertex3d(maxKomorek-.1f, fire_y - alpha*light_size, fire_z - alpha*light_size);
			glTexCoord2d(0,1); glVertex3d(maxKomorek-.1f, fire_y + alpha*light_size, fire_z - alpha*light_size);
			glEnd();	
		}

		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);

		glLoadIdentity();

		glBindTexture(GL_TEXTURE_2D, texture[14]);
		glColor4d(1,1,1,laser_blast_life2);
		glBegin(GL_QUADS);
		glTexCoord2d(0,1); glVertex3d(-1.5f,-2,0);
		glTexCoord2d(1,1); glVertex3d(1.5f,-2,0);
		glTexCoord2d(1,0); glVertex3d(1.5f,0,-500);
		glTexCoord2d(0,0); glVertex3d(-1.5f,0,-500);
		glEnd();

		glLoadIdentity();
		glRotated(myszY,1.f,0,0);
		glRotated(sc_obrY,0,1.f,0);
		glTranslated(xtra,-40,ztra);  
		glTranslated(fire_x,fire_y,fire_z);
		glRotated(-sc_obrY,0,1.f,0);
		glRotated(-myszY,1.f,0,0);

		glBindTexture(GL_TEXTURE_2D, texture[13]);
		glColor4d(1,1,1,laser_blast_life);
		glBegin(GL_QUADS);
		glTexCoord2d(1,1); glVertex3d( laser_blast_size, laser_blast_size,0);
		glTexCoord2d(1,0); glVertex3d( laser_blast_size,-laser_blast_size,0);
		glTexCoord2d(0,0); glVertex3d(-laser_blast_size,-laser_blast_size,0);
		glTexCoord2d(0,1); glVertex3d(-laser_blast_size, laser_blast_size,0);
		glEnd();

		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		if (laser_blast_life <= 0) 
		{
			jestLaser_blast = false;
			laser_blast_life = 1;
			laser_blast_life2 = 1;
			laser_blast_life3 = .5f;
			laser_blast_life3i = .015f;
			laser_blast_size = .1f;
		}
	}

	// jesli jest rykoszet od karabinu
	if (jestRykoszet)
	{   
		int i;
		bool jestRykoszetComplete = true;
		glDisable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		for (int j = 0; j < maxStrzalow; j++)
		{
			for (i = 8; i < maxCzastek; i++)
			{
				if (czastki[i][j].life > 0)
				{
					jestRykoszetComplete = false;

					czastki[i][j].x += czastki[i][j].xi;
					czastki[i][j].y += czastki[i][j].yi;
					czastki[i][j].z += czastki[i][j].zi;

					kolizja(czastki[i][j].x,  czastki[i][j].y,  czastki[i][j].z,
						czastki[i][j].xi, czastki[i][j].yi, czastki[i][j].zi, 0, .75f);

					czastki[i][j].yi -= .01f;  // niby grawitacja
					czastki[i][j].xi *= .99;
					czastki[i][j].yi *= .99;
					czastki[i][j].zi *= .99;
					czastki[i][j].life -= czastki[i][j].fade;

					glLoadIdentity();			//rysowanie rykoszetow
					glRotated(myszY,1.f,0,0);
					glRotated(sc_obrY,0,1.f,0);
					glTranslated(xtra,-40,ztra);  
					glTranslated(czastki[i][j].x,czastki[i][j].y,czastki[i][j].z);
					glRotated(-sc_obrY,0,1.f,0);
					glRotated(-myszY,1.f,0,0);

					czastki[i][j].triangle_rotate_x += czastki[i][j].triangle_rotate_xi;
					czastki[i][j].triangle_rotate_y += czastki[i][j].triangle_rotate_yi;
					czastki[i][j].triangle_rotate_z += czastki[i][j].triangle_rotate_zi;
					czastki[i][j].triangle_rotate_xi -= czastki[i][j].fade;
					czastki[i][j].triangle_rotate_yi -= czastki[i][j].fade;
					czastki[i][j].triangle_rotate_zi -= czastki[i][j].fade;
					glRotated(czastki[i][j].triangle_rotate_x,1,0,0);
					glRotated(czastki[i][j].triangle_rotate_y,0,1,0);
					glRotated(czastki[i][j].triangle_rotate_z,0,0,1);

					glColor4d(0,0,0,czastki[i][j].life);

					glBegin(GL_TRIANGLES);
					glVertex3d(czastki[i][j].triangle_x1*4, czastki[i][j].triangle_y1*4, czastki[i][j].triangle_z1*4);
					glVertex3d(czastki[i][j].triangle_x2*4, czastki[i][j].triangle_y2*4, czastki[i][j].triangle_z2*4);
					glVertex3d(czastki[i][j].triangle_x3*4, czastki[i][j].triangle_y3*4, czastki[i][j].triangle_z3*4);
					glEnd();
				}
			}
		}
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE);

		glBindTexture(GL_TEXTURE_2D, texture[16]);

		// dymek od karabinu
		for (int j = 0; j < maxStrzalow; j++)
		{
			for (i = 0; i < 8; i++)
			{  	  
				if (czastki[i][j].life > 0)
				{
					jestRykoszetComplete = false;

					czastki[i][j].x += czastki[i][j].xi/2;
					czastki[i][j].y += czastki[i][j].yi/2;
					czastki[i][j].z += czastki[i][j].zi/2;

					czastki[i][j].yi += .01f;

					czastki[i][j].xi *= .9f;
					czastki[i][j].yi *= .9f;
					czastki[i][j].zi *= .9f;

					czastki[i][j].life -= czastki[i][j].fade/2;

					glLoadIdentity();		//rysowanie dymku
					glRotated(myszY,1.f,0,0);
					glRotated(sc_obrY,0,1.f,0);
					glTranslated(xtra,-40,ztra);  	    
					glTranslated(czastki[i][j].x,czastki[i][j].y,czastki[i][j].z);
					glRotated(-sc_obrY,0,1.f,0);
					glRotated(-myszY,1.f,0,0);

					glColor4d(.18f,.17f,.15f,czastki[i][j].life);
					glBegin(GL_QUADS);
					glTexCoord2d(1,1); glVertex3d( czastki[i][j].size, czastki[i][j].size,0);
					glTexCoord2d(1,0); glVertex3d( czastki[i][j].size,-czastki[i][j].size,0);
					glTexCoord2d(0,0); glVertex3d(-czastki[i][j].size,-czastki[i][j].size,0);
					glTexCoord2d(0,1); glVertex3d(-czastki[i][j].size, czastki[i][j].size,0);
					glEnd();
				}  
			}
		}
		glEnable(GL_DEPTH_TEST);

		if (jestRykoszetComplete)
			jestRykoszet = false;
	}

	
	glDisable(GL_DEPTH_TEST);							// rysowanie ikon broni (HUD)

	glLoadIdentity();
	glTranslated(-712,-560.0f,-1000.0f);				//pozycja ikon

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_TEXTURE_2D); 
	glBegin(GL_QUADS);
	glColor4d(.4f,.2f,0,.5f);
	glVertex2f(-10,-10);
	glVertex2f(-10,71+10);
	glColor4d(1,1,1,0);
	glVertex2f(512+10,71+10);
	glVertex2f(512+10,-10);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE); 

	glBindTexture(GL_TEXTURE_2D,texture[11]);

	if (jestKarabin)
		glColor4d(1,.5f,0,1);
	else
		glColor4d(1,1,1,.15f);
	glBegin(GL_QUADS);
	glTexCoord2d(1,1); glVertex2f(256,71);
	glTexCoord2d(1,0); glVertex2f(256,0);
	glTexCoord2d(0,0); glVertex2f(0,0);
	glTexCoord2d(0,1); glVertex2f(0,71);
	glEnd();

	glTranslated(256,0,0);   
	glBindTexture(GL_TEXTURE_2D,texture[12]);

	if (jestLaser)
		glColor4d(1,.5f,0,1);
	else
		glColor4d(1,1,1,.15f);
	glBegin(GL_QUADS);
	glTexCoord2d(1,1); glVertex2f(256,71);
	glTexCoord2d(1,0); glVertex2f(256,0);
	glTexCoord2d(0,0); glVertex2f(0,0);
	glTexCoord2d(0,1); glVertex2f(0,71);
	glEnd();

	glEnable(GL_DEPTH_TEST);           

	// celownik  
	glDisable(GL_DEPTH_TEST);
	glLoadIdentity();
	glTranslated(0.0f,0.0f,-35.0f);   

	glBlendFunc(GL_SRC_ALPHA,GL_ONE);

	glColor4f(1,1,1,1);
	glBindTexture(GL_TEXTURE_2D,texture[15]);
	glBegin(GL_QUADS);
	glTexCoord2d(1,1); glVertex2f(1,1);
	glTexCoord2d(1,0); glVertex2f(1,-1);
	glTexCoord2d(0,0); glVertex2f(-1,-1);
	glTexCoord2d(0,1); glVertex2f(-1,1);
	glEnd();

	glEnable(GL_DEPTH_TEST);

	
	glDisable(GL_DEPTH_TEST);								//tablica wynikow
	glBindTexture(GL_TEXTURE_2D, textures[0].texID);		// wybor czcionki
	glColor4d( 1, 1, 0, 1 );	
	glPrint( 150,-230,1,"Punktacja" );					
	glPrint( 150,-250,1,"Ty: %d", punktyGracza );					
	glPrint( 150,-270,1,"Wrog: %d", punktyPrzeciwnika );					
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);



	if (zostalesZabity)
	{
		glDisable(GL_DEPTH_TEST);								//informacja o zabiciu
		glBindTexture(GL_TEXTURE_2D, textures[0].texID);		// wybor czcionki
		glColor4d( 1, 0, 0, 1 );	
		glPrint( -120,-30, 1,"ZOSTALES ZABITY!!!" );					
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_2D);
		frame_counter2++;
		if (frame_counter2 > 100 )
		{
			frame_counter2 = 0;
			zostalesZabity = FALSE;
		}
	}






	if (zliczFPSy)
	{ 
		glDisable(GL_DEPTH_TEST);
		klatki++;    
		if (klatki%10 == 0) 									// zliczanie wyswietlanych klatek na sekunde, wypisanie co 10 
		{
			czas2 = TimerGetTime()/1000;
			roznCzasu = abs(czas2-czas1);      
			czas1 = TimerGetTime()/1000;     
			if (roznCzasu != 0)
				klatekNaSek = 10/(roznCzasu);
		}             

		glBindTexture(GL_TEXTURE_2D, textures[0].texID);		// wybor czcionki
		glColor4d(0,1,0,1);	
		glPrint(-350,250,1,"Klatek/s: %4.0f", klatekNaSek);					
		glPrint(-350,230,1,"Czas: %4.2f", czas1-startCzas);					
		glPrint(-350,210,1,"x:%4.0f y:%4.0f z:%4.0f h:%4.0f roty:%4.4f ",  pozX, pozY, pozZ, obrot, sc_obrY );			//moje informacje debugujace zmienne		
		glPrint(-350,190,1,"mx:%4.2f my:%4.2f _h:%4.2f", myszX, myszY  );					
		glPrint(-350,170,1,"xp:%4.0f zp:%4.0f hp:%4.0f odlegl::%4.0f",  przecX, przecZ, przecObrot, pitagoras((pozX-przecX),(pozZ-przecZ)) );
		glPrint(-350,150,1,"wyl:%4.2f traf:%4.0f sinh:%4.2f rozn:%4.2f",  ((pozX-przecX)/pitagoras((pozX-przecX),(pozZ-przecZ))), float(przeciwnikTrafiony), sin(obrot*piprzez180) , abs( ( (pozX-przecX)/pitagoras((pozX-przecX),(pozZ-przecZ)) )   - sin(obrot*piprzez180) ));			//moje informacje debugujace zmienne		
		glPrint(-350,130,1,"frameDie:%d", frame_counter);		


		glEnable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_2D);
	}
	else
	{
		czas2 = TimerGetTime()/1000;
		roznCzasu = abs(czas2-czas1);
		while (roznCzasu < .015f) // .015 = 66 klatek na sekunde
		{
			//Sleep(1);
			czas2 = TimerGetTime()/1000;
			roznCzasu = abs(czas2-czas1);      
		}
		czas1 = TimerGetTime()/1000;     
	}
	return TRUE;					// kontynuuj renderowanie nastepnej klatki
}
void zamknijOkno()										// zamkniecie okna
{
	CleanUp();
	for (int i = 0; i < maxStrzalow; i++)
	{
		Dzwiek1[i].ReleaseBuffer();
		Dzwiek2[i].ReleaseBuffer();
	}
	SoundManager.Uninitialize();

	glDeleteLists(fonty,256);							// Delete All 256 Display Lists
	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;										// Set hWnd To NULL
	}

	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}
}
int uruchomSiecSerwer()
{
	WSADATA wsaData;

    struct addrinfo *result = NULL, hints;

    
    // Initialize Winsock
    wynik = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (wynik != 0) {
        printf("WSAStartup failed with error: %d\n", wynik);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    wynik = getaddrinfo(NULL, PORT, &hints, &result);
    if ( wynik != 0 ) {
        printf("getaddrinfo failed with error: %d\n", wynik);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    socketSluchacz = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socketSluchacz == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    wynik = bind( socketSluchacz, result->ai_addr, (int)result->ai_addrlen);
    if (wynik == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(socketSluchacz);
        WSACleanup();
        return 1;
    }
	int i = 1;
	wynik = setsockopt(socketSluchacz, IPPROTO_TCP, TCP_NODELAY, (char *)&i, sizeof(i));
    if (wynik == SOCKET_ERROR) {
        wprintf(L"setsockopt for SO_KEEPALIVE failed with error: %u\n", WSAGetLastError());
    } else
        wprintf(L"Set SO_KEEPALIVE: ON\n");


    freeaddrinfo(result);

    wynik = listen(socketSluchacz, SOMAXCONN);
    if (wynik == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(socketSluchacz);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    socketLaczeniowy = accept(socketSluchacz, NULL, NULL);
    if (socketLaczeniowy == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(socketSluchacz);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(socketSluchacz);
    return 0;

}
int uruchomSiecKlient()
{
	WSADATA wsaData;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    
     // Initialize Winsock
    wynik = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (wynik != 0) {
        printf("WSAStartup failed with error: %d\n", wynik);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    wynik = getaddrinfo(SERWER, PORT, &hints, &result);
    if ( wynik != 0 ) {
        printf("getaddrinfo failed with error: %d\n", wynik);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        socketLaczeniowy = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (socketLaczeniowy == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        wynik = connect( socketLaczeniowy, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (wynik == SOCKET_ERROR) {
            closesocket(socketLaczeniowy);
            socketLaczeniowy = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (socketLaczeniowy == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
	
	return 0;

}
LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

	//HWND	hWnd,			// Handle For This Window
	//UINT	uMsg,			// Message For This Window
	//WPARAM	wParam,		// Additional Message Information
	//LPARAM	lParam		// Additional Message Information
{	 
	if (uMsg == WM_MOUSEMOVE)
	{
		if (LOWORD(lParam) > 370) 
			SetCursorPos(10,(int)mouse_y), mouse_wrap_y--;
		else if (LOWORD(lParam) < 10) 
			SetCursorPos(370,(int)mouse_y), mouse_wrap_y++;

		if (HIWORD(lParam) > 370) 
			SetCursorPos((int)mouse_x,10), mouse_wrap_x--;
		else if (HIWORD(lParam) < 10) 
			SetCursorPos((int)mouse_x,370), mouse_wrap_x++;

		if (abs(mouse_x-LOWORD(lParam)) > 300)
		{
			if (mouse_x > 360/2)
				myszX += ((mouse_x-360)-LOWORD(lParam))/czulosc;
			else if (LOWORD(lParam) > 360/2)
				myszX += (mouse_x-(LOWORD(lParam)-360))/czulosc;
		}
		else
		{ 
			myszX += (mouse_x-LOWORD(lParam))/czulosc;
		} 

		if (myszX > 360) myszX = 0;
		else if (myszX < 0) myszX = 360;

		if (abs(mouse_y-HIWORD(lParam)) > 300)
		{ 
			if (mouse_y > 360/2)
				myszY -= ((mouse_y-360)-HIWORD(lParam))/czulosc;
			else if (LOWORD(lParam) > 360/2)
				myszY -= (mouse_y-(HIWORD(lParam)-360))/czulosc;
		}
		else
		{ 
			myszY -= (mouse_y-HIWORD(lParam))/czulosc;
		} 

		if (myszY > 85) myszY = 85;
		else if (myszY < -85) myszY = -85;

		mouse_x = LOWORD(lParam);          
		mouse_y = HIWORD(lParam);
	}			

	else if (uMsg == WM_LBUTTONDOWN)
	{     
		if (!(jestLaser && laser_blast_delay > 0))
		{
			isFire = true;
			stanModeluGracz = MODEL_SHOT;
		}

	}
	else if (uMsg == WM_ACTIVATE)							// Watch For Window Activate Message
	{
		if (!HIWORD(wParam))					// Check Minimization State
		{
			active=TRUE;						// Program Is Active
		}
		else
		{
			active=FALSE;						// Program Is No Longer Active
		}

	}

	else if (uMsg == WM_SYSCOMMAND)							// Intercept System Commands
	{
		switch (wParam)							// Check System Calls
		{
		case SC_SCREENSAVE:					// Screensaver Trying To Start?
		case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?

			return 0;							// Prevent From Happening
		}
	}

	else if (uMsg == WM_CLOSE)								// Did We Receive A Close Message?
	{
		PostQuitMessage(0);						// Send A Quit Message
	}

	else if (uMsg == WM_KEYDOWN)							// Is A Key Being Held Down?
	{
		klawisze[wParam] = TRUE;					// If So, Mark It As TRUE
	}

	else if (uMsg == WM_KEYUP)								// Has A Key Been Released?
	{
		klawisze[wParam] = FALSE;					// If So, Mark It As FALSE
	}

	else if (uMsg == WM_SIZE)								// Resize The OpenGL Window
	{
		przeskalujScene(LOWORD(lParam),HIWORD(lParam));  // LoWord=Width, HiWord=Height
	}        		


	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
	*	title			- Title To Appear At The Top Of The Window				*
	*	width			- Width Of The GL Window Or Fullscreen Mode				*
	*	height			- Height Of The GL Window Or Fullscreen Mode			*
	*	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
	*	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	HINSTANCE	hInstance;				// Holds The Instance Of The Application
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style


	fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	//if (MessageBoxEx(NULL,"Uruchom gre jako:\n\nSerwer = TAK\nKlient = NIE","Pytanie",MB_YESNO|MB_ICONQUESTION,2)==IDYES)
	//{
	//	jestSerwerem=TRUE;
	//}
	//else jestSerwerem=FALSE;


	ChangeDisplaySettings(NULL,0);								// If So Switch Back To The Desktop

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}


	//if (MessageBoxEx(NULL,"Rozdzielczosc:	TAK=1366x768	NIE=1280x1024","Pytanie",MB_YESNO|MB_ICONQUESTION,2)==IDYES)
	//{
	//	width = 1366;
	//	height = 768;
	//}
	//else
	//{
	//	width = 1280;
	//	height = 1024;
	//};



	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;


		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		width = 1920;
		height = 1080;
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height

		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			width = 1366;
			height = 768;
			dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
			dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height

			if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
			{

				// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
				if (MessageBox(NULL,"Tryb pelnoekranowy nie jest dostepny. Uruchomic w oknie?","GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
				{
					fullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
				}
				else
				{
					// Pop Up A Message Box Letting User Know The Program Is Closing.
					MessageBox(NULL,"Program zamkniety.","ERROR",MB_OK|MB_ICONSTOP);
					return FALSE;									// Return FALSE
				}
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;	// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;						// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;	// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}

	// Create The Window
	if (!(hWnd=CreateWindowEx(	dwExStyle,			// Extended Style For The Window
		"OpenGL",			// Class Name
		title,				// Window Title
		dwStyle,			// Window Style
		0, 0,				// Window Position
		width, height,		// Selected Width And Height
		NULL,				// No Parent Window
		NULL,				// No Menu
		hInstance,			// Instance
		NULL)))				// Dont Pass Anything To WM_CREATE
	{
		zamknijOkno();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
	{
		zamknijOkno();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		zamknijOkno();								// Reset The Display
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		zamknijOkno();								// Reset The Display
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		zamknijOkno();								// Reset The Display
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
	{
		zamknijOkno();								// Reset The Display
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	przeskalujScene(width, height);					// Set Up Our Perspective GL Screen

	if (!inicjujGL())								// Initialize Our Newly Created GL Window
	{
		zamknijOkno();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)			// Window Show State

	//HINSTANCE	hInstance,			// Instance
	//HINSTANCE	hPrevInstance,		// Previous Instance
	//LPSTR		lpCmdLine,			// Command Line Parameters
	//int		nCmdShow		// Window Show State

{

	MSG		msg;									// Windows Message Structure

	if (!CreateGLWindow("Strzelanka 3D - Kamil Karecki - WSTI",SZER,WYS,32,fullscreen))
	{
		return 0;
	}

	if (jestSerwerem) uruchomSiecSerwer();
	else uruchomSiecKlient();

	srand (time(0)*100);
	pozX = rand()%maxKomorek*2-maxKomorek;	// poz x poczatkowa
	//srand (time(0));
	pozZ = rand()%maxKomorek*2-maxKomorek;	// poz z poczatkowa
	
	while(!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
			{
				done=TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From renderujScene()
			if ((active && !renderujScene()) || klawisze[VK_ESCAPE])	// Active?  Was There A Quit Received?
			{
				done=TRUE;							// ESC or renderujScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)

				
				is_Fired--;
				if (is_Fired < 0) is_Fired = 0;


				if (klawisze[VK_UP])  // Move forwards
				{
					XP -= (GLdouble)sin(obrot*piprzez180) * 10.0f;	
					ZP -= (GLdouble)cos(obrot*piprzez180) * 10.0f;
					stanModeluGracz = MODEL_RUN;
				}
				else if (klawisze['W'])  // Move forwards
				{
					XP -= (GLdouble)sin(obrot*piprzez180) * 10.0f;	
					ZP -= (GLdouble)cos(obrot*piprzez180) * 10.0f;
					stanModeluGracz = MODEL_RUN;
				}

				if (klawisze[VK_DOWN]) // Move backwards
				{
					XP += (GLdouble)sin(obrot*piprzez180) * 10.0f;	
					ZP += (GLdouble)cos(obrot*piprzez180) * 10.0f;
					stanModeluGracz = MODEL_RUN;
				}	               
				else if (klawisze['S']) // Move backwards
				{
					XP += (GLdouble)sin(obrot*piprzez180) * 10.0f;	
					ZP += (GLdouble)cos(obrot*piprzez180) * 10.0f;
					stanModeluGracz = MODEL_RUN;
				}	

				if (klawisze['A'])  // strafe left
				{
					XP += (GLdouble)sin((obrot-90)*piprzez180) * 10.0f;	
					ZP += (GLdouble)cos((obrot-90)*piprzez180) * 10.0f;
					stanModeluGracz = MODEL_RUN;
				}

				if (klawisze['D']) // strafe right
				{
					XP += (GLdouble)sin((obrot+90)*piprzez180) * 10.0f;	
					ZP += (GLdouble)cos((obrot+90)*piprzez180) * 10.0f;
					stanModeluGracz = MODEL_RUN;
				}	               

				if (klawisze['L']) // na dol
				{
					pozY--;
					stanModeluGracz = MODEL_JUMP;
				}	               

				if (klawisze['K']) // do gory
				{
					pozY++;
					stanModeluGracz = MODEL_IDLE;
				}	               

				if (klawisze['-'])
				{
					czulosc-=0.1;
				}	               

				if (klawisze['='])
				{
					czulosc+=0.1;
				}	               

				if (klawisze['1'])  // wybor karabinu
				{
					jestKarabin = true;	
					jestLaser = false;
				}

				if (klawisze['2'])  // wybor lasera
				{
					jestKarabin = false;
					jestLaser = true;
				}

				if (klawisze[VK_SPACE])
				{
					stanModeluGracz = MODEL_SHOT;
					if (jestKarabin && is_Fired == 0)
					{
						is_Fired = 7;  // 1 strzal na 7 klatek
						isFire = true;
					}
					else if ((jestLaser && laser_blast_delay <= 0)) 
					{
						isFire = true;
					}
				}

				if (klawisze[VK_LEFT]) // Turn left
				{
					zprot += .5f;
				}
				else if (klawisze[VK_RIGHT]) // Turn right
				{
					zprot -= .5f;
				}			

				if (klawisze['J'] && !fp) // wlacznik licznika FPS i innych informacji
				{
					zliczFPSy = !zliczFPSy;
					fp = true;
				}
				else if (!klawisze['J'])
				{
					fp = false;
				}
			}
		}
	}

	shutdown(socketLaczeniowy, SD_SEND);
	closesocket(socketLaczeniowy);
    WSACleanup();
	zamknijOkno();
	return (msg.wParam);						// wyjscie z programu
}

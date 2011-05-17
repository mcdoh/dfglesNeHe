#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <directfb.h>
#include <directfbgl.h>

#include <GLES/gl.h>


// the super interface
IDirectFB *dfb;

// the primary surface (surface of primary layer)
IDirectFBSurface *primary;

// the GL context
IDirectFBGL *primary_gl;

// our font
IDirectFBFont *font;

// event buffer
IDirectFBEventBuffer *events;

// macro for a safe call to DirectFB functions
#define DFBCHECK(x...)                                         \
{                                                              \
	err = x;                                                   \
	if (err != DFB_OK) {                                       \
		fprintf(stderr, "%s <%d>:\n\t", __FILE__, __LINE__);   \
		DirectFBErrorFatal(#x, err);                           \
	}                                                          \
}

static int screen_width, screen_height;

static unsigned long T0 = 0;
static GLint Frames = 0;
static GLfloat fps = 0;

static inline unsigned long get_millis()
{
	struct timeval tv;

	gettimeofday (&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}


static GLfloat inc_x = 0.0, inc_y = 0.0, inc_z = 0.0;

GLfloat xrot;
GLfloat yrot;
GLfloat zrot;

GLuint texture;

typedef struct
{
	int width;
	int height;
	unsigned char *data;
} textureImage;

// simple loader for 24bit bitmaps (data is in rgb-format)
bool loadBMP(textureImage* texti)
{
	int i;
	FILE* file;
	unsigned short int bfType;
	long int bfOffBits;
	short int biPlanes;
	short int biBitCount;
	long int biSizeImage;
	unsigned char temp;
	
	file = fopen("NeHe.bmp","r");

	if (!file)
	{
		printf("no file!\n");
		return false;
	}

	if (!fread((char*)&bfType, sizeof(short int), 1, file))
	{
		printf("couldn't read from file!\n");
		return false;
	}

	// check if file is a bitmap
	if (bfType != 19778)
	{
		printf("not a bitmap!\n");
		return false;
	}
	else
		printf("bfType: %d\n",bfType);

	// get the position of the actual bitmap data
	fseek(file,10,SEEK_SET);
	if (!fread((char*)&bfOffBits, sizeof(long int), 1, file))
	{
		printf("couldn't read start of data!\n");
		return false;
	}
	else
		printf("bfOffBits: %i\n",bfOffBits);

	// get the width of the bitmap
	fseek(file,18,SEEK_SET);
	if (!fread((char*)&texti->width, sizeof(int), 1, file))
	{
		printf("couldn't read width of data!\n");
		return false;
	}
	else
		printf("width: %d\n",texti->width);

	// get the height of the bitmap
	fseek(file,22,SEEK_SET);
	if (!fread((char*)&texti->height, sizeof(int), 1, file))
	{
		printf("couldn't read height of data!\n");
		return false;
	}
	else
		printf("height: %d\n",texti->height);

	// get the number of planes (must be set to 1)
	fseek(file,26,SEEK_SET);
	if (!fread((char*)&biPlanes, sizeof(short int), 1, file))
	{
		printf("couldn't read number of planes!\n");
		return false;
	}
	else
		printf("biPlanes: %d\n",biPlanes);

	if (biPlanes != 1)
	{
		printf("oh no! planes is not equal to 1!\n");
		return false;
	}

	// get the number of bits per pixel
	fseek(file,28,SEEK_SET);
	if (!fread((char*)&biBitCount, sizeof(short int), 1, file))
	{
		printf("couldn't read pits per pixel!\n");
		return false;
	}
	else
		printf("biBitCount: %d\n",biBitCount);

	if (biBitCount != 24)
	{
		printf("oh no! bitcount not equal to 24!\n");
		return false;
	}
	
	// calculate the size of the image in bytes
	biSizeImage = texti->width * texti->height * 3;
	texti->data = malloc(biSizeImage);

	// seek to the actual data
	fseek(file,bfOffBits,SEEK_SET);
	if (!fread((char*)texti->data, biSizeImage, 1, file))
	{
		printf("couldn't read the actual image data!\n");
		return false;
	}

	// swap red and blue (bgr -> rgb)
	for (i = 0; i < biSizeImage; i += 3)
	{
//		printf("(%d,%d,%d)\n",texti->data[i],texti->data[i+1],texti->data[i+2]);
		temp = texti->data[i];
		texti->data[i] = texti->data[i + 2];
		texti->data[i + 2] = temp;
	}

	fclose(file);

	printf("well okay, then, everything seems fine!\n");
	return true;
}

void makeTexture(GLuint* texture)
{
	textureImage* texti = malloc(sizeof(textureImage));
	
	if (loadBMP(texti))
	{
		glGenTextures(1, texture);   // create the texture
		glBindTexture(GL_TEXTURE_2D, *texture);

		// generate the texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texti->width, texti->height, 0, GL_RGB, GL_UNSIGNED_BYTE, texti->data);

		// enable linear filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		free(texti->data);
	}
	else
		*texture = 0;

	free(texti);
}

static void draw(void)
{
    GLfloat texcoords[4][2];
    GLfloat vertices[4][3];
    GLubyte indices[4]={0, 1, 3, 2}; // QUAD to TRIANGLE_STRIP conversion

	// Clear The Screen And The Depth Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Move Into The Screen 5 Units
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -5.0f);

	glRotatef(xrot, 1.0f, 0.0f, 0.0f); // Rotate On The X Axis
	glRotatef(yrot, 0.0f, 1.0f, 0.0f); // Rotate On The Y Axis
	glRotatef(zrot, 0.0f, 0.0f, 1.0f); // Rotate On The Z Axis

	// Select Our Texture
	glBindTexture(GL_TEXTURE_2D, texture);

	// Set pointers to vertices and texcoords
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

	// Enable vertices and texcoords arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Front Face
	texcoords[0][0]=0.0f; texcoords[0][1]=0.0f;
	vertices[0][0]=-1.0f; vertices[0][1]=-1.0f; vertices[0][2]=1.0f;
	texcoords[1][0]=1.0f; texcoords[1][1]=0.0f;
	vertices[1][0]=1.0f;  vertices[1][1]=-1.0f; vertices[1][2]=1.0f;
	texcoords[2][0]=1.0f; texcoords[2][1]=1.0f;
	vertices[2][0]=1.0f;  vertices[2][1]=1.0f; vertices[2][2]=1.0f;
	texcoords[3][0]=0.0f; texcoords[3][1]=1.0f;
	vertices[3][0]=-1.0f; vertices[3][1]=1.0f; vertices[3][2]=1.0f;

	// Draw one textured plane using two stripped triangles
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, indices);

	// Back Face
	// Normal Pointing Away From Viewer
	texcoords[0][0]=1.0f; texcoords[0][1]=0.0f;
	vertices[0][0]=-1.0f; vertices[0][1]=-1.0f; vertices[0][2]=-1.0f;
	texcoords[1][0]=1.0f; texcoords[1][1]=1.0f;
	vertices[1][0]=-1.0f; vertices[1][1]=1.0f; vertices[1][2]=-1.0f;
	texcoords[2][0]=0.0f; texcoords[2][1]=1.0f;
	vertices[2][0]=1.0f;  vertices[2][1]=1.0f; vertices[2][2]=-1.0f;
	texcoords[3][0]=0.0f; texcoords[3][1]=0.0f;
	vertices[3][0]=1.0f; vertices[3][1]=-1.0f; vertices[3][2]=-1.0f;

	// Draw one textured plane using two stripped triangles
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, indices);

	// Top Face
	texcoords[0][0]=1.0f; texcoords[0][1]=1.0f;
	vertices[0][0]=-1.0f; vertices[0][1]=1.0f; vertices[0][2]=-1.0f;
	texcoords[1][0]=0.0f; texcoords[1][1]=1.0f;
	vertices[1][0]=-1.0f; vertices[1][1]=1.0f; vertices[1][2]=1.0f;
	texcoords[2][0]=0.0f; texcoords[2][1]=0.0f;
	vertices[2][0]=1.0f;  vertices[2][1]=1.0f; vertices[2][2]=1.0f;
	texcoords[3][0]=1.0f; texcoords[3][1]=0.0f;
	vertices[3][0]=1.0f;  vertices[3][1]=1.0f; vertices[3][2]=-1.0f;

	// Draw one textured plane using two stripped triangles
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, indices);

	// Bottom Face
	texcoords[0][0]=0.0f; texcoords[0][1]=1.0f;
	vertices[0][0]=-1.0f; vertices[0][1]=-1.0f; vertices[0][2]=-1.0f;
	texcoords[1][0]=0.0f; texcoords[1][1]=0.0f;
	vertices[1][0]=1.0f;  vertices[1][1]=-1.0f; vertices[1][2]=-1.0f;
	texcoords[2][0]=1.0f; texcoords[2][1]=0.0f;
	vertices[2][0]=1.0f;  vertices[2][1]=-1.0f; vertices[2][2]=1.0f;
	texcoords[3][0]=1.0f; texcoords[3][1]=1.0f;
	vertices[3][0]=-1.0f; vertices[3][1]=-1.0f; vertices[3][2]=1.0f;

	// Draw one textured plane using two stripped triangles
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, indices);

	// Right face
	texcoords[0][0]=0.0f; texcoords[0][1]=0.0f;
	vertices[0][0]=1.0f;  vertices[0][1]=-1.0f; vertices[0][2]=-1.0f;
	texcoords[1][0]=1.0f; texcoords[1][1]=0.0f;
	vertices[1][0]=1.0f;  vertices[1][1]=1.0f; vertices[1][2]=-1.0f;
	texcoords[2][0]=1.0f; texcoords[2][1]=1.0f;
	vertices[2][0]=1.0f;  vertices[2][1]=1.0f; vertices[2][2]=1.0f;
	texcoords[3][0]=0.0f; texcoords[3][1]=1.0f;
	vertices[3][0]=1.0f;  vertices[3][1]=-1.0f; vertices[3][2]=1.0f;

	// Draw one textured plane using two stripped triangles
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, indices);

	// Left Face
	texcoords[0][0]=1.0f; texcoords[0][1]=0.0f;
	vertices[0][0]=-1.0f; vertices[0][1]=-1.0f; vertices[0][2]=-1.0f;
	texcoords[1][0]=1.0f; texcoords[1][1]=1.0f;
	vertices[1][0]=-1.0f; vertices[1][1]=-1.0f; vertices[1][2]=1.0f;
	texcoords[2][0]=0.0f; texcoords[2][1]=1.0f;
	vertices[2][0]=-1.0f; vertices[2][1]=1.0f; vertices[2][2]=1.0f;
	texcoords[3][0]=0.0f; texcoords[3][1]=0.0f;
	vertices[3][0]=-1.0f; vertices[3][1]=1.0f; vertices[3][2]=-1.0f;

	// Draw one textured plane using two stripped triangles
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, indices);

	// Disable texcoords and vertices arrays
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	// Flush all drawings
	glFinish();

	xrot += 0.03f;
	yrot += 0.02f;
	zrot += 0.04f;
}

static void reshape(int width, int height)
{
    // Height / width ration
    GLfloat ratio;

    // Protect against a divide by zero
    if (width == 0)
        width = 1;

    ratio = (GLfloat)height / (GLfloat)width;

    // Setup our viewport.
    glViewport(0, 0, (GLint)width, (GLint)height);

    // change to the projection matrix and set our viewing volume.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set our perspective
	glFrustumf(-1.0, 1.0, -ratio, ratio, 1.0, 100.0);

    // Make sure we're chaning the model view and not the projection
    glMatrixMode(GL_MODELVIEW);

    // Reset The View
    glLoadIdentity();
}

static void init(int argc, char *argv[])
{
	makeTexture(&texture);

	// enable texture mapping
	glEnable(GL_TEXTURE_2D);

	// Enable smooth shading
	glShadeModel(GL_SMOOTH);

	// Set the background black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Depth buffer setup
	glClearDepthf(1.0f);

	// Enables Depth Testing
	glEnable(GL_DEPTH_TEST);

	// only show the front face to fix depth bugs
	glEnable(GL_CULL_FACE);

	// The Type Of Depth Test To Do
	glDepthFunc(GL_LEQUAL);

	// Really Nice Perspective Calculations
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

int main(int argc, char *argv[])
{
	int quit = 0;
	DFBResult err;
	DFBSurfaceDescription dsc;

	DFBCHECK(DirectFBInit(&argc, &argv));

	// create the super interface
	DFBCHECK(DirectFBCreate(&dfb));

	// create an event buffer for all devices with these caps
	DFBCHECK(dfb->CreateInputEventBuffer(dfb, DICAPS_KEYS | DICAPS_AXES, DFB_FALSE, &events));

	// set our cooperative level to DFSCL_FULLSCREEN for exclusive access to the primary layer
	dfb->SetCooperativeLevel(dfb, DFSCL_FULLSCREEN);

	// get the primary surface, i.e. the surface of the primary layer we have exclusive access to
	dsc.flags = DSDESC_CAPS;
	dsc.caps  = DSCAPS_PRIMARY | DSCAPS_DOUBLE | DSCAPS_OPENGL_HINT;

	DFBCHECK(dfb->CreateSurface(dfb, &dsc, &primary));

	// get the size of the surface and fill it
	DFBCHECK(primary->GetSize(primary, &screen_width, &screen_height));
	DFBCHECK(primary->FillRectangle(primary, 0, 0, screen_width, screen_height));
	primary->Flip(primary, NULL, 0);

	// create the default font and set it
	DFBCHECK(dfb->CreateFont(dfb, NULL, NULL, &font));
	DFBCHECK(primary->SetFont(primary, font));

	// get the GL context
	DFBCHECK(primary->GetGL(primary, &primary_gl));

	DFBCHECK(primary_gl->Lock(primary_gl));

	init(argc, argv);
	reshape(screen_width, screen_height);

	DFBCHECK(primary_gl->Unlock(primary_gl));

	T0 = get_millis();

	while (!quit)
	{
		DFBInputEvent evt;
		unsigned long t;

		DFBCHECK(primary_gl->Lock(primary_gl));

		draw();

		DFBCHECK(primary_gl->Unlock(primary_gl));

		if (fps)
		{
			char buf[64];

			snprintf(buf, 64, "%4.1f FPS\n", fps);

			primary->SetColor(primary, 0xff, 0, 0, 0xff);
			primary->DrawString(primary, buf, -1, screen_width - 5, 5, DSTF_TOPRIGHT);
		}

		primary->Flip(primary, NULL, 0);
		Frames++;


		t = get_millis();
		if (t - T0 >= 2000)
		{
			GLfloat seconds = (t - T0) / 1000.0;

			fps = Frames / seconds;

			T0 = t;
			Frames = 0;
		}


		while (events->GetEvent(events, DFB_EVENT(&evt)) == DFB_OK)
		{
			switch (evt.type)
			{
				case DIET_KEYPRESS:
					switch (evt.key_symbol)
					{
						case DIKS_ESCAPE:
							quit = 1;
							break;
						case DIKS_CURSOR_UP:
 							inc_x = 0.25;
							break;
						case DIKS_CURSOR_DOWN:
							inc_x = -0.25;
							break;
						case DIKS_CURSOR_LEFT:
							inc_y = -0.25;
							break;
						case DIKS_CURSOR_RIGHT:
							inc_y = 0.25;
							break;
						case DIKS_PAGE_UP:
							inc_z = 0.25;
							break;
						case DIKS_PAGE_DOWN:
							inc_z = -0.25;
							break;
						default:
							;
					}
					break;
				case DIET_KEYRELEASE:
					switch (evt.key_symbol)
					{
						case DIKS_CURSOR_UP:
							inc_x = 0;
							break;
						case DIKS_CURSOR_DOWN:
							inc_x = 0;
							break;
						case DIKS_CURSOR_LEFT:
							inc_y = 0;
							break;
						case DIKS_CURSOR_RIGHT:
							inc_y = 0;
							break;
						case DIKS_PAGE_UP:
							inc_z = 0;
							break;
						case DIKS_PAGE_DOWN:
							inc_z = 0;
							break;
						default:
							;
					}
					break;
				case DIET_AXISMOTION:
					if (evt.flags & DIEF_AXISREL)
					{
						switch (evt.axis)
						{
							case DIAI_X:
								yrot += evt.axisrel / 2.0;
								break;
							case DIAI_Y:
								xrot += evt.axisrel / 2.0;
								break;
							case DIAI_Z:
								zrot += evt.axisrel / 2.0;
								break;
							default:
								;
						}
					}
					break;
				default:
					;
			}
		}

		xrot += inc_x;
		yrot += inc_y;
		zrot += inc_z;
	}

	// release our interfaces to shutdown DirectFB
	primary_gl->Release(primary_gl);
	primary->Release(primary);
	font->Release(font);
	events->Release(events);
	dfb->Release(dfb);

	return 0;
}


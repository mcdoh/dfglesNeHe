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


static GLfloat view_x = 0.0, view_y = 0.0, view_z = 0.0;
static GLfloat inc_x = 0.0, inc_y = 0.0, inc_z = 0.0;

static void draw(void)
{
	// rotational variables for the triangle and square;
    static GLfloat rtri, rquad;

	GLfloat colors[24][4];
	GLfloat vertices[24][3];

	// Clear The Screen And The Depth Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable COLOR and VERTEX arrays
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	// Setup pointers to COLOR and VERTEX arrays
	glColorPointer(4, GL_FLOAT, 0, colors);
	glVertexPointer(3, GL_FLOAT, 0, vertices);

	// Move Left 1.5 Units And Into The Screen 6.0
	glLoadIdentity();
	glTranslatef((view_x - 1.5f), (view_y + 0.0f), (view_z - 5.0f));

	// rotate the triangle about the Y axis
    glRotatef(rtri, 0.0f, 1.0f, 0.0f);

	// red
	colors[0][0] = 1.0f; colors[0][1] = 0.0f; colors[0][2] = 0.0f; colors[0][3] = 1.0f;
	// Top Of Triangle
	vertices[0][0] = 0.0f; vertices[0][1] = 1.0f; vertices[0][2] = 0.0f;
	// green
	colors[1][0] = 0.0f; colors[1][1] = 1.0f; colors[1][2] = 0.0f; colors[1][3] = 1.0f;
	// Left Of Triangle
	vertices[1][0] = -1.0f; vertices[1][1] = -1.0f; vertices[1][2] = 1.0f;
	// blue
	colors[2][0] = 0.0f; colors[2][1] = 0.0f; colors[2][2] = 1.0f; colors[2][3] = 1.0f;
	// Right Of Triangle
	vertices[2][0] = 1.0f; vertices[2][1] = -1.0f; vertices[2][2] = 1.0f;

	// Red
	colors[3][0] = 1.0f; colors[3][1] = 0.0f; colors[3][2] = 0.0f; colors[3][3] = 1.0f;
	// Top Of Triangle (Right)
	vertices[3][0] = 0.0f; vertices[3][1] = 1.0f; vertices[3][2] = 0.0f;
	// Blue
	colors[4][0] = 0.0f; colors[4][1] = 0.0f; colors[4][2] = 1.0f; colors[4][3] = 1.0f;
	// Left Of Triangle (Right)
	vertices[4][0] = 1.0f; vertices[4][1] = -1.0f; vertices[4][2] = 1.0f;
	// Green
	colors[5][0] = 0.0f; colors[5][1] = 1.0f; colors[5][2] = 0.0f; colors[5][3] = 1.0f;
	// Right Of Triangle (Right)
	vertices[5][0] = 1.0f; vertices[5][1] = -1.0f; vertices[5][2] = -1.0f;

	// Red
	colors[6][0] = 1.0f; colors[6][1] = 0.0f; colors[6][2] = 0.0f; colors[6][3] = 1.0f;
	// Top Of Triangle (Back)
	vertices[6][0] = 0.0f; vertices[6][1] = 1.0f; vertices[6][2] = 0.0f;
	// Green
	colors[7][0] = 0.0f; colors[7][1] = 1.0f; colors[7][2] = 0.0f; colors[7][3] = 1.0f;
	// Left Of Triangle (Back)
	vertices[7][0] = 1.0f; vertices[7][1] = -1.0f; vertices[7][2] = -1.0f;
	// Blue
	colors[8][0] = 0.0f; colors[8][1] = 0.0f; colors[8][2] = 1.0f; colors[8][3] = 1.0f;
	// Right Of Triangle (Back)
	vertices[8][0] = -1.0f; vertices[8][1] = -1.0f; vertices[8][2] = -1.0f;

	// Red
	colors[9][0] = 1.0f; colors[9][1] = 0.0f; colors[9][2] = 0.0f; colors[9][3] = 1.0f;
	// Top Of Triangle (Left)
	vertices[9][0] = 0.0f; vertices[9][1] = 1.0f; vertices[9][2] = 0.0f;
	// Blue
	colors[10][0] = 0.0f; colors[10][1] = 0.0f; colors[10][2] = 1.0f; colors[10][3] = 1.0f;
	// Left Of Triangle (Left)
	vertices[10][0] = -1.0f; vertices[10][1] = -1.0f; vertices[10][2] = -1.0f;
	// Green
	colors[11][0] = 0.0f; colors[11][1] = 1.0f; colors[11][2] = 0.0f; colors[11][3] = 1.0f;
	// Right Of Triangle (Left)
	vertices[11][0] = -1.0f; vertices[11][1] = -1.0f; vertices[11][2] = 1.0f;

	// Drawing Using Triangles, draw triangles using 12 vertices
	glDrawArrays(GL_TRIANGLES, 0, 12);

	// Move Right 3 Units
	glLoadIdentity();
	glTranslatef((view_x + 1.5f), (view_y + 0.0f), (view_z - 5.0f));

	// rotate the square about the X axis
    glRotatef(rquad, 1.0f, 0.0f, 0.0f);

	// Draw a cube //
	
	// Set The Color To Green
	colors[0][0] = colors[1][0] = colors[2][0] = colors[3][0] = 0.0f;
	colors[0][1] = colors[1][1] = colors[2][1] = colors[3][1] = 1.0f;
	colors[0][2] = colors[1][2] = colors[2][2] = colors[3][2] = 0.0f;
	colors[0][3] = colors[1][3] = colors[2][3] = colors[3][3] = 1.0f;
	// Top Right Of The Quad (Top)
	vertices[0][0] = 1.0f;  vertices[0][1] = 1.0f; vertices[0][2] = -1.0f;
	// Top Left Of The Quad (Top)
	vertices[1][0] = -1.0f; vertices[1][1] = 1.0f; vertices[1][2] = -1.0f;
	// Bottom Right Of The Quad (Top)
	vertices[2][0] = 1.0f;  vertices[2][1] = 1.0f; vertices[2][2] = 1.0f;
	// Bottom Left Of The Quad (Top)
	vertices[3][0] = -1.0f; vertices[3][1] = 1.0f; vertices[3][2] = 1.0f;

	// Set The Color To Orange
	colors[4][0] = colors[5][0] = colors[6][0] = colors[7][0] = 1.0f;
	colors[4][1] = colors[5][1] = colors[6][1] = colors[7][1] = 0.5f;
	colors[4][2] = colors[5][2] = colors[6][2] = colors[7][2] = 0.0f;
	colors[4][3] = colors[5][3] = colors[6][3] = colors[7][3] = 1.0f;
	// Top Right Of The Quad (Bottom)
	vertices[4][0] = 1.0f;  vertices[4][1] = -1.0f; vertices[4][2] = 1.0f;
	// Top Left Of The Quad (Bottom)
	vertices[5][0] = -1.0f; vertices[5][1] = -1.0f; vertices[5][2] = 1.0f;
	// Bottom Right Of The Quad (Bottom)
	vertices[6][0] = 1.0f;  vertices[6][1] = -1.0f; vertices[6][2] = -1.0f;
	// Bottom Left Of The Quad (Bottom)
	vertices[7][0] = -1.0f; vertices[7][1] = -1.0f; vertices[7][2] = -1.0f;

	// Set The Color To Red
	colors[8][0] = colors[9][0] = colors[10][0] = colors[11][0] = 1.0f;
	colors[8][1] = colors[9][1] = colors[10][1] = colors[11][1] = 0.0f;
	colors[8][2] = colors[9][2] = colors[10][2] = colors[11][2] = 0.0f;
	colors[8][3] = colors[9][3] = colors[10][3] = colors[11][3] = 1.0f;
	// Top Right Of The Quad (Front)
	vertices[8][0] = 1.0f;   vertices[8][1] = 1.0f;   vertices[8][2] = 1.0f;
	// Top Left Of The Quad (Front)
	vertices[9][0] = -1.0f;  vertices[9][1] = 1.0f;   vertices[9][2] = 1.0f;
	// Bottom Right Of The Quad (Front)
	vertices[10][0] = 1.0f;  vertices[10][1] = -1.0f; vertices[10][2] = 1.0f;
	// Bottom Left Of The Quad (Front)
	vertices[11][0] = -1.0f; vertices[11][1] = -1.0f; vertices[11][2] = 1.0f;

	// Set The Color To Yellow
	colors[12][0] = colors[13][0] = colors[14][0] = colors[15][0] = 1.0f;
	colors[12][1] = colors[13][1] = colors[14][1] = colors[15][1] = 1.0f;
	colors[12][2] = colors[13][2] = colors[14][2] = colors[15][2] = 0.0f;
	colors[12][3] = colors[13][3] = colors[14][3] = colors[15][3] = 1.0f;
	// Top Right Of The Quad (Back)
	vertices[12][0] = 1.0f;  vertices[12][1] = -1.0f; vertices[12][2] = -1.0f;
	// Top Left Of The Quad (Back)
	vertices[13][0] = -1.0f; vertices[13][1] = -1.0f; vertices[13][2] = -1.0f;
	// Bottom Right Of The Quad (Back)
	vertices[14][0] = 1.0f;  vertices[14][1] = 1.0f;  vertices[14][2] = -1.0f;
	// Bottom Left Of The Quad (Back)
	vertices[15][0] = -1.0f; vertices[15][1] = 1.0f;  vertices[15][2] = -1.0f;

	// Set The Color To Blue
	colors[16][0] = colors[17][0] = colors[18][0] = colors[19][0] = 0.0f;
	colors[16][1] = colors[17][1] = colors[18][1] = colors[19][1] = 0.0f;
	colors[16][2] = colors[17][2] = colors[18][2] = colors[19][2] = 1.0f;
	colors[16][3] = colors[17][3] = colors[18][3] = colors[19][3] = 1.0f;
	// Top Right Of The Quad (Left)
	vertices[16][0] = -1.0f; vertices[16][1] = 1.0f;  vertices[16][2] = 1.0f;
	// Top Left Of The Quad (Left)
	vertices[17][0] = -1.0f; vertices[17][1] = 1.0f;  vertices[17][2] = -1.0f;
	// Bottom Right Of The Quad (Left)
	vertices[18][0] = -1.0f; vertices[18][1] = -1.0f; vertices[18][2] = 1.0f;
	// Bottom Left Of The Quad (Left)
	vertices[19][0] = -1.0f; vertices[19][1] = -1.0f; vertices[19][2] = -1.0f;

	// Set The Color To Violet
	colors[20][0] = colors[21][0] = colors[22][0] = colors[23][0] = 1.0f;
	colors[20][1] = colors[21][1] = colors[22][1] = colors[23][1] = 0.0f;
	colors[20][2] = colors[21][2] = colors[22][2] = colors[23][2] = 1.0f;
	colors[20][3] = colors[21][3] = colors[22][3] = colors[23][3] = 1.0f;
	// Top Right Of The Quad (Right)
	vertices[20][0] = 1.0f; vertices[20][1] = 1.0f;  vertices[20][2] = -1.0f;
	// Top Left Of The Quad (Right)
	vertices[21][0] = 1.0f; vertices[21][1] = 1.0f;  vertices[21][2] = 1.0f;
	// Bottom Right Of The Quad (Right)
	vertices[22][0] = 1.0f; vertices[22][1] = -1.0f; vertices[22][2] = -1.0f;
	// Bottom Left Of The Quad (Right)
	vertices[23][0] = 1.0f; vertices[23][1] = -1.0f; vertices[23][2] = 1.0f;

	// Drawing using triangle strips, draw triangle strips using 4 vertices
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);

	// Disable color and vertex arrays
    glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	// Flush all drawings
	glFinish();

	// increase the rotation of the triangle
    rtri+=0.2f;
	// decrease the rotation of the square
    rquad-=0.15f;
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
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

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
 							inc_y = 0.1;
							break;
						case DIKS_CURSOR_DOWN:
							inc_y = -0.1;
							break;
						case DIKS_CURSOR_LEFT:
							inc_x = -0.1;
							break;
						case DIKS_CURSOR_RIGHT:
							inc_x = 0.1;
							break;
						case DIKS_PAGE_UP:
							inc_z = 0.01;
							break;
						case DIKS_PAGE_DOWN:
							inc_z = -0.01;
							break;
						default:
							;
					}
					break;
				case DIET_KEYRELEASE:
					switch (evt.key_symbol)
					{
						case DIKS_CURSOR_UP:
							inc_y = 0;
							break;
						case DIKS_CURSOR_DOWN:
							inc_y = 0;
							break;
						case DIKS_CURSOR_LEFT:
							inc_x = 0;
							break;
						case DIKS_CURSOR_RIGHT:
							inc_x = 0;
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
								view_x += evt.axisrel / 2.0;
								break;
							case DIAI_Y:
								view_y -= evt.axisrel / 2.0;
								break;
							case DIAI_Z:
								view_z += evt.axisrel / 2.0;
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

		view_x += inc_x;
		view_y += inc_y;
		view_z += inc_z;
	}

	// release our interfaces to shutdown DirectFB
	primary_gl->Release(primary_gl);
	primary->Release(primary);
	font->Release(font);
	events->Release(events);
	dfb->Release(dfb);

	return 0;
}


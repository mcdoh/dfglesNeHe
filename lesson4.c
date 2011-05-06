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

	GLfloat colors[4][4];
	GLfloat vertices[4][3];

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
	vertices[1][0] = -1.0f; vertices[1][1] = -1.0f; vertices[1][2] = 0.0f;
	// blue
	colors[2][0] = 0.0f; colors[2][1] = 0.0f; colors[2][2] = 1.0f; colors[2][3] = 1.0f;
	// Right Of Triangle
	vertices[2][0] = 1.0f; vertices[2][1] = -1.0f; vertices[2][2] = 0.0f;

	// Drawing Using Triangles, draw triangles using 3 vertices
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Move Right 3 Units
	glLoadIdentity();
	glTranslatef((view_x + 1.5f), (view_y + 0.0f), (view_z - 5.0f));

	// rotate the square about the X axis
    glRotatef(rquad, 1.0f, 0.0f, 0.0f);

	// Set The Color To Blue One Time Only
	glColor4f(0.5f, 0.5f, 1.0f, 1.0f);

	// Disable vertex color array, use global color
	glDisableClientState(GL_COLOR_ARRAY);

	// Top Right Of The Quad
	vertices[0][0] = 1.0f;  vertices[0][1] = 1.0f;  vertices[0][2] = 0.0f;
	// Top Left Of The Quad
	vertices[1][0] = -1.0f; vertices[1][1] = 1.0f;  vertices[1][2] = 0.0f;
	// Bottom Left Of The Quad
	vertices[2][0] = 1.0f;  vertices[2][1] = -1.0f; vertices[2][2] = 0.0f;
	// Bottom Right Of The Quad
	vertices[3][0] = -1.0f; vertices[3][1] = -1.0f; vertices[3][2] = 0.0f;

	// Drawing using triangle strips, draw triangles using 4 vertices
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Disable vertex array
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


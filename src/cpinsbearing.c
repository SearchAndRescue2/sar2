#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>
#include "matrixmath.h"
#include "gw.h"
#include "v3dtex.h"
#include "cpvalues.h"
#include "cpins.h"
#include "cpinsbearing.h"
#include "cp.h"


CPIns *CPInsBearingNew(void *cp);
static void CPInsBearingValuesChanged(
	CPIns *ins, ControlPanelValues *v, void *data
);
static void CPInsBearingManage(
	CPIns *ins, ControlPanelValues *v, void *data
);
static void CPInsBearingDelete(CPIns *ins, void *data);


#ifndef PI
# define PI     3.14159265
#endif

#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? (float)atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))

#define RADTODEG(r)     ((r) * 180.0 / PI)
#define DEGTORAD(d)     ((d) * PI / 180.0)

#define NORMAL3F(x,y,z) (glNormal3f(    \
			 (GLfloat)(x),  \
			 (GLfloat)(z),  \
			 (GLfloat)-(y)  \
			))
#define TEXCOORD2F(x,y) (glTexCoord2f(          \
			 (GLfloat)(x),          \
			 (GLfloat)(1.0 - (y))   \
			))
#define VERTEX3FC(x,y,z) (glVertex3f(			\
			 (GLfloat)((x) / 100.0),        \
			 (GLfloat)((z) / 100.0),        \
			 (GLfloat)((y) / -100.0)        \
			))
#define ROTATEH(r)      (glRotatef((GLfloat)-RADTODEG(r), \
			0.0f, 1.0f, 0.0f))
#define ROTATEP(r)      (glRotatef((GLfloat)-RADTODEG(r), \
			1.0f, 0.0f, 0.0f))
#define ROTATEB(r)      (glRotatef((GLfloat)-RADTODEG(r), \
			0.0f, 0.0f, 1.0f))
#define TRANSLATE(x,y,z)        (glTranslatef(          \
			 (GLfloat)((x) / 100.0),        \
			 (GLfloat)((z) / 100.0),        \
			 (GLfloat)((y) / -100.0)        \
			))


/*
 *      Creates a new bearing instrument.
 */
CPIns *CPInsBearingNew(void *cp)
{
	CPInsBearing *insv;
	CPIns *ins = CPInsNew(
	    sizeof(CPInsBearing),
	    cp,
	    CPINS_TYPE_BEARING,
	    "Bearing"
	);
	if(ins == NULL)
	    return(NULL);

	insv = CPINS_BEARING(ins);
	insv->bearing_mag_last = (float)DEGTORAD(0.0);
	insv->bearing_mag_vel = (float)DEGTORAD(0.0);
	insv->bearing_mag_vel_dec = (float)DEGTORAD(5.0);
	insv->bearing_mechanical_offset = (float)DEGTORAD(0.0);

	CPInsSetFunction(
	    ins, "values_changed",
	    CPInsBearingValuesChanged, NULL
	);
	CPInsSetFunction(
	    ins, "manage",
	    CPInsBearingManage, NULL
	);
	CPInsSetFunction(
	    ins, "delete",
	    CPInsBearingDelete, NULL
	);

	return(ins);
}

/*
 *	"values_changed" callback.
 */
static void CPInsBearingValuesChanged(
	CPIns *ins, ControlPanelValues *v, void *data
)
{
	ControlPanel *cp;
	gw_display_struct *display;
	int cstate;
	GLfloat width, height;
	v3d_texture_ref_struct *tex_bg, *tex_fg;
	CPInsBearing *insv = CPINS_BEARING(ins);
	if((insv == NULL) || (v == NULL))
	    return;

	cp = CONTROL_PANEL(ins->cp);
	display = CONTROL_PANEL_DISPLAY(cp);
	if(display == NULL)
	    return;

	cstate = v->color_state;

	/* At this point the GL perspective matrix should already be
	 * orthoginal and it's size set to match the size of the
	 * frame buffer.  Now get the size of the frame buffer.
	 */
	width = (GLfloat)ins->res_width;
	height = (GLfloat)ins->res_height;

	/* Get instrument background and foreground textures. */
	tex_bg = ins->tex_bg;
	tex_fg = ins->tex_fg;


	/* Update bearing drift. */
	if(True)
	{
	    insv->bearing_mag_last = v->heading;
	}



	/* Begin drawing. */

	/* Draw background. */
	if(tex_bg != NULL)
	{
	    double a[3 * 1], r[3 * 1];
	    GLfloat	half_width = width / 2,
			half_height = height / 2;
	    float heading = insv->bearing_mag_last +
		insv->bearing_mechanical_offset;

	    glColor3f(1.0f, 1.0f, 1.0f);
	    StateGLEnable(&display->state_gl, GL_TEXTURE_2D);
	    V3DTextureSelect(tex_bg);

	    glBegin(GL_QUADS);
	    {
		a[2] = 0.0;

#define DO_SET_VERTEX	{		\
 MatrixRotateHeading3(a, heading, r);	\
 glVertex2f(				\
  (GLfloat)(r[0] + half_width),		\
  (GLfloat)(half_height - r[1])		\
 );					\
}

		glTexCoord2f(0.0f, 0.0f);
		a[0] = -half_width;
		a[1] = half_height;
		DO_SET_VERTEX

		glTexCoord2f(0.0f, 1.0f);
		a[0] = -half_width;
		a[1] = -half_height;
		DO_SET_VERTEX

		glTexCoord2f(1.0f, 1.0f);
		a[0] = half_width;
		a[1] = -half_height;
		DO_SET_VERTEX

		glTexCoord2f(1.0f, 0.0f);
		a[0] = half_width;
		a[1] = half_height;
		DO_SET_VERTEX

#undef DO_SET_VERTEX
	    }
	    glEnd();
	}


	/* Draw foretround. */
	if(tex_fg != NULL)
	{
	    glColor3f(1.0f, 1.0f, 1.0f);
	    StateGLEnable(&display->state_gl, GL_TEXTURE_2D);
	    V3DTextureSelect(tex_fg);

	    glBegin(GL_QUADS);
	    {
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.0f, height);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(width, height);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(width, 0.0f);
	    }
	    glEnd();
	}

	CPInsRealizeTexture(ins);
}

/*
 *      "manage" callback.
 */
static void CPInsBearingManage(
	CPIns *ins, ControlPanelValues *v, void *data
)
{
	CPInsBearing *insv = CPINS_BEARING(ins);
	if(insv == NULL)
	    return;



}

/*
 *      Deletes resources of instrument but not instrument itself.
 */
static void CPInsBearingDelete(CPIns *ins, void *data)
{
	CPInsBearing *insv = CPINS_BEARING(ins);
	if(insv == NULL)
	    return;
}

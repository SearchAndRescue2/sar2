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
#include "cpinshorizon.h"
#include "cp.h"


CPIns *CPInsHorizonNew(void *cp);
static void CPInsHorizonDrawHorizonPlate(
	GLfloat p, GLfloat b,                   /* In radians. */
	GLfloat width, GLfloat height,
	int deg_visible,			/* Degrees visible. */
	GLfloat bank_notch_radius,		/* 0.0 to 1.0 */
	CPColor *bg, CPColor *fg, CPColor *text
);
static void CPInsHorizonValuesChanged(
	CPIns *ins, ControlPanelValues *v, void *data
);
static void CPInsHorizonManage(
	CPIns *ins, ControlPanelValues *v, void *data
);
static void CPInsHorizonDelete(CPIns *ins, void *data);


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
 *      Creates a new artificial horizon instrument.
 */
CPIns *CPInsHorizonNew(void *cp)
{
	CPInsHorizon *insv;
	CPIns *ins = CPInsNew(
	    sizeof(CPInsHorizon),
	    cp,
	    CPINS_TYPE_HORIZON,
	    "Horizon"
	);
	if(ins == NULL)
	    return(NULL);

	insv = CPINS_HORIZON(ins);
	insv->deg_visible = 40;
	insv->bank_notch_radius = 0.85f;

	CPInsSetFunction(
	    ins, "values_changed",
	    CPInsHorizonValuesChanged, NULL
	);
	CPInsSetFunction(
	    ins, "manage",
	    CPInsHorizonManage, NULL
	);
	CPInsSetFunction(
	    ins, "delete",
	    CPInsHorizonDelete, NULL
	);

	return(ins);
}

/*
 *	Draws the horizon ticks and plates.
 */
static void CPInsHorizonDrawHorizonPlate(
	GLfloat p, GLfloat b,			/* In radians. */
	GLfloat width, GLfloat height,
	int deg_visible,			/* Degrees visible. */
	GLfloat bank_notch_radius,		/* 0.0 to 1.0 */
	CPColor *bg, CPColor *fg, CPColor *text
)
{
	int 	ppd;			/* Pixels per degree. */
	float	p_deg,			/* Delta pitch in degrees. */
		half_width = width / 2,
		half_height = height / 2;
	double a[3 * 1], r[3 * 1];

	/* All calculations will be relative to center 0, 0 and
	 * then translated (and y axis flipped) just before passing
	 * to each drawing function.
	 */

	/* Calculate pixels per degree, currently we want to show
	 * a range of 40 degrees.
	 */
	ppd = (int)MAX(height / deg_visible, 1);

	/* Calculate delta pitch in degrees where negative pitch is down
	 * and positive pitch is up. (90.0, -90.0).
	 */
	if(p > DEGTORAD(180))
	    p_deg = (float)(360.0 - RADTODEG(p));
	else
	    p_deg = (float)-RADTODEG(p);


/* Takes vector a and rotates it by the bank b (in radians) and sets
 * its vertex (applying appropriate offsets).
 * The vector a must be centered at around 0,0.
 */
#define DO_SET_VERTEX	{		\
 MatrixRotateHeading3(a, -b, r);	\
 glVertex2f(				\
  (GLfloat)(r[0] + half_width),		\
  (GLfloat)(half_height - r[1])		\
 );					\
}


	/* Draw sky. */
	glColor3f(bg->r, bg->g, bg->b);
	glBegin(GL_QUADS);
	{
	    glVertex2f(0.0f, 0.0f);
	    glVertex2f(0.0f, height);
	    glVertex2f(width, height);
	    glVertex2f(width, 0.0f);
	}
	glEnd();


	/* Draw ground. */
	if(p_deg <= deg_visible)
	{
	    a[2] = 0.0;

	    glColor3f(fg->r, fg->g, fg->b);
	    glBegin(GL_QUADS);
	    {
		/* Upper left. */
		a[0] = -width;
		a[1] = MIN((0.0 - p_deg), deg_visible) * ppd;
		DO_SET_VERTEX

		/* Lower left. */
		a[0] = -width;
		a[1] = MAX((-130.0 - p_deg), -deg_visible) * ppd;
		DO_SET_VERTEX

		/* Lower right. */
		a[0] = width;
		a[1] = MAX((-130.0 - p_deg), -deg_visible) * ppd;
		DO_SET_VERTEX

		/* Upper right. */
		a[0] = width;
		a[1] = MIN((0.0 - p_deg), deg_visible) * ppd;
		DO_SET_VERTEX
	    }
	    glEnd();

	    glColor3f(text->r, text->g, text->b);
	    glBegin(GL_LINES);
	    {
		a[0] = -0.01 * width;
		a[1] = (0.0 - p_deg) * ppd;
		DO_SET_VERTEX
		a[0] = -1.0 * width;
		a[1] = (-40.0 - p_deg) * ppd;
		DO_SET_VERTEX

		a[0] = 0.01 * width;
		a[1] = (0.0 - p_deg) * ppd;
		DO_SET_VERTEX
		a[0] = 1.0 * width;
		a[1] = (-40.0 - p_deg) * ppd;
		DO_SET_VERTEX

		a[0] = -0.2 * width;
		a[1] = (0.0 - p_deg) * ppd;
		DO_SET_VERTEX
		a[0] = -1.0 * width;
		a[1] = (-20.0 - p_deg) * ppd;
		DO_SET_VERTEX
		a[0] = 0.2 * width;
		a[1] = (0.0 - p_deg) * ppd;
		DO_SET_VERTEX
		a[0] = 1.0 * width;
		a[1] = (-20.0 - p_deg) * ppd;
		DO_SET_VERTEX
	    }
	    glEnd();

	}


	/* Draw bank notch. */
	glColor3f(text->r, text->g, text->b);
	glBegin(GL_TRIANGLES);
	{
	    a[0] = 0.0;
	    a[1] = bank_notch_radius * half_height;
	    DO_SET_VERTEX
	    a[0] = -0.05 * half_width;
	    a[1] = (bank_notch_radius - 0.15) * half_height;
	    DO_SET_VERTEX
	    a[0] = 0.05 * half_width;
	    a[1] = (bank_notch_radius - 0.15) * half_height;
	    DO_SET_VERTEX
	}
	glEnd();

	/* Begin drawing ticks. */
	glColor3f(text->r, text->g, text->b);
	glBegin(GL_LINES);
	{
	    int i, d, degree = 0;

	    a[2] = 0.0;

	    /* Horizon (0 degree) line. */
	    a[0] = -0.35 * width;
	    a[1] = (degree - p_deg) * ppd;
	    DO_SET_VERTEX
	    a[0] = -0.1 * width;
	    a[1] = (degree - p_deg) * ppd;
	    DO_SET_VERTEX
	    a[0] = 0.1 * width;
	    a[1] = (degree - p_deg) * ppd;
	    DO_SET_VERTEX
	    a[0] = 0.35 * width;
	    a[1] = (degree - p_deg) * ppd;
	    DO_SET_VERTEX

	    /* Begin drawing 10 degree ticks, start degree off to the
	     * nearest 10th degree.
	     */
	    degree = (int)(p_deg / 10) * 10;
	    for(i = 0, d = degree - (2 * 10);
		i < 6;
		i++, d += 10
	    )
	    {
		if(d == 0)
		    continue;

		a[0] = -0.2 * width;
		a[1] = (d - p_deg) * ppd;
		DO_SET_VERTEX
		a[0] = 0.2 * width;
		a[1] = (d - p_deg) * ppd;
		DO_SET_VERTEX
	    }

	    /* Begin drawing 5 degree ticks. */
	    for(i = 0, d = degree - (2 * 10) + 5;
		i < 5;
		i++, d += 10
	    )
	    {
		if(d == 0)
		    continue;

		a[0] = -0.12 * width;
		a[1] = (d - p_deg) * ppd;
		DO_SET_VERTEX
		a[0] = 0.12 * width;
		a[1] = (d - p_deg) * ppd;
		DO_SET_VERTEX
	    }
	}
	glEnd();



#undef DO_SET_VERTEX
}

/*
 *	"values_changed" callback.
 */
static void CPInsHorizonValuesChanged(
	CPIns *ins, ControlPanelValues *v, void *data
)
{
	ControlPanel *cp;
	gw_display_struct *display;
	int cstate;
	GLfloat width, height;
	v3d_texture_ref_struct *tex_bg, *tex_fg;
	CPInsHorizon *insv = CPINS_HORIZON(ins);
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
	width = (float)ins->res_width;
	height = (float)ins->res_height;

	/* Get instrument background and foreground textures. */
	tex_bg = ins->tex_bg;
	tex_fg = ins->tex_fg;


	/* Begin drawing. */
/*
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
 */

	/* Draw background. */



	/* Draw horizon. */
	StateGLDisable(&display->state_gl, GL_TEXTURE_2D);
	CPInsHorizonDrawHorizonPlate(
	    v->pitch, v->bank,
	    width, height,
	    MAX(insv->deg_visible, 1),
	    insv->bank_notch_radius,
	    &ins->color_bg[cstate],
	    &ins->color_fg[cstate],
	    &ins->color_text[cstate]
	);


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
 *	"manage" callback.
 */
static void CPInsHorizonManage(
	CPIns *ins, ControlPanelValues *v, void *data
)
{
	CPInsHorizon *insv = CPINS_HORIZON(ins);
	if(insv == NULL)
	    return;



}

/*
 *      Deletes resources of instrument but not instrument itself.
 */
static void CPInsHorizonDelete(CPIns *ins, void *data)
{
	CPInsHorizon *insv = CPINS_HORIZON(ins);
	if(insv == NULL)
	    return;
}

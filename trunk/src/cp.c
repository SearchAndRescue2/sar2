#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>
#include "gw.h"
#include "v3dtex.h"
#include "cpvalues.h"
#include "cpins.h"
#include "cp.h"


time_t CPCurrentTime(ControlPanel *cp);

CPIns *CPGetInsByNumber(ControlPanel *cp, int i);
int CPAppendIns(ControlPanel *cp, CPIns *ins);

ControlPanel *CPNew(gw_display_struct *display);
void CPDelete(ControlPanel *cp);

void CPSetPosition(
	ControlPanel *cp,
	GLfloat x, GLfloat y, GLfloat z	/* In centimeters. */
);
void CPSetDirection(
	ControlPanel *cp,
        GLfloat heading, GLfloat pitch, GLfloat bank	/* In radians. */
);
void CPSetSize(
        ControlPanel *cp,
	GLfloat width, GLfloat height	/* In centimeters. */
);
void CPSetTexture(ControlPanel *cp, const char *path);

void CPResetTimmers(ControlPanel *cp, time_t t);
void CPChangeValues(ControlPanel *cp, ControlPanelValues *v);
void CPDraw(ControlPanel *cp);
void CPManage(ControlPanel *cp, ControlPanelValues *v);


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

#define NORMAL3F(x,y,z)	(glNormal3f( 	\
			 (GLfloat)(x),	\
			 (GLfloat)(z),	\
			 (GLfloat)-(y)	\
			))
#define TEXCOORD2F(x,y)	(glTexCoord2f(		\
			 (GLfloat)(x),		\
			 (GLfloat)(y)		\
			))
#define VERTEX3F(x,y,z)	(glVertex3f(			\
			 (GLfloat)((x) / 100.0),	\
			 (GLfloat)((z) / 100.0),	\
			 (GLfloat)((y) / -100.0)	\
			))
#define ROTATEH(r)	(glRotatef((GLfloat)-RADTODEG(r), \
			 0.0f, 1.0f, 0.0f))
#define ROTATEP(r)	(glRotatef((GLfloat)-RADTODEG(r), \
			 1.0f, 0.0f, 0.0f))
#define ROTATEB(r)	(glRotatef((GLfloat)-RADTODEG(r), \
			 0.0f, 0.0f, 1.0f))
#define TRANSLATE(x,y,z)	(glTranslatef(		\
                         (GLfloat)((x) / 100.0),	\
                         (GLfloat)((z) / 100.0),	\
                         (GLfloat)((y) / -100.0)	\
                        ))


/*
 *	Returns the current time in milliseconds.
 */
time_t CPCurrentTime(ControlPanel *cp)
{
	const ControlPanelValues *v = CONTROL_PANEL_VALUES(cp);
	return((v != NULL) ? v->current_time : 0);
}

/*
 *	Returns the instrument at index i.
 */
CPIns *CPGetInsByNumber(ControlPanel *cp, int i)
{
	if(cp == NULL)
	    return(NULL);
	if((i < 0) || (i >= cp->total_ins))
	    return(NULL);
	else
	    return(cp->ins[i]);
}

/*
 *	Appends the instrument to the control panel's list of 
 *	instruments, returning the new index or -1 on error.
 */
int CPAppendIns(ControlPanel *cp, CPIns *ins)
{
	int i;

	if((cp == NULL) || (ins == NULL))
	    return(-1);

	i = MAX(cp->total_ins, 0);
	cp->total_ins = i + 1;
	cp->ins = (CPIns **)realloc(
	    cp->ins,
	    cp->total_ins * sizeof(CPIns *)
	);
	if(cp->ins == NULL)
	{
	    cp->total_ins = 0;
	    return(-1);
	}

	cp->ins[i] = ins;

	return(i);
}

/*
 *	Creates a new control panel.
 */
ControlPanel *CPNew(gw_display_struct *display)
{
	ControlPanel *cp = CONTROL_PANEL(calloc(1, sizeof(ControlPanel)));
	if(cp == NULL)
	    return(NULL);

	cp->display = display;
	cp->width = 100.0f;
	cp->height = 10.0f;

	return(cp);
}

/*
 *	Deletes the control panel and all it's resources.
 */
void CPDelete(ControlPanel *cp)
{
	int i;

	if(cp == NULL)
	    return;

	/* Instruments. */
	for(i = 0; i < cp->total_ins; i++)
	    CPInsDelete(cp->ins[i]);
	free(cp->ins);
	cp->ins = NULL;
	cp->total_ins = 0;

        V3DTextureDestroy(cp->tex);
        cp->tex = NULL;

	free(cp);
}


/*
 *	Set control panel position.
 */
void CPSetPosition(
        ControlPanel *cp,
        GLfloat x, GLfloat y, GLfloat z /* In centimeters. */
)
{
        if(cp == NULL)
            return;

	cp->x = x;
	cp->y = y;
	cp->z = z;
}

/*
 *	Set control panel direction.
 */
void CPSetDirection(
        ControlPanel *cp,
        GLfloat heading, GLfloat pitch, GLfloat bank    /* In radians. */
)
{
        if(cp == NULL)
            return;

	cp->heading = heading;
	cp->pitch = pitch;
	cp->bank = bank;
}

/*
 *	Set control panel size.
 */
void CPSetSize(
        ControlPanel *cp,
        GLfloat width, GLfloat height   /* In centimeters. */
)
{
        if(cp == NULL)
            return;

	cp->width = width;
	cp->height = height;
}

/*
 *	Set control panel texture.
 */
void CPSetTexture(ControlPanel *cp, const char *path)
{
        if(cp == NULL)
            return;

        V3DTextureDestroy(cp->tex);
        cp->tex = NULL;

	if((path != NULL) ? (*path == '\0') : GL_TRUE)
	    return;

	cp->tex = V3DTextureLoadFromFile2DPreempt(
	    path, "control_panel_tex",
	    V3D_TEX_FORMAT_RGBA
	);
}


/*
 *	Resets all timmers on the control panel to t.
 */
void CPResetTimmers(ControlPanel *cp, time_t t)
{
	int i;
	ControlPanelValues *v;

        if(cp == NULL)
            return;

	v = &cp->values;
	v->current_time = t;

	for(i = 0; i < cp->total_ins; i++)
            CPInsResetTimmers(cp->ins[i], t);
}

/*
 *	Updates the values of the control panel and all its instruments.
 *
 *	The GL frame buffer will be used to update the instrument's
 *	textures.  The viewport and other GL states will be modified to
 *	accomidate this.
 */
void CPChangeValues(ControlPanel *cp, ControlPanelValues *v)
{
        int i;
        ControlPanelValues *cpv;

        gw_display_struct *display = CONTROL_PANEL_DISPLAY(cp);
        if((display == NULL) || (v == NULL))
            return;

	/* Set up GL states. */
        cpv = CONTROL_PANEL_VALUES(cp);
	memcpy(cpv, v, sizeof(ControlPanelValues));

        /* Instruments. */
        for(i = 0; i < cp->total_ins; i++)
            CPInsChangeValues(cp->ins[i], v);
}


/*
 *	Redraws the control panel and all its instruments.
 *
 *	The model view translation should currently be at the center of
 *	the cockpit.
 */
void CPDraw(ControlPanel *cp)
{
	int i;
	gw_display_struct *display = CONTROL_PANEL_DISPLAY(cp);
	v3d_texture_ref_struct *tex;
	ControlPanelValues *v;
        if(display == NULL) 
            return;

        v = CONTROL_PANEL_VALUES(cp);
	if(v == NULL)
	    return;

	/* Set up GL states. */
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

	StateGLDisable(&display->state_gl, GL_LIGHTING);
	StateGLDisable(&display->state_gl, GL_BLEND);
	StateGLDisable(&display->state_gl, GL_FOG);
	StateGLDisable(&display->state_gl, GL_DEPTH_TEST);
	StateGLDepthFunc(&display->state_gl, GL_ALWAYS);

	StateGLEnable(&display->state_gl, GL_ALPHA_TEST);
	StateGLAlphaFunc(&display->state_gl, GL_GREATER, 0.5);
	StateGLShadeModel(&display->state_gl, GL_FLAT);
	StateGLEnable(&display->state_gl, GL_TEXTURE_2D);
	StateGLTexEnvI(
	    &display->state_gl,
	    GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE
	);
        StateGLDisable(&display->state_gl, GL_CULL_FACE);
        StateGLDisable(&display->state_gl, GL_COLOR_MATERIAL);

	/* Draw fullscreen? */
	if(v->fullscreen)
	{
	    int width, height;
	    GWContextGet(
		display, GWContextCurrent(display),
		NULL, NULL,
		NULL, NULL,
		&width, &height
	    );
	    if((width > 1) && (height > 1) &&
	       (cp->width > 1) && (cp->height > 1)
	    )
	    {
		CPRectangle rect;

		GWOrtho2DCoord(
		    display,
		    0.0f, (float)(width - 1),
		    0.0f, (float)(height - 1)
		);

		/* Calculate position and size of control panel in
		 * window coordinates.
		 */
		rect.width = (GLfloat)width;
		rect.height = (GLfloat)width * cp->height / cp->width;
		rect.x = 0;
		rect.y = height - rect.height;

                /* Get control panel background texture and draw
                 * background.
                 */
                tex = cp->tex;
                if(tex != NULL)
                {
                    glColor3f(1.0f, 1.0f, 1.0f);
                    V3DTextureSelect(tex);
                }
                else
                {
                    glColor3f(0.0f, 0.0f, 0.0f);
		}
                glBegin(GL_QUADS);
                {
                    glTexCoord2f(0, 0);
		    glVertex2f(rect.x, rect.y);
                    glTexCoord2f(0, 1);
                    glVertex2f(rect.x, rect.y + rect.height - 1);
                    glTexCoord2f(1, 1);
                    glVertex2f(rect.x + rect.width - 1, rect.y + rect.height - 1);
                    glTexCoord2f(1, 0);
                    glVertex2f(rect.x + rect.width - 1, rect.y);
                }
                glEnd();

                /* Instruments. */
                for(i = 0; i < cp->total_ins; i++)
                    CPInsDrawFullScreen(cp->ins[i], &rect);
	    }
	}
	else
	{
	    /* Draw 3d, so translate to control panel position. */
	    glPushMatrix();
	    TRANSLATE(cp->x, cp->y, cp->z);
	    if(cp->heading != 0.0f)
		ROTATEH(cp->heading);
	    if(cp->pitch != 0.0f)
		ROTATEP(cp->pitch);
	    if(cp->bank != 0.0f)
		ROTATEB(cp->bank);

	    /* Get control panel background texture and draw
	     * background.
	     */
	    tex = cp->tex;
	    if(tex != NULL)
	    {
		glColor3f(1.0f, 1.0f, 1.0f);
		V3DTextureSelect(tex);
	    }
	    else
	    {
		glColor3f(0.0f, 0.0f, 0.0f);
	    }
	    glBegin(GL_QUADS);
	    {
		NORMAL3F(0.0, 0.0, 1.0);
	        TEXCOORD2F(0, 0);
	        VERTEX3F(0.0, 0.0, 0.0);
	        TEXCOORD2F(0, 1);
                VERTEX3F(0.0, -cp->height, 0.0);
	        TEXCOORD2F(1, 1);
                VERTEX3F(cp->width, -cp->height, 0.0);
	        TEXCOORD2F(1, 0);
                VERTEX3F(cp->width, 0.0, 0.0);
	    }
	    glEnd();

            /* Instruments. */
            for(i = 0; i < cp->total_ins; i++)
                CPInsDraw(cp->ins[i]);

	    glPopMatrix();

	}
}


/*
 *	Manages the control panel and all its instruments.
 *
 *	This function should be called once per loop.
 *
 *	Only the members current_time and time_compensation in v are
 *	used.
 */
void CPManage(ControlPanel *cp, ControlPanelValues *v)
{
	int i;

	if((cp == NULL) || (v == NULL))
	    return;

	for(i = 0; i < cp->total_ins; i++)
            CPInsManage(cp->ins[i], v);
}


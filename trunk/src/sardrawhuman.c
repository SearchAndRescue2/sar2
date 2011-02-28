#include <math.h>
#include <sys/types.h>

#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#include "gw.h"
#include "stategl.h"
#include "obj.h"
#include "objutils.h"
#include "sar.h"
#include "sardraw.h"
#include "sardrawdefs.h"
#include "config.h"


void SARDrawHumanIterate(
	sar_dc_struct *dc,
	float height, float mass,
	sar_human_flags flags,
	const sar_color_struct *palette,	/* Human colors */
	int water_ripple_tex_num,
	sar_grad_anim_t anim_pos
);
void SARDrawHuman(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_human_struct *human
);


/*
 *	Called by SARDrawHuman() to draw one human.
 *
 *	All inputs assumed valid.
 */
void SARDrawHumanIterate(
	sar_dc_struct *dc,
	float height, float mass,
	sar_human_flags flags,
	const sar_color_struct *palette,	/* Human colors */
	int water_ripple_tex_num,
	sar_grad_anim_t anim_pos
)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	const sar_option_struct *opt = dc->option;
	sar_scene_struct *scene = dc->scene;
	StateGLBoolean lighting = state->lighting;
	GLenum shade_model_mode = state->shade_model_mode;
	StateGLBoolean texture_2d_state = state->texture_2d;
	float anim_coeff = (float)anim_pos /
	    (float)((sar_grad_anim_t)-1);

	float torso_angle;
	float left_bisep_angle, right_bisep_angle;
	float left_shoulder_angle, right_shoulder_angle;
	float left_trisep_angle, right_trisep_angle;
	float left_hip_to_thigh_angle, right_hip_to_thigh_angle;
	float left_knee_to_calv_angle, right_knee_to_calv_angle;

	float thigh_len = 0.3f;
	float calv_len = 0.4f;
	float torso_len = 0.8f;
	float bisep_len = 0.3f;
	float trisep_len = 0.4f;
	float base_to_torso = 0.0f;	/* Ground to bottom of torso
					 * (calculated later).
					 */
	float streatcher_height = 0.8f;	/* Ground to bed of streatcher. */

	const sar_color_struct *c;
	Boolean draw_shadow_std = False;

/* Macro to set the color pointed to by c.  The color must be specifying
 * a color on the human's body, because dc->flir will be checked and if
 * it is True then the color will be brightened.
 */
#define SET_BODY_COLOR					\
{ if(dc->flir) {						\
 float g = (float)(((c->r + c->g + c->b) / 3.0) / 2.0);	\
 glColor4f(0.5f + g, 1.0f, 0.5f + g, 1.0f);		\
} else {						\
 glColor4f(c->r, c->g, c->b, c->a);			\
} }

	/* Set up gl states. */
	if(dc->flir)
	    StateGLDisable(state, GL_LIGHTING);
	StateGLDisable(state, GL_TEXTURE_2D);
	V3DTextureSelect(NULL);
	StateGLShadeModel(state, opt->gl_shade_model);

	/* Calculate angles of body parts. Note that all zero radians
	 * indicates human is standing upright with hands to side.
	 */
	if(flags & SAR_HUMAN_FLAG_LYING)
	{
	    torso_angle = (float)(1.5 * PI);
	    left_shoulder_angle = (float)(0.0 * PI);
	    right_shoulder_angle = (float)(0.0 * PI);
	    left_bisep_angle = (float)(0.0 * PI);
	    right_bisep_angle = (float)(0.0 * PI);
	    left_trisep_angle = (float)(0.0 * PI);
	    right_trisep_angle = (float)(0.0 * PI);
	    left_hip_to_thigh_angle = (float)(0.0 * PI);
	    right_hip_to_thigh_angle = (float)(0.0 * PI);
	    left_knee_to_calv_angle = (float)(0.0 * PI);
	    right_knee_to_calv_angle = (float)(0.0 * PI);

	    base_to_torso = thigh_len + calv_len;

	    if(flags & SAR_HUMAN_FLAG_ON_STREATCHER)
	    {
		/* Some translating will be done later. */
	    }
	}
	else if(flags & SAR_HUMAN_FLAG_SIT)
	{
	    /* Sitting base at tush. */
	    torso_angle = (float)(0.0 * PI);
	    left_shoulder_angle = (float)(0.0 * PI);
	    right_shoulder_angle = (float)(0.0 * PI);
	    left_bisep_angle = (float)(0.0 * PI);
	    right_bisep_angle = (float)(0.0 * PI);
	    left_trisep_angle = (float)(1.75 * PI);
	    right_trisep_angle = (float)(1.75 * PI);
	    left_hip_to_thigh_angle = (float)(1.5 * PI);
	    right_hip_to_thigh_angle = (float)(1.5 * PI);
	    left_knee_to_calv_angle = (float)(0.5 * PI);
	    right_knee_to_calv_angle = (float)(0.5 * PI);

	    base_to_torso = 0.0f;
	}
	else if(flags & SAR_HUMAN_FLAG_SIT_DOWN)
	{
	    torso_angle = (float)(0.0 * PI);
	    left_shoulder_angle = (float)(0.0 * PI);
	    right_shoulder_angle = (float)(0.0 * PI);
	    left_bisep_angle = (float)(0.0 * PI);
	    right_bisep_angle = (float)(0.0 * PI);
	    left_trisep_angle = (float)(1.75 * PI);
	    right_trisep_angle = (float)(1.75 * PI);
	    left_hip_to_thigh_angle = (float)(1.5 * PI);
	    right_hip_to_thigh_angle = (float)(1.5 * PI);
	    left_knee_to_calv_angle = (float)(0.0 * PI);
	    right_knee_to_calv_angle = (float)(0.0 * PI);

	    base_to_torso = 0.0f;
	}
	else if(flags & SAR_HUMAN_FLAG_SIT_UP)
	{
	    /* Sitting base at feet. */
	    torso_angle = (float)(0.0 * PI);
	    left_shoulder_angle = (float)(0.0 * PI);
	    right_shoulder_angle = (float)(0.0 * PI);
	    left_bisep_angle = (float)(0.0 * PI);
	    right_bisep_angle = (float)(0.0 * PI);
	    left_trisep_angle = (float)(1.75 * PI);
	    right_trisep_angle = (float)(1.75 * PI);
	    left_hip_to_thigh_angle = (float)(1.5 * PI);
	    right_hip_to_thigh_angle = (float)(1.5 * PI); 
	    left_knee_to_calv_angle = (float)(0.5 * PI);
	    right_knee_to_calv_angle = (float)(0.5 * PI);
	
	    base_to_torso = calv_len;
	}
	else if(flags & SAR_HUMAN_FLAG_DIVER_CATCHER)
	{
	    /* Diver catching victim, used for hoist rescue end drawing. */
	    torso_angle = (float)(0.0 * PI);
	    left_shoulder_angle = (float)(0.1 * PI);
	    right_shoulder_angle = (float)(1.9 * PI);
	    left_bisep_angle = (float)(1.8 * PI);
	    right_bisep_angle = (float)(1.8 * PI);
	    left_trisep_angle = (float)(1.8 * PI);   
	    right_trisep_angle = (float)(1.8 * PI);
	    left_hip_to_thigh_angle = (float)(1.9 * PI);
	    right_hip_to_thigh_angle = (float)(1.9 * PI);
	    left_knee_to_calv_angle = (float)(0.2 * PI); 
	    right_knee_to_calv_angle = (float)(0.2 * PI);

	    /* If gripped it implies on end of rescue hoist rope. */
	    if(flags & SAR_HUMAN_FLAG_GRIPPED)
		base_to_torso = torso_len;
	    else if(flags & SAR_HUMAN_FLAG_IN_WATER)
		base_to_torso = torso_len * -0.92f;
	    else
		base_to_torso = thigh_len + calv_len;
	}
	/* Run should be checked last since lying and setting have
	 * precidence.
	 */
	else if(flags & SAR_HUMAN_FLAG_RUN)
	{
	    if(anim_coeff > 0.5f)
		anim_coeff = (1.0f - anim_coeff) / 0.5f;
	    else
		anim_coeff = anim_coeff / 0.5f;

	    torso_angle = (float)(0.1 * PI);
	    left_shoulder_angle = (float)(0.0 * PI);
	    right_shoulder_angle = (float)(0.0 * PI);
	    if(flags & SAR_HUMAN_FLAG_PUSHING)
	    {
		left_bisep_angle = (float)(1.7 * PI);
		right_bisep_angle = (float)(1.7 * PI);
		left_trisep_angle = (float)(1.6 * PI);
		right_trisep_angle = (float)(1.6 * PI);
	    }
	    else
	    {
		left_bisep_angle = (float)(
		    (1.7 + (0.35 * (1.0 - anim_coeff))) * PI
		);
		right_bisep_angle = (float)(
		    (1.7 + (0.35 * anim_coeff)) * PI
		);
		left_trisep_angle = (float)(
		    (1.6 + (0.15 * (1.0 - anim_coeff))) * PI
		);
		right_trisep_angle = (float)(
		    (1.6 + (0.15 * anim_coeff)) * PI
		);
	    }
	    left_hip_to_thigh_angle = (float)(
		(0.2 - (0.5 * anim_coeff)) * PI
	    );
	    right_hip_to_thigh_angle = (float)(
		(0.2 - (0.5 * (1.0 - anim_coeff))) * PI
	    );
	    left_knee_to_calv_angle = (float)(0.2 * PI);
	    right_knee_to_calv_angle = (float)(0.2 * PI);

	    base_to_torso = thigh_len + calv_len;

	    draw_shadow_std = True;
	}
	/* All else assume just standing. */
	else
	{
	    /* Standing in water? */
	    if(flags & SAR_HUMAN_FLAG_IN_WATER)
	    {
		/* Standing in water, arms fore and moving. */
		if(anim_coeff > 0.5f)
		    anim_coeff = (float)((1.0 - anim_coeff) / 0.5);
		else
		    anim_coeff = (float)(anim_coeff / 0.5);

		torso_angle = (float)(0.0 * PI);
		left_shoulder_angle = (float)((0.4 * anim_coeff) * PI);
		right_shoulder_angle = (float)((2 - (0.4 * anim_coeff)) * PI);
		left_bisep_angle = (float)(1.5 * PI);
		right_bisep_angle = (float)(1.5 * PI);
		left_trisep_angle = (float)(0.0 * PI);
	        right_trisep_angle = (float)(0.0 * PI);
		left_hip_to_thigh_angle = (float)(0.0 * PI);
		right_hip_to_thigh_angle = (float)(0.0 * PI);
		left_knee_to_calv_angle = (float)(0.0 * PI);
		right_knee_to_calv_angle = (float)(0.0 * PI);

		/* Since in water, move torso down. */
		base_to_torso = torso_len * -0.92f;
	    }
	    else
	    {
		/* Standing on ground and aware? */
		if(((flags & SAR_HUMAN_FLAG_NEED_RESCUE) &&
		    (flags & SAR_HUMAN_FLAG_AWARE)) ||
		   (flags & SAR_HUMAN_FLAG_ALERT)
		)
		{
		    /* Need rescue and aware that rescuer is
		     * near by, move arms fore and wave.
		     */
		    /* As well if we just want to alert */
		    if(anim_coeff > 0.5f)
			anim_coeff = (float)((1.0 - anim_coeff) / 0.5);
		    else
			anim_coeff = (float)(anim_coeff / 0.5);

		    torso_angle = (float)(0.0 * PI);
		    left_shoulder_angle = (float)(
			(0.7 + (0.4 * anim_coeff)) * PI
		    );
		    right_shoulder_angle = (float)(
			(1.3 - (0.4 * anim_coeff)) * PI
		    );
		    left_bisep_angle = (float)(0.0 * PI);
		    right_bisep_angle = (float)(0.0 * PI);
		    left_trisep_angle = (float)(0.0 * PI);
		    right_trisep_angle = (float)(0.0 * PI);
		    left_hip_to_thigh_angle = (float)(0.0 * PI);
		    right_hip_to_thigh_angle = (float)(0.0 * PI);
		    left_knee_to_calv_angle = (float)(0.0 * PI);
		    right_knee_to_calv_angle = (float)(0.0 * PI);

		    base_to_torso = thigh_len + calv_len;
		}
		else
		{
		    /* Just standing on ground. */
		    torso_angle = (float)(0.0 * PI);
		    left_shoulder_angle = (float)(0.0 * PI);
		    right_shoulder_angle = (float)(0.0 * PI);
		    left_bisep_angle = (float)(0.0 * PI);
		    right_bisep_angle = (float)(0.0 * PI);
		    left_trisep_angle = (float)(0.0 * PI);
		    right_trisep_angle = (float)(0.0 * PI);
		    left_hip_to_thigh_angle = (float)(0.0 * PI);
		    right_hip_to_thigh_angle = (float)(0.0 * PI);
		    left_knee_to_calv_angle = (float)(0.0 * PI);
		    right_knee_to_calv_angle = (float)(0.0 * PI);

		    base_to_torso = thigh_len + calv_len;
		}
		draw_shadow_std = True;
	    }
	}

	/* At this point the human's appendage length and rotation
	 * values have now been set up.
	 */

	/* Draw shadow? */
	if(draw_shadow_std)
	{
	    float theta, r = 0.5f;
	    StateGLBoolean lighting = state->lighting;
	    StateGLBoolean depth_mask_flag = state->depth_mask_flag;
	    GLenum shade_model_mode = state->shade_model_mode;

	    /* Set up GL states. */
	    if(lighting)
		StateGLDisable(state, GL_LIGHTING);
	    StateGLEnable(state, GL_BLEND);
	    StateGLDisable(state, GL_ALPHA_TEST);
	    StateGLDepthMask(state, GL_FALSE);
	    StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
	    StateGLBlendFunc(
		state, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
	    );
	    StateGLPolygonOffset(
		state,
		(GLfloat)opt->gl_polygon_offset_factor, -1.0f
	    );
	    StateGLShadeModel(state, GL_FLAT);

	    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
	    glBegin(GL_TRIANGLE_FAN);
	    glNormal3f(0.0f, 1.0f, 0.0f);
	    for(theta = 2.0f; theta > 0.0f; theta -= 0.25f)
		glVertex3f(
		    (float)(r * sin(theta * PI)),
		    0.0f,
		    (float)-(r * cos(theta * PI))
		);
	    glEnd();

	    /* Restore GL states. */
	    if(lighting)
		StateGLEnable(state, GL_LIGHTING);
	    StateGLDisable(state, GL_POLYGON_OFFSET_FILL);
	    StateGLDisable(state, GL_BLEND);
	    StateGLDepthMask(state, depth_mask_flag);
	    StateGLEnable(state, GL_ALPHA_TEST);
	    StateGLShadeModel(state, shade_model_mode);
	}

	/* Draw streatcher? */
	if(flags & SAR_HUMAN_FLAG_ON_STREATCHER)
	{
	    float x = 0.35f, y = 1.05f;
	    float h = streatcher_height - 0.1f, hl = 0.1f;

	    glColor4f(0.8f, 0.8f, 0.8f, 1.0f);

	    /* Bed of streatcher. */
	    glBegin(GL_QUADS);
	    {
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(x, h, -y);
		glVertex3f(-x, h, -y);
		glVertex3f(-x, h, y);
		glVertex3f(x, h, y);

		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(-x, h, -y);
		glVertex3f(x, h, -y);
		glVertex3f(x, h, y);
		glVertex3f(-x, h, y);
	    }
	    glEnd();

	    glColor4f(0.7f, 0.7f, 0.7f, 1.0f);

	    /* Left grids. */
	    glBegin(GL_LINE_LOOP);
	    {
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-x, hl, -y);
		glVertex3f(-x, h, y);
		glVertex3f(-x, hl, y);
		glVertex3f(-x, h, -y);
	    }
	    glEnd();

	    /* Right grids. */
	    glBegin(GL_LINE_LOOP);  
	    {
		glNormal3f(1.0, 0.0, 0.0);
		glVertex3f(x, hl, -y);
		glVertex3f(x, h, y);
		glVertex3f(x, hl, y);
		glVertex3f(x, h, -y);
	    }
	    glEnd();


	    glColor4f(0.1f, 0.1f, 0.1f, 1.0f);

	    /* Simple square wheels. */
	    glBegin(GL_QUADS);
	    {
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-x, 0.0f, -y);
		glVertex3f(-x, 0.0f, -(y - 0.1f));
		glVertex3f(-x, hl, -(y - 0.1f));
		glVertex3f(-x, hl, -y);
	    }
	    glEnd();

	    glBegin(GL_QUADS);        
	    {
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-x, hl, y);
		glVertex3f(-x, hl, (y - 0.1f));
		glVertex3f(-x, 0.0f, (y - 0.1f)); 
		glVertex3f(-x, 0.0f, y);
	    }
	    glEnd();

	    glBegin(GL_QUADS);
	    {
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(x, 0.0f, -y);
		glVertex3f(x, hl, -y);
		glVertex3f(x, hl, -(y - 0.1f));
		glVertex3f(x, 0.0f, -(y - 0.1f));
	    }
	    glEnd();

	    glBegin(GL_QUADS);
	    {
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(x, 0.0f, (y - 0.1f));
		glVertex3f(x, hl, (y - 0.1f));
		glVertex3f(x, hl, y);
		glVertex3f(x, 0.0f, y);
	    }
	    glEnd();
	}

	/* Begin drawing human. */

	/* Torso. */
	glPushMatrix();
	{
	    /* Check if on streatcher, if so translate up. */
	    if(flags & SAR_HUMAN_FLAG_ON_STREATCHER)  
	    {
		/* Push on extra matrix and move up (matrix poped later). */
		glPushMatrix();
		glTranslatef(
		    0.0f,
		    streatcher_height,
		    -(base_to_torso + 0.2f)
		);
	    }

	    /* Move to base of torso (the tush). */
	    glRotatef(
		(GLfloat)-SFMRadiansToDegrees(torso_angle),
		1.0f, 0.0f, 0.0f
	    );
	    glTranslatef(0.0f, base_to_torso, 0.0f);

	    /* Move to center of torso and draw torso. */
	    glPushMatrix();
	    {
		/* Hips of torso. */
		c = &palette[SAR_HUMAN_COLOR_HIPS];
		SET_BODY_COLOR
		glBegin(GL_QUADS);
		{
	            SARDrawBoxBaseNS(
			0.45f * torso_len,
			0.25f * torso_len,
			0.10f * torso_len,
			True
		    );
		}
		glEnd();


		glTranslatef(0.0f, 0.10f * torso_len, 0.0f);

		/* Upper torso. */
		c = &palette[SAR_HUMAN_COLOR_TORSO];
		SET_BODY_COLOR
		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.45f * torso_len,
			0.25f * torso_len,
			0.90f * torso_len,
			False
		    );
		}
		glEnd();
	    }
	    glPopMatrix();
	    /* Back to base of torso. */

	    /* Draw left leg. */
	    glPushMatrix();
	    {
		/* Thigh. */
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(left_hip_to_thigh_angle),
		    1.0f, 0.0f, 0.0f
		);
		glTranslatef(-0.16f * torso_len, 0.0f, 0.0f);

		c = &palette[SAR_HUMAN_COLOR_LEGS];
		SET_BODY_COLOR

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.5f * thigh_len,
			0.5f * thigh_len,
			-thigh_len,
			True
		    );
		}
		glEnd();

		/* Calv. */
		glTranslatef(0.0f, -thigh_len, 0.0f);
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(left_knee_to_calv_angle),
		    1.0f, 0.0f, 0.0f
		);

		/* Use same color as for thighs. */

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.3f * calv_len,
			0.3f * calv_len,
			-calv_len,
			True
		    );
		}
		glEnd();

		/* Feet. */
		glTranslatef(0.0f, -calv_len, -0.05f);

		c = &palette[SAR_HUMAN_COLOR_FEET];
		SET_BODY_COLOR

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.1f,
			0.2f,
			-0.1f,
			True
		    );
		}
		glEnd();
	    }
	    glPopMatrix();
	    /* Back to base of torso. */

	    /* Draw right leg. */
	    glPushMatrix();
	    {
		/* Thigh. */
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(right_hip_to_thigh_angle),
		    1.0f, 0.0f, 0.0f
		);
		glTranslatef(0.16f * torso_len, 0.0f, 0.0f);

		c = &palette[SAR_HUMAN_COLOR_LEGS];
		SET_BODY_COLOR

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.5f * thigh_len,
			0.5f * thigh_len,
			-thigh_len,
			True
		    );  
		}
		glEnd();   
		
		/* Calv. */
		glTranslatef(0.0f, -thigh_len, 0.0f);
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(right_knee_to_calv_angle),
		    1.0f, 0.0f, 0.0f
		);

		/* Use same color as for thighs. */

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.3f * calv_len,
			0.3f * calv_len,
			-calv_len,
			True
		    );
		}
		glEnd();

		/* Feet. */
		glTranslatef(0.0f, -calv_len, -0.05f);

		c = &palette[SAR_HUMAN_COLOR_FEET];
		SET_BODY_COLOR

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.1f,
			0.2f,
			-0.1f,
			True
		    );
		}
		glEnd();
	    }
	    glPopMatrix();
	    /* Back to base of torso. */

	    /* Move to shoulders. */
	    glTranslatef(0.0f, 0.95f * torso_len, 0.0f);

	    /* Draw left arm. */
	    glPushMatrix();
	    {
		/* Bisep and shoulder. */
		glTranslatef(
		    (-0.225f * torso_len) + (-0.25f * bisep_len),
		    0.0f,
		    0.0f
		);
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(left_bisep_angle),
		    1.0f, 0.0f, 0.0f
		);
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(left_shoulder_angle),
		    0.0f, 0.0f, 1.0f
		);

		c = &palette[SAR_HUMAN_COLOR_ARMS];
		SET_BODY_COLOR

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.5f * bisep_len,
			0.5f * bisep_len,
			-bisep_len,
			True
		    );
		}
		glEnd();

		/* Trisep. */
		glTranslatef(0.0f, -bisep_len, 0.0f);
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(left_trisep_angle),
		    1.0f, 0.0f, 0.0f
		);

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.28f * trisep_len,
			0.28f * trisep_len,
			-trisep_len,
			True
		    );
		}
		glEnd();


		/* Hand. */
		glTranslatef(0.0f, -trisep_len, 0.0f);

		c = &palette[SAR_HUMAN_COLOR_HANDS];
		SET_BODY_COLOR

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(0.2f, 0.1f, -0.2f, True);
		}
		glEnd();
	    }
	    glPopMatrix();
	    /* Back to shoulders. */

	    /* Draw right arm. */
	    glPushMatrix();
	    {
		/* Bisep. */
		glTranslatef(
		    (0.225f * torso_len) + (0.25f * bisep_len),
		    0.0f,
		    0.0f
		);
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(right_bisep_angle),
		    1.0f, 0.0f, 0.0f
		);
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(right_shoulder_angle),
		    0.0f, 0.0f, 1.0f
		);

		c = &palette[SAR_HUMAN_COLOR_ARMS];
		SET_BODY_COLOR

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.5f * bisep_len,
			0.5f * bisep_len,
			-bisep_len,
			True
		    );
		}
		glEnd();

		/* Trisep. */
		glTranslatef(0.0f, -bisep_len, 0.0f);
		glRotatef(
		    (GLfloat)-SFMRadiansToDegrees(right_trisep_angle),
		    1.0f, 0.0f, 0.0f
		);

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(
			0.28f * trisep_len,
			0.28f * trisep_len,
			-trisep_len,
			True
		    );
		}
		glEnd();


		/* Hand. */
		glTranslatef(0.0f, -trisep_len, 0.0f);

		c = &palette[SAR_HUMAN_COLOR_HANDS];
		SET_BODY_COLOR

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(0.2f, 0.1f, -0.2f, True);
		}
		glEnd();
	    }
	    glPopMatrix();
	    /* Back to shoulders. */


	    /* Head. */
	    glTranslatef(0.0f, 0.05f * torso_len, 0.0f);

	    c = &palette[SAR_HUMAN_COLOR_FACE];
	    SET_BODY_COLOR

	    glBegin(GL_QUADS);
	    {
		SARDrawBoxBaseNS(0.18f, 0.18f, 0.25f, False);
	    }
	    glEnd();


	    /* Hair. */
	    glPushMatrix();
	    {
		glTranslatef(0.0f, 0.25f, 0.0f);

		c = &palette[SAR_HUMAN_COLOR_HAIR];
		SET_BODY_COLOR

		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(0.18f, 0.18f, 0.1f, False);
		}
		glEnd();
	    }
	    glPopMatrix();
	    /* Back of head hair. */
	    glPushMatrix();
	    {
		glTranslatef(0.0f, 0.05f, -(-0.09f - 0.03f));
		glBegin(GL_QUADS);
		{
		    SARDrawBoxBaseNS(0.18f, 0.06f, 0.25f, True);
		}
		glEnd();
	    }
	    glPopMatrix();


	    /* Check if on streatcher, if so we need to pop one matrix
	     * level that was added for translation up on to streatcher.
	     */
	    if(flags & SAR_HUMAN_FLAG_ON_STREATCHER)
	    {
		glPopMatrix();
	    }
	}
	glPopMatrix();

	/* Re-enable GL_TEXTURE_2D as needed. */
	if(texture_2d_state)
	    StateGLEnable(state, GL_TEXTURE_2D);

	/* Draw water ripples if in water. */
	if(flags & SAR_HUMAN_FLAG_IN_WATER)
	{
	    GLboolean depth_mask_flag;
	    int tex_num, frame_num;
	    v3d_texture_ref_struct *t;
	    float hr = 5.0;	/* Half ripple radius. */


	    tex_num = water_ripple_tex_num;
	    if(SARIsTextureAllocated(scene, tex_num) &&
	       opt->textured_objects
	    )
	    {
		t = scene->texture_ref[tex_num];

		/* Select frame by animation, re-update anim_coeff. */
		anim_coeff = (float)anim_pos /
		    (float)((sar_grad_anim_t)-1);

		frame_num = (int)(anim_coeff * t->total_frames);
		if(frame_num >= t->total_frames)
		    frame_num = t->total_frames - 1;
		if(frame_num < 0)
		    frame_num = 0;
		V3DTextureSelectFrame(t, frame_num);

		glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

		StateGLEnable(state, GL_BLEND);
		StateGLDisable(state, GL_ALPHA_TEST);
		depth_mask_flag = state->depth_mask_flag;
		StateGLDepthMask(state, GL_FALSE);
		StateGLEnable(state, GL_POLYGON_OFFSET_FILL);
		StateGLBlendFunc(
		    state, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
		);
		StateGLPolygonOffset(
		    state,
		    (GLfloat)opt->gl_polygon_offset_factor, -1.0f
		);

		glBegin(GL_QUADS);
		{
		    glNormal3f(0, 1, 0);
		    glTexCoord2d(0, 1 - 0);
		    glVertex3f(-hr, 0, hr);
		    glTexCoord2d(1, 1 - 0);
		    glVertex3f(hr, 0, hr);
		    glTexCoord2d(1, 1 - 1);
		    glVertex3f(hr, 0, -hr);
		    glTexCoord2d(0, 1 - 1);
		    glVertex3f(-hr, 0, -hr);
		}
		glEnd();

		StateGLDisable(state, GL_POLYGON_OFFSET_FILL);
		StateGLDisable(state, GL_BLEND);
		StateGLDepthMask(state, depth_mask_flag);
		StateGLEnable(state, GL_ALPHA_TEST);
	    }
	}

	/* Restore GL states. */
	if(lighting)
	    StateGLEnable(state, GL_LIGHTING);
	StateGLShadeModel(state, shade_model_mode);

#undef SET_BODY_COLOR
}


/*
 *      Draws a human specified by the object obj_ptr.
 *
 *      The human substructure human should be the one on
 *      the object.
 *
 *      All inputs are assumed valid.
 */
void SARDrawHuman(
	sar_dc_struct *dc, sar_object_struct *obj_ptr,
	sar_object_human_struct *human
)
{
	/* Draw main human model for this call first, passing the
	 * given human object structure's values directly.
	 */
	SARDrawHumanIterate(
	    dc,
	    human->height, human->mass,
	    human->flags, human->color,
	    human->water_ripple_tex_num,
	    human->anim_pos
	);

	/* Draw assisting humans if any and as appropriate.
	 * Do not draw assisting humans if this human is being gripped
	 * (ie in a rescue basket).
	 */
	if((human->assisting_humans > 0) &&
	   !(human->flags & SAR_HUMAN_FLAG_GRIPPED)
	)
	{
	    int i;
	    sar_obj_flags_t flags;
	    float s = 0.8f;	/* Left/right spacing. */

	    /* Draw all assisting human objects, left and right. */
	    for(i = 0; i < human->assisting_humans; i++)
	    {
		glPushMatrix();
		{
		    /* Translate to the side and a bit behind. */
		    glTranslatef(s, 0.0f, 0.2f);

		    /* Set up `filtered' human flags that will be
		     * passed to the human draw itteration function.
		     * This is so that we don't pass all the flags, 
		     * because in some situations the human may be
		     * lying on a streatcher and the assisting humans
		     * need to be running.
		     */
		    flags = 0;
		    if(human->flags & SAR_HUMAN_FLAG_ALERT)
		        flags |= SAR_HUMAN_FLAG_ALERT;
		    if(human->flags & SAR_HUMAN_FLAG_AWARE)
			flags |= SAR_HUMAN_FLAG_AWARE;
		    if(human->flags & SAR_HUMAN_FLAG_IN_WATER)
			flags |= SAR_HUMAN_FLAG_IN_WATER;
		    if(human->flags & SAR_HUMAN_FLAG_RUN)
			flags |= SAR_HUMAN_FLAG_RUN;
		    if(human->flags & SAR_HUMAN_FLAG_PUSHING)
			flags |= SAR_HUMAN_FLAG_PUSHING;

		    /* Draw our human object as an assisting human,
		     * using the filtered human flags and the 
		     * assisting_human_color color palette
		     */
		    SARDrawHumanIterate(
			dc,
			human->height, human->mass,
			flags, human->assisting_human_color,
			human->water_ripple_tex_num,
			human->anim_pos
		    );
		}
		glPopMatrix();

		/* Shift spacing to other side. */
		s *= -1.0;
	    }
	}
}

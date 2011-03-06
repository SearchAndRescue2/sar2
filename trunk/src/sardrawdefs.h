/**********************************************************************
*   This file is part of Search and Rescue II (SaR2).                 *
*                                                                     *
*   SaR2 is free software: you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License v.2 as       *
*   published by the Free Software Foundation.                        *
*                                                                     *
*   SaR2 is distributed in the hope that it will be useful, but       *
*   WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See          *
*   the GNU General Public License for more details.                  *
*                                                                     *
*   You should have received a copy of the GNU General Public License *
*   along with SaR2.  If not, see <http://www.gnu.org/licenses/>.     *
***********************************************************************/

/*
	Definations for sardraw*.c Modules

	All sardraw*.c modules should #include this.
 */

#ifndef SARDRAWDEFS_H
#define SARDRAWDEFS_H

#define LOG(x)		(((x) > 0.0f) ? log(x) : 0.0f)
#define POW(x,y)	(((x) > 0.0f) ? pow(x,y) : 0.0f)
#define SQRT(x)		(((x) > 0.0f) ? sqrt(x) : 0.0f)

#define MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)	(MIN(MAX((a),(l)),(h)))
#define STRLEN(s)	(((s) != NULL) ? ((int)strlen(s)) : 0)

#define RADTODEG(r)	((r) * 180.0 / PI)
#define DEGTORAD(d)	((d) * PI / 180.0)


/* Get camera orientation back from GL (can be True or False)?
 * Note: This supposedly slows GL down dramatically if True
 */
#define GET_CAM_DIR	False

/* Turns on GL_DEPTH_TEST and sets the DepthFunc */
#define SAR_DRAW_DEPTH_TEST_ON	{			\
 state_gl_struct *state = &display->state_gl;		\
 StateGLEnable(state, GL_DEPTH_TEST);			\
 StateGLDepthFunc(state, GL_LEQUAL);			\
}
/* Turns off GL_DEPTH_TEST and sets the DepthFunc */
#define SAR_DRAW_DEPTH_TEST_OFF	{			\
 state_gl_struct *state = &display->state_gl;		\
 StateGLDisable(state, GL_DEPTH_TEST);			\
 StateGLDepthFunc(state, GL_ALWAYS);			\
}

/* Turns on GL_TEXTURE_1D and sets the TexEnv */
#define SAR_DRAW_TEXTURE_1D_ON	{			\
 state_gl_struct *state = &display->state_gl;		\
 StateGLEnable(state, GL_TEXTURE_1D);			\
 StateGLTexEnvI(					\
  state,						\
  GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE	\
 );							\
}
/* Turns off GL_TEXTURE_1D */
#define SAR_DRAW_TEXTURE_1D_OFF	{			\
 StateGLDisable(&display->state_gl, GL_TEXTURE_1D);	\
}

/* Turns on GL_TEXTURE_2D and sets the TexEnv */
#define SAR_DRAW_TEXTURE_2D_ON	{			\
 state_gl_struct *state = &display->state_gl;		\
 StateGLEnable(state, GL_TEXTURE_2D);			\
 StateGLTexEnvI(					\
  state,						\
  GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE	\
 );							\
}
/* Turns off GL_TEXTURE_2D */
#define SAR_DRAW_TEXTURE_2D_OFF	{			\
 StateGLDisable(&display->state_gl, GL_TEXTURE_2D);	\
}

/* Force disables certain GL states which might have been enabled
 * during a call to SARVisualModelCallList()
 */
#define SAR_DRAW_POST_CALLLIST_RESET_STATES	{	\
 state_gl_struct *state = &display->state_gl;		\
 StateGLEnableF(state, GL_ALPHA_TEST, GL_TRUE);		\
 StateGLDisableF(state, GL_BLEND, GL_TRUE);		\
 StateGLPointSize(state, 1.0f);				\
}


/* Sets the color mask to force FLIR color tinting */
#define SAR_DRAW_FLIR_COLOR_MASK_SET	{		\
 glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);	\
}
#define SAR_DRAW_COLOR_MASK_UNSET	{		\
 glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);	\
}



#endif	/* SARDRAWDEFS_H */

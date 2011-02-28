/*
                        Control Panel Values
 */

#ifndef CPVALUES_H
#define CPVALUES_H

#include <sys/types.h>
#include <GL/gl.h>


/*
 *      Color states:
 */
#define CP_COLOR_STATE_NORMAL   0       /* Day */
#define CP_COLOR_STATE_DIM      1       /* Dusk/dawn with no lights */
#define CP_COLOR_STATE_DARK     2       /* Night with no lights */
#define CP_COLOR_STATE_LIGHTED  3       /* Dusk/dawn/night with lights on */

#define CP_COLOR_STATES         4


/*
 *      Control Panel color structure:
 */
typedef struct {

        GLfloat r, g, b;

} CPColor;
#define CPCOLOR(p)      ((CPColor *)(p))

/*
 *	Control Panel rectangle structure:
 */
typedef struct {

	GLfloat x, y, width, height;

} CPRectangle;
#define CPRECTANGLE(p)	((CPRectangle *)(p))


/*
 *	Control Panel Values structure:
 *
 *	Contains current values of the aircraft.
 */
typedef struct _ControlPanelValues ControlPanelValues;
struct _ControlPanelValues {

	time_t	current_time;	/* Current time in milliseconds */
	float	time_compensation;	/* Coefficient 0.0 to 1000.0 */
	GLfloat	tod;		/* Current time of day in seconds */

#define CP_TOD_CODE_DAY		1
#define CP_TOD_CODE_DAWN	2
#define CP_TOD_CODE_NIGHT	3
#define CP_TOD_CODE_DUSK	4
	int	tod_code;

	/* If GL_TRUE then draw control panel full screen */
	GLboolean	fullscreen;

	int	color_state;	/* One of CP_COLOR_STATE_* */

	/* Velocity & speed (in miles per hour) */
	GLfloat vel_x, vel_y, vel_z, speed;

	/* Attitude */
	GLfloat	heading, pitch, bank;

	/* Altitude (in meters and feet) */
	GLfloat	alt, alt_feet;

	GLfloat	gear_coeff;	/* 0.0 is up and 1.0 is down */

};

#endif	/* CPVALUES_H */

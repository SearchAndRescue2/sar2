#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>

#ifdef __MSW__
# include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#include "../include/string.h"

#include "gw.h"
#include "stategl.h"
#include "messages.h"
#include "sar.h"
#include "objutils.h"
#include "sardraw.h"
#include "sardrawdefs.h"
#include "config.h"


void SARDrawHelp(sar_dc_struct *dc);
void SARDrawMessages(sar_dc_struct *dc);
void SARDrawBanner(sar_dc_struct *dc);
void SARDrawCameraRefTitle(sar_dc_struct *dc);
static void SARDrawSlewCoordinates(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
);
static void SARDrawControlIcons(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
);
void SARDrawControlMessages(sar_dc_struct *dc);


/*
 *      Draws help message.
 *
 *      Will also check if current display_help page number exceeds the
 *      total number of drawable pages, if it does then display_help
 *      will be set back to 0 and nothing will be drawn.
 */
void SARDrawHelp(sar_dc_struct *dc)
{
	sar_core_struct *core_ptr = dc->core_ptr;
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	const sar_option_struct *opt = dc->option;
	const int width = dc->width, height = dc->height;
	const char *cstrptr;
	int line_num, xc, yc, x0, y0, xmax, ymax, fw, fh;
	GWFont *font = opt->message_font;
	const sar_color_struct *color = &opt->message_color;
	char num_str[80];
	const char *mesg[] = SAR_KEYS_HELP_MESSAGES;
	int     mesg_lines, mesg_strings,
		mesg_lines_drawn = 0,
		page_num, total_pages,
		mesg_lines_per_page,
		mesg_line_max = SAR_KEYS_HELP_MESSAGE_LINE_MAX;

	if(font == NULL)
	    return;

	GWGetFontSize(font, NULL, NULL, &fw, &fh);
	if((fw <= 0) || (fh <= 0))
	    return;

	/* Get the current help page number, which corresponds to
	 * display_help that starting from index 1 (not 0, since 0
	 * means do not display help).
	 */
	page_num = core_ptr->display_help;

	/* Calculate total lines in help message list. */
	mesg_strings = (int)(sizeof(mesg) / sizeof(char *));
	/* Must be in pairs. */
	if((mesg_strings % 2) != 0)
	    return;
	if(mesg_strings < 2)
	    return;
	/* Calculate total lines. */
	mesg_lines = mesg_strings / 2;

	/* Initialize starting x and y position in window
	 * coordinates.
	 */
	xc = x0 = 10;
	yc = y0 = 5;

	/* Calculate maximum bounds in window coordinates. */
	xmax = mesg_line_max * fw;
	ymax = height - fh - 5;

	/* Calculate lines per page and total pages, remember to excude
	 * two lines for the heading.
	 */
	mesg_lines_per_page = MAX(ymax - yc - (fh * 2), 0) / fh;
	total_pages = (int)ceil(
	    (float)mesg_lines / (float)mesg_lines_per_page
	);


	/* Check if current display_help page number exceeds the
	 * total number of drawable pages, if it does then display_help
	 * will be set back to 0 and nothing will be drawn.
	 */
	if(page_num > total_pages)
	{
	    core_ptr->display_help = 0;
	    return;
	}


	/* Draw shaded background. */
	StateGLEnable(state, GL_BLEND);
	StateGLBlendFunc(
	    state, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
	);
	glBegin(GL_QUADS);
	{
	    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
	    glVertex2i(0, 0);
	    glVertex2i(width, 0);
	    glVertex2i(width, height);
	    glVertex2i(0, height);
	}
	glEnd();
	StateGLDisable(state, GL_BLEND);


	/* Set font. */
	GWSetFont(display, font);

	/* Draw heading. */
#define DO_DRAW_STRING                  {               \
glColor4f(color->r, color->g, color->b, color->a);      \
GWDrawString(display, xc, yc, cstrptr);                 \
xc += (int)(STRLEN(cstrptr) * fw);                      \
}
#define DO_DRAW_STRING_HIGHLIGHT        {               \
glColor4f(color->r, color->g, 0.0f, color->a);          \
GWDrawString(display, xc, yc, cstrptr);                 \
xc += (int)(STRLEN(cstrptr) * fw);                      \
}

	cstrptr = SAR_MESG_HELP_PAGE_HEADING;
	DO_DRAW_STRING
	xc += fw;

	sprintf(num_str, "%i", page_num);
	cstrptr = num_str;
	DO_DRAW_STRING_HIGHLIGHT

	cstrptr = "/";
	DO_DRAW_STRING

	sprintf(num_str, "%i", total_pages);
	cstrptr = num_str;
	DO_DRAW_STRING
	xc += 3 * fw;

	cstrptr = "(";
	DO_DRAW_STRING

	cstrptr = "F1";
	DO_DRAW_STRING_HIGHLIGHT

	cstrptr = " = Next | ";
	DO_DRAW_STRING

	cstrptr = "ESC";
	DO_DRAW_STRING_HIGHLIGHT

	cstrptr = " = Exit)";
	DO_DRAW_STRING

#undef DO_DRAW_STRING_HIGHLIGHT
#undef DO_DRAW_STRING


	/* Increment two additional lines so we have one blank line. */
	yc += (fh * 2);
	mesg_lines_drawn += 2;

	/* Set line_num to starting message line number. */
	line_num = mesg_lines_per_page * (page_num - 1);
	if(line_num >= 0)
	{
	    /* Begin drawing help message lines, remember that
	     * each line comes in 2 strings.
	     */
	    while((yc < ymax) && (line_num < mesg_lines))
	    {
		/* Seek x position back to initial. */
		xc = x0;

		/* Draw command. */
		cstrptr = mesg[(line_num * 2) + 0];
		if(cstrptr != NULL)
		{
		    glColor4f(color->r, color->g, 0.0f, color->a);
		    GWDrawString(
			display, xc, yc,
			cstrptr
		    );
		    xc += (STRLEN(cstrptr) * fw) + (2 * fw);
		}

		/* Draw description. */
		cstrptr = mesg[(line_num * 2) + 1];
		if(cstrptr != NULL)
		{
		    glColor4f(color->r, color->g, color->b, color->a);
		    GWDrawString(
			display, xc, yc,
			cstrptr
		    );
		}

		yc += fh;
		line_num++;
	    }
	}
}


/*
 *      Draws the standard messages defined (if any) on the given scene
 *      structure.
 *
 *      Standard messages are only drawn if message_display_until is
 *      positive.
 */
void SARDrawMessages(sar_dc_struct *dc)
{
	gw_display_struct *display = dc->display;
	state_gl_struct *state = &display->state_gl;
	const sar_option_struct *opt = dc->option;
	sar_scene_struct *scene = dc->scene;
	const int width = dc->width, height = dc->height;
	const int half_width = width / 2;
	GWFont *font = opt->message_font;
	int i, fw, fh, offset_x, offset_y, bg_width, bg_height;
	int max_chars_per_line, lines_to_draw, lines_drawn;

	/* Coordinates are handled in left hand rule xy plane,
	 * the y axis is inverted at the call to the draw
	 * function
	 */

	/* Not currently displaying messages? */
	if(scene->message_display_until <= 0l)
	    return;

	/* Time to stop displaying messages? */
	if(scene->message_display_until < cur_millitime)
	{
	    /* Reset message display until time to 0 and delete all
	     * messages on the scene
	     *
	     * This will not change the size of the message pointer
	     * array
	     */
	    SARMessageClearAll(scene);
	    return;
	}

	GWSetFont(display, font);
	GWGetFontSize(font, NULL, NULL, &fw, &fh);
	offset_x = 0;
	offset_y = (int)(height * 0.2f);

	/* Calculate the maximum number of characters visible on one
	 * line due to the width of the GL context and minus 2
	 * characters to allow for margin
	 */
	max_chars_per_line = (fw > 0) ? ((width / fw) - 2) : 0;
	if(max_chars_per_line <= 0)
	    return;

	/* Count number of non-NULL messages available, plus extra
	 * lines for cases where the width of the message is too long
	 * and needs to be carried to subsequent line(s)
	 */
	lines_to_draw = 0;
	for(i = scene->total_messages - 1; i >= 0; i--)
	{
	    int len = STRLEN(scene->message[i]);
	    while(len > 0)
	    {
		lines_to_draw++;
		len -= max_chars_per_line;
	    }
	}
/* This has problems with Y positioning
	if(lines_to_draw > scene->total_messages)
	    lines_to_draw = scene->total_messages;
 */

	/* Calculate background size */
	bg_width = width;	/* Background encompasses entire width */
	bg_height = (fh * lines_to_draw);

	/* Draw darkened background */
	StateGLEnable(state, GL_BLEND);
	StateGLBlendFunc(
	    state, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
	);
	glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
	glBegin(GL_QUADS);
	{
	    glVertex2d(0, offset_y);
	    glVertex2d(bg_width, offset_y);
	    glVertex2d(bg_width, (offset_y + bg_height));
	    glVertex2d(0, (offset_y + bg_height));
	}
	glEnd();
	StateGLDisable(state, GL_BLEND);

	/* Begin drawing messages, first message is boldest color */
	lines_drawn = 0;
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	offset_y += fh;
	for(i = scene->total_messages - 1; i >= 0; i--)
	{
	    const char *s = scene->message[i];
	    int	n, max_chars_cur_line,
		mesg_len_left = STRLEN(s),
		extra_lines = MAX((mesg_len_left - 1) / max_chars_per_line, 0);

	    /* Move Y position back to draw extra lines */
	    offset_y += fh * extra_lines;

	    /* Draw all characters for this message, use multiple lines
	     * as needed until mesg_len_left is no longer positive or
	     * lines_drawn exceeds the total number of lines to be
	     * drawn
	     */
	    while(mesg_len_left > 0)
	    {
		/* Calculate the maximum characters for current line */
		max_chars_cur_line = MIN(mesg_len_left, max_chars_per_line);

		/* Calculate X position of text */
		offset_x = (int)(
		    half_width - (((extra_lines > 0) ?
			max_chars_per_line : max_chars_cur_line) * fw / 2)
		);

		/* Met or exceeded the number of lines to draw? */
		if(lines_drawn < lines_to_draw)
		{
		    /* Draw all characters that will fit on this line */
		    for(n = 0; n < max_chars_cur_line; n++)
		    {
			GWDrawCharacter(
			    display,
			    offset_x,
			    height - offset_y,
			    *s
			);
			s++;		/* Go to next character in message */
			offset_x += fw;	/* Move X position to next character */
		    }
		    lines_drawn++;
		}

		/* Count number of characters drawn */
		mesg_len_left -= max_chars_per_line;

		/* Move Y position to previous (not next) line if more
		 * lines are needed to draw this message
		 */
		if(mesg_len_left > 0)
		    offset_y -= fh;
	    }

	    /* Move Y position to next line */
	    offset_y += fh * (1 + extra_lines);

	    /* Darken text color for drawing subsequent line(s) */
	    glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
	}
}

/*
 *      Draws sticky banner messages.
 *
 *      Sticky banner messages only drawn if the sticky_banner_message
 *      message list contains data (not NULL).
 */
void SARDrawBanner(sar_dc_struct *dc)
{
	gw_display_struct *display = dc->display;
	sar_scene_struct *scene = dc->scene;
	const sar_option_struct *opt = dc->option;
	const int width = dc->width, height = dc->height;
	const int half_width = width / 2;
	GWFont *font = opt->banner_font;
	int i, fw, fh, offset_x, offset_y;
	const char *s;

	/* Coordinates are handled in left hand rule xy plane,
	 * the y axis is inverted at the call to the draw
	 * function.
	 */

	/* No banner messages? */
	if((scene->sticky_banner_message == NULL) ||
	   (scene->total_sticky_banner_messages < 1)
	)
	    return;

	GWSetFont(display, font);
	GWGetFontSize(font, NULL, NULL, &fw, &fh);
	offset_x = 0;
	offset_y = (int)(height * 0.8f);

	/* Begin drawing messages, first message is boldest color. */
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	/* Draw first line. */
	offset_y += fh;
	i = 0;
	while(i < MIN(scene->total_sticky_banner_messages, 1))
	{
	    s = scene->sticky_banner_message[i];
	    if(s == NULL)
		break;

	    offset_x = (int)(half_width - (STRLEN(s) * fw / 2));

	    GWDrawString(
		display,
		offset_x,
		height - offset_y,
		s
	    );

	    i++;
	    offset_y -= fh;
	}

	/* Give some spacing between first line and smaller subsequent
	 * lines.
	 */
	offset_y -= 5;

	/* Begin drawing subsequent lines (using different font). */
	font = opt->message_font;
	GWSetFont(display, font);
	GWGetFontSize(font, NULL, NULL, &fw, &fh);
	while(i < scene->total_sticky_banner_messages)
	{
	    s = scene->sticky_banner_message[i];
	    if(s == NULL)
		break;

	    offset_x = (int)(half_width - (STRLEN(s) * fw / 2));

	    GWDrawString(
		display,
		offset_x,
		height - offset_y,
		s
	    );

	    i++;
	    offset_y -= fh;
	}
}

/*
 *      Draws camera reference title.
 *
 *      Camera reference title is only drawn if
 *      camera_ref_title_display_until is positive.
 */
void SARDrawCameraRefTitle(sar_dc_struct *dc)
{
	gw_display_struct *display = dc->display;
	sar_scene_struct *scene = dc->scene;
	const sar_option_struct *opt = dc->option;
	GWFont *font = opt->message_font;
	const int margin_x = 5, margin_y = 5;
	const char *s = scene->camera_ref_title;

	/* Coordinates are handled in left hand rule xy plane,
	 * the y axis is inverted at the call to the draw
	 * function.
	 */

	/* Not currently displaying camera reference title? */
	if((scene->camera_ref_title_display_until <= 0) ||
	   (scene->camera_ref_title == NULL)
	)
	    return;

	/* Time to stop displaying? */
	if(scene->camera_ref_title_display_until < cur_millitime)
	{
	    /* Clear title. */
	    SARCameraRefTitleSet(scene, NULL);
	    return;
	}

	if(s != NULL)
	{
	    GWSetFont(display, font);
	    if(dc->flir)
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    else
		SARDrawSetColor(&opt->message_color);
	    GWDrawString(
		display,
		margin_x,
		margin_y,
		s
	    );
	}
}

/*
 *      Draws slew coordinates of the given object.
 *
 *      The call to this function is responsible for checking if the 
 *      given object is in slew mode.
 *
 *      The font and color should already be set prior this this call.
 *
 *      Inputs assumed valid.
 */
static void SARDrawSlewCoordinates(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
)
{
	gw_display_struct *display = dc->display;
	sar_scene_struct *scene = dc->scene;
	const sar_position_struct *pos = &obj_ptr->pos;
	const sar_direction_struct *dir = &obj_ptr->dir;
	const int margin_x = 5, margin_y = 5;
	float dms_x, dms_y;
	char text[256];

	if(False)
	{
	    SFMMToDMS(
		pos->x, pos->y,
		scene->planet_radius,
		scene->dms_x_offset, scene->dms_y_offset,
		&dms_x, &dms_y
	    );
	    sprintf(
		text,
		"%s %s  Alt: %.0f",
		SFMLongitudeToString(dms_x),
		SFMLatitudeToString(dms_y),
		SFMMetersToFeet(pos->z)
	    );
	    GWDrawString(
		display,
		margin_x,
		margin_y,
		text
	    );
	}
	else
	{
	    sprintf(
		text,
 "%i:%2i Player: (hpb) %.0f %.0f %.0f  (xyz) %.2f %.2f %.2f(%.2f feet)\n",
		(int)(scene->tod / 3600),
		(int)((int)((int)scene->tod / 60) % 60),
		SFMRadiansToDegrees(dir->heading),
		SFMRadiansToDegrees(dir->pitch),
		SFMRadiansToDegrees(dir->bank),
		pos->x, pos->y, pos->z,
		SFMMetersToFeet(pos->z)
	    );
	    GWDrawString(
		display,
		margin_x,
		margin_y,
		text
	    );
	}
}


/*
 *	Draws control icons, such as FLIR, door open (user set).
 */
static void SARDrawControlIcons(
	sar_dc_struct *dc, sar_object_struct *obj_ptr
)
{
/*
	gw_display_struct *display = dc->display;
	sar_scene_struct *scene = dc->scene;
 */
	const sar_option_struct *opt = dc->option;
	const int flight_model = dc->player_flight_model_type;
/*	sar_object_aircraft_struct *obj_aircraft_ptr = 
		SAR_OBJ_GET_AIRCRAFT(obj_ptr);
 */
	const sar_obj_part_struct *part;
	int x, y;
	const int width = dc->width, height = dc->height;
	const int icon_width = 15, icon_height = 15;
	static u_int8_t flir_bm[] = {
0x00, 0x00, 0x00, 0x00, 0x60, 0x0c, 0x60, 0x0c, 0x78, 0x3c, 0x7c, 0x7c,
0xff, 0xfe, 0x80, 0x02, 0x80, 0x02, 0xff, 0xfe, 0x7e, 0xfc, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	static u_int8_t slew_bm[] = {
0x01, 0x00, 0x03, 0x80, 0x07, 0xc0, 0x01, 0x00, 0x01, 0x00, 0x23, 0x88,
0x67, 0xcc, 0xff, 0xfe, 0x67, 0xcc, 0x23, 0x88, 0x01, 0x00, 0x01, 0x00,
0x07, 0xc0, 0x03, 0x80, 0x01, 0x00
	};
	static u_int8_t rescue_door_user_opened_bm[] = {
0x00, 0x00, 0xfe, 0x00, 0xff, 0xf8, 0xfe, 0x08, 0xfa, 0x08, 0xfa, 0x48,
0xfe, 0xc8, 0xc7, 0xf8, 0xc6, 0xc8, 0xc6, 0x48, 0xfe, 0x08, 0xfe, 0x08,
0x07, 0xf8, 0x00, 0x00, 0x00, 0x00
	};

	if(dc->flir)
	    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	else
	    SARDrawSetColor(&opt->message_color);

	/* Set starting raster position */
	x = 15;
	y = 30;

/* Draws the given bitmap data p and increments the x and y
 * position.
 */
#define RASTER_BITMAP(p)			\
{ if(((p) != NULL) && (x < width)) {		\
 glRasterPos2i(x, height - (y + icon_height));	\
 glBitmap(					\
  icon_width, icon_height,			\
  0.0f, 0.0f,					\
  (GLfloat)icon_width, 0.0f,			\
  (p)						\
 );						\
 x += icon_width + 5;				\
} }
	/* FLIR */
	if(dc->flir)
	{ RASTER_BITMAP(flir_bm); }

	/* Slew */
	if(flight_model == SAR_FLIGHT_MODEL_SLEW)
	{ RASTER_BITMAP(slew_bm); }

	/* Rescue door user opened? */
	part = SARObjGetPartPtr(
	    obj_ptr, SAR_OBJ_PART_TYPE_DOOR_RESCUE, 0
	);
	if((part != NULL) ? (part->flags & SAR_OBJ_PART_FLAG_STATE) : False)
	{
	    if(part->flags & SAR_OBJ_PART_FLAG_DOOR_STAY_OPEN)
	    { RASTER_BITMAP(rescue_door_user_opened_bm); }
	}



#undef RASTER_BITMAP
}

/*
 *	Draws control messages, such as wheel brakes, overspeed, and
 *	time compression.
 */
void SARDrawControlMessages(sar_dc_struct *dc)
{
	gw_display_struct *display = dc->display;
	sar_scene_struct *scene = dc->scene;
	const gctl_struct *gc = dc->gctl;
	const sar_option_struct *opt = dc->option;
	const int width = dc->width, height = dc->height;
	GWFont *font = opt->message_font;
	sar_object_struct *obj_ptr = scene->player_obj_ptr;
	const int flight_model = dc->player_flight_model_type;
	const int margin_x = 5, margin_y = 5;
	int fw, fh;


	GWGetFontSize(
	    font,
	    NULL, NULL,
	    &fw, &fh
	);
	GWSetFont(display, font);

	/* Slew coordinates (if player is in slew mode) */
	if((flight_model == SAR_FLIGHT_MODEL_SLEW) &&
	   (obj_ptr != NULL)
	)
	{
	    if(dc->flir)
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	    else
		SARDrawSetColor(&opt->message_color);
	    SARDrawSlewCoordinates(dc, obj_ptr);
	}

	/* Player wheel brakes (if on) or autopilot (if otherwise) */
	switch(dc->player_wheel_brakes)
	{
	  case 2:
	    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	    GWDrawString(
		display,
		margin_x,
		height - margin_y - fh,
		SAR_MESG_PARKING_BRAKES
	    );
	    break;

	  case 1:
	    glColor4f(
		(GLfloat)MAX(
		    (gc != NULL) ? gc->wheel_brakes_coeff : 1.0, 0.0
		),
		0.0f, 0.0f, 1.0f
	    );
	    GWDrawString(
		display,
		margin_x,
		height - margin_y - fh,
		SAR_MESG_WHEEL_BRAKES
	    );
	    break;

	  default:
	    if(dc->player_autopilot)
	    {
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		GWDrawString(
		    display,
		    margin_x,
		    height - margin_y - fh,
		    (flight_model == SAR_FLIGHT_MODEL_AIRPLANE) ?
			SAR_MESG_AUTOPILOT :
			(flight_model == SAR_FLIGHT_MODEL_HELICOPTER) ?
			    SAR_MESG_AUTOHOVER : ""
		);
	    }
	    break;
	}

	/* Stall? */
	if(dc->player_stall)
	{
	    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	    GWDrawString(
		display,
		width - margin_x - (fw * STRLEN(SAR_MESG_STALL)),
		height - margin_y - fh,
		SAR_MESG_STALL
	    );
	}
	/* Overspeed? */
	else if(dc->player_overspeed)
	{
	    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	    GWDrawString(
		display,
		width - margin_x - (fw * STRLEN(SAR_MESG_OVERSPEED)),
		height - margin_y - fh,
		SAR_MESG_OVERSPEED
	    );
	}

	/* Time compression */
	if(time_compression != 1.0f)
	{
	    char text[256];
	    sprintf(
		text, "%s: %.0f%%",
		(time_compression > 1.0f) ?
		    SAR_MESG_TIME_ACCELERATION : SAR_MESG_SLOW_MOTION,
		time_compression * 100.0f
	    );
	    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	    GWDrawString(
		display,
		width - margin_x - (fw * STRLEN(text)),
		margin_y,
		text
	    );
	}

	/* Control icons */
	if(obj_ptr != NULL)
	    SARDrawControlIcons(dc, obj_ptr);
}

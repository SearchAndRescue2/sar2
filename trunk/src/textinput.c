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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __MSW__
# include <windows.h>
#endif

#include <GL/gl.h>

#include "../include/string.h"
#include "gw.h"
#include "stategl.h"
#include "textinput.h"


static void SARTextInputHistoryRecord(
	text_input_struct *p, const char *value
);

text_input_struct *SARTextInputNew(
	gw_display_struct *display,
	GWFont *font
);
void SARTextInputDelete(text_input_struct *p);

static void SARTextInputCharInsert(text_input_struct *p, char c);
static void SARTextInputCharBackspace(text_input_struct *p);
static void SARTextInputCharDelete(text_input_struct *p);
void SARTextInputHandleKey(
	text_input_struct *p, int k, Boolean state
);
void SARTextInputHandlePointer(
	text_input_struct *p,
	int x, int y, gw_event_type type, int btn_num
);

Boolean SARTextInputIsMapped(text_input_struct *p);
void SARTextInputMap(
	text_input_struct *p,
	const char *label, const char *value,
	void (*func_cb)(const char *, void *),
	void *data
);
void SARTextInputUnmap(text_input_struct *p);
void SARTextInputDraw(text_input_struct *p);


#define ATOI(s)         (((s) != NULL) ? atoi(s) : 0)
#define ATOL(s)         (((s) != NULL) ? atol(s) : 0)
#define ATOF(s)         (((s) != NULL) ? atof(s) : 0.0f)
#define STRDUP(s)       (((s) != NULL) ? strdup(s) : NULL)

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define CLIP(a,l,h)     (MIN(MAX((a),(l)),(h)))
#define STRLEN(s)       (((s) != NULL) ? ((int)strlen(s)) : 0)


/*
 *	Records the given value on to the prompt's list of history
 *	values.
 */
static void SARTextInputHistoryRecord(
	text_input_struct *p, const char *value
)
{
	int i, max = 5;

	if((p == NULL) || (value == NULL))
	    return;

	if(p->total_history < max)
	{
	    /* Allocate one more pointer */
	    i = MAX(p->total_history, 0);
	    p->total_history = i + 1;
	    p->history = (char **)realloc(
		p->history, p->total_history * sizeof(char *)
	    );
	    if(p->history == NULL)
	    {
		p->total_history = 0;
		return;
	    }

	    /* Shift pointers */
	    for(; i >= 1; i--)
		p->history[i] = p->history[i - 1];

	    /* Add new value */
	    i = 0;
	    if(i < p->total_history)
		p->history[i] = STRDUP(value);
	}
	else
	{
	    /* Delete oldest (highest) index value */
	    i = p->total_history - 1;
	    if(i >= 0)
		free(p->history[i]);

	    /* Shift pointers */
	    for(; i >= 1; i--)
		p->history[i] = p->history[i - 1];

	    /* Add new value */
	    i = 0;
	    if(i < p->total_history)
		p->history[i] = STRDUP(value);
	}
}



/*
 *	Create a new text input prompt.
 */
text_input_struct *SARTextInputNew(
	gw_display_struct *display,
	GWFont *font
)
{
	text_input_struct *p = TEXT_INPUT(calloc(
	    1, sizeof(text_input_struct)
	));
	if(p == NULL)
	    return(NULL);

	p->display = display;
	p->font = font;

	p->label = NULL;
	p->buf = NULL;
	p->len = 0;
	p->pos = 0;

	p->last_value_x = 0;
	p->last_value_y = 0;

	p->data = NULL;
	p->func_cb = NULL;

	p->history = NULL;
	p->total_history = 0;
	p->last_history_num = -1;

	return(p);
}

/*
 *	Deletes the text input prompt.
 */
void SARTextInputDelete(text_input_struct *p)
{
	int i;

	if(p == NULL)
	    return;

	for(i = 0; i < p->total_history; i++)
	    free(p->history[i]);
	free(p->history);

	free(p->label);
	free(p->buf);
	free(p);
}

/*
 *	Insert character.
 */
static void SARTextInputCharInsert(text_input_struct *p, char c)
{
	int i, n;
	char *buf;
	if(!isprint(c))
	    return;

	/* Increase buffer allocation */
	i = MAX(p->len, 0);
	p->len = i + 1;
	p->buf = buf = (char *)realloc(
	    p->buf, (p->len + 1) * sizeof(char)
	);
	if(buf == NULL)
	{
	    p->len = p->pos = 0;
	    return;
	}

	/* Sanitize position */
	if(p->pos >= p->len)
	    p->pos = p->len - 1;
	if(p->pos < 0)
	    p->pos = 0;

	/* Shift buffer at position */
	for(i = p->len - 1, n = p->pos; i > n; i--)
	    buf[i] = buf[i - 1];
	buf[p->len] = '\0';

	/* Insert new character */
	buf[p->pos] = c;

	/* Increment position */
	p->pos++;
}

/*
 *	Backspace.
 */
static void SARTextInputCharBackspace(text_input_struct *p)
{
	int i;
	char *buf = p->buf;
	if((buf == NULL) || (p->pos <= 0))
	    return;

	/* Reduce buffer one character left of current position */
	for(i = p->pos - 1; i < p->len; i++)
	    buf[i] = buf[i + 1];

	/* Move position one character to the left */
	p->pos--;

	/* Reduce buffer allocation */
	p->len--;
	if(p->len < 0)
	    p->len = 0;
	p->buf = (char *)realloc(
	    p->buf, (p->len + 1) * sizeof(char)
	);
	if(p->buf == NULL)
	{
	    p->len = p->pos = 0;
	    return;
	}
}

/*
 *	Delete.
 */
static void SARTextInputCharDelete(text_input_struct *p)
{
	int i;
	char *buf = p->buf;
	if((buf == NULL) || (p->pos >= p->len))
	    return;

	/* Reduce buffer at current position */
	for(i = MAX(p->pos, 0); i < p->len; i++)
	    buf[i] = buf[i + 1];

	/* Reduce buffer allocation */
	p->len--;
	if(p->len < 0)
	    p->len = 0;
	p->buf = (char *)realloc(
	    p->buf, (p->len + 1) * sizeof(char)
	);
	if(p->buf == NULL)
	{
	    p->len = p->pos = 0;
	    return;
	}
}

/*
 *	Handles key event if prompt is mapped.
 */
void SARTextInputHandleKey(
	text_input_struct *p, int k, Boolean state
)
{
	int events_handled = 0;
	Boolean ctrl, shift;
	gw_display_struct *display = (p != NULL) ? p->display : NULL;
	if(!SARTextInputIsMapped(p) ||
	   (display == NULL) || !state
	)
	    return;

	/* Get modifier keys */
	ctrl = display->ctrl_key_state;
	shift = display->shift_key_state;

	/* Handle key */
	switch(k)
	{
	  case 0x1b:		/* Escape */
	    /* Unmap prompt, delete value, and unset callback
	     * function.
	     */
	    if(p->func_cb != NULL)
	    {
		void (*func_cb)(const char *, void *) = p->func_cb;
		void *data = p->data;

		/* Unmap prompt, delete value, and unset
		 * callback function.
		 */
		SARTextInputUnmap(p);

		/* Call function callback */
		func_cb(NULL, data);
	    }
	    events_handled++;
	    break;

	  case GWKeyUp:
	    if(p->history != NULL)
	    {
		int i = MAX(p->last_history_num + 1, 0);
		if((i >= 0) && (i < p->total_history))
		{
		    free(p->buf);
		    p->buf = STRDUP(p->history[i]);
		    p->len = STRLEN(p->buf);
		    p->pos = p->len;
		    p->last_history_num = i;
		}		
	    }
	    events_handled++;
	    break;

	  case GWKeyDown:
	    if(p->history != NULL)
	    {
		int i = p->last_history_num - 1;
		if((i >= 0) && (i < p->total_history))
		{
		    free(p->buf);
		    p->buf = STRDUP(p->history[i]);
		    p->len = STRLEN(p->buf);
		    p->pos = p->len;
		    p->last_history_num = i;
		}
	    }
	    events_handled++;
	    break;

	  case GWKeyLeft:
	    /* Move position to the left */
	    p->pos--;
	    if(p->pos < 0)
		p->pos = 0;
	    events_handled++;
	    break;

	  case GWKeyRight:
	    /* Move position to the right */
	    p->pos++;
	    if(p->pos > p->len)
		p->pos = p->len;
	    events_handled++;
	    break;

	  case GWKeyHome:
	    /* Move position to beginning */
	    p->pos = 0;
	    events_handled++;
	    break;

	  case GWKeyEnd:
	    /* Move position to end (past the value's last character */
	    p->pos = p->len;
	    events_handled++;
	    break;

	  case GWKeyBackSpace:
	    /* Backspace */
	    SARTextInputCharBackspace(p);
	    events_handled++;
	    break;

	  case GWKeyDelete:
	    /* Delete (or backspace if shift modifier key is held) */
	    if(shift)
		SARTextInputCharBackspace(p);
	    else
		SARTextInputCharDelete(p);
	    events_handled++;
	    break;

	  case '\n':		/* Enter */
	    /* Call callback function and then unmap prompt */
	    if(p->func_cb != NULL)
	    {
		/* We need to make a copy of the value and record
		 * the function pointer since both values on the
		 * prompt structure p will be deleted during the
		 * prompt unmap (which occures first) before the
		 * function callback is called.
		 */
		void (*func_cb)(const char *, void *) = p->func_cb;
		char *value = STRDUP(p->buf);
		void *data = p->data;

		/* Record this history value */
		SARTextInputHistoryRecord(p, value);

		/* Reset last recalled history value index number */
		p->last_history_num = -1;

		/* Unmap prompt, delete value, and unset
		 * callback function.
		 */
		SARTextInputUnmap(p);

		/* Call function callback using the coppied value */
		func_cb(value, data);

		/* Deallocate value */
		free(value);

		events_handled++;
	    }
	    break;

	  default:
	    /* Insert a character */
	    SARTextInputCharInsert(
		p, (char)(shift ? toupper(k) : k)
	    );
	    events_handled++;
	    break;
	}
}

/*
 *	Handles pointer event if prompt is mapped.
 */
void SARTextInputHandlePointer(
	text_input_struct *p,
	int x, int y, gw_event_type type, int btn_num
)
{
	int events_handled = 0;
	int fw, fh;
	gw_display_struct *display = (p != NULL) ? p->display : NULL;
	GWFont *font = (p != NULL) ? p->font : NULL;
	if(!SARTextInputIsMapped(p) || (display == NULL))
	    return;

	GWGetFontSize(font, NULL, NULL, &fw, &fh);

	/* Pointer event in y axis bounds? */
	if((events_handled == 0) &&
	   (y >= p->last_value_y) &&
	   (y < (p->last_value_y + fh))
	)
	{
	    /* Handle by event type */
	    switch(type)
	    {
	      case GWEventTypeButtonPress:
	      case GWEventTypeButtonRelease:
		if(btn_num == 1)
		{
		    int pos = (int)((fw > 0) ?
			((x - p->last_value_x) / fw) : 0
		    );
		    if(pos > p->len)
			pos = p->len;
		    if(pos < 0)
			pos = 0;
		    p->pos = pos;
		    events_handled++;
		}
		break;
	      case GWEventTypePointerMotion:
	      case GWEventType2ButtonPress:
	      case GWEventType3ButtonPress:
		break;
	    }
	}
}


/*
 *	Chefcks if the text input prompt is mapped by checking if its
 *	function callback is set.
 */
Boolean SARTextInputIsMapped(text_input_struct *p)
{
	return((p != NULL) ? (p->func_cb != NULL) : False);
}

/*
 *	Maps the text input prompt and sets it's label, value, and
 *	function callback.
 */
void SARTextInputMap(
	text_input_struct *p,
	const char *label, const char *value,
	void (*func_cb)(const char *, void *),
	void *data
)
{
	if(p == NULL)
	    return;

	free(p->label);
	p->label = STRDUP(label);

	if(value != NULL)
	{
	    free(p->buf);
	    p->buf = STRDUP(value);
	    p->len = p->pos = STRLEN(p->buf);
	}
	else
	{
	    free(p->buf);
	    p->buf = NULL;
	    p->len = p->pos = 0;
	}

	p->func_cb = func_cb;
	p->data = data;
}

/*
 *	Unmaps the text input prompt, deleting the value and unsetting
 *	the function callback.
 */
void SARTextInputUnmap(text_input_struct *p)
{
	if(p == NULL)
	    return;

	free(p->label);
	p->label = NULL;
	free(p->buf);
	p->buf = NULL;
	p->len = 0;
	p->pos = 0;

	p->data = NULL;
	p->func_cb = NULL;
}       

/*
 *	Draws text input prompt.
 */
void SARTextInputDraw(text_input_struct *p)
{
	const char *label, *buf;
	int label_len, buf_len, pos, len;
	int x0, y0, xc, yc, fw, fh;
	int width, height;
	StateGLBoolean alpha_test;
	gw_display_struct *display = (p != NULL) ? p->display : NULL;
	GWFont *font = (p != NULL) ? p->font : NULL;
	if(!SARTextInputIsMapped(p) ||
	   (display == NULL) || (font == NULL)
	)
	    return;

	GWContextGet(
	    display, GWContextCurrent(display),
	    NULL, NULL,
	    NULL, NULL,
	    &width, &height
	);
	if((width <= 0) || (height <= 0))
	    return;

	GWGetFontSize(font, NULL, NULL, &fw, &fh);

	pos = p->pos;

	label = p->label;
	buf = p->buf;

	label_len = STRLEN(label);
	buf_len = STRLEN(buf);


	/* Calculat4 starting coordinates, use length of buffer plus
	 * tolorence and additional characters (7 more chars).
	 */
	len = (label_len + buf_len + 7) * fw;
	if(len > width)
	    x0 = width - len;
	else
	    x0 = (width / 2) - (len / 2);
	y0 = (int)(0.5 * height);

	xc = x0;
	yc = y0;

	/* Draw shaded background */
	alpha_test = display->state_gl.alpha_test;
	StateGLDisable(&display->state_gl, GL_ALPHA_TEST);
	StateGLEnable(&display->state_gl, GL_BLEND);
	StateGLBlendFunc(
	    &display->state_gl,
	    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA
	);
	glBegin(GL_QUADS);
	{
	    glColor4d(0.0, 0.0, 0.0, 0.5);
	    glVertex2d(0, yc - fh - 2);
	    glVertex2d(width, yc - fh - 2);
	    glVertex2d(width, yc + 2);
	    glVertex2d(0, yc + 2);
	}
	glEnd();
	StateGLDisable(&display->state_gl, GL_BLEND);
	if(alpha_test)
	    StateGLEnable(&display->state_gl, GL_ALPHA_TEST);


	/* Set text color and font */
	glColor4d(1.0, 1.0, 1.0, 1.0);
	GWSetFont(display, font);

	/* Draw label */
	GWDrawString(display, xc, yc, label);
	xc += (label_len * fw);

	/* Draw deliminator */
	GWDrawString(display, xc, yc, ": ");
	xc += (2 * fw);

	/* At this poin we need to record the position of the start
	 * of the value (upper left corner in window coordinates)
	 * for use by other functions.
	 */
	p->last_value_x = xc;
	p->last_value_y = yc;

	/* Draw value and cursor */
	GWDrawString(display, xc, yc, buf);
	GWDrawString(display, xc + (pos * fw), yc, "_");
}

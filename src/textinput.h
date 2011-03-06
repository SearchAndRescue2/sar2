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
			   Text Input Prompt
 */

#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "gw.h"

typedef struct {

	gw_display_struct	*display;
	GWFont			*font;

	char	*label;
	char	*buf;
	int	pos,
		len;		/* Number of characters in buffer not
				 * counting last null character */

	/* Position of the start (upper left corner) of last value
	 * text position in window coordinates
	 *
	 * This is updated each time SARTextInputDraw() is called
	 */
	int last_value_x, last_value_y;

	void *data;
	void (*func_cb)(const char *, void *);

	char	**history;
	int	total_history, last_history_num;

} text_input_struct;
#define TEXT_INPUT(p)	((text_input_struct *)(p))


extern text_input_struct *SARTextInputNew(
	gw_display_struct *display,
	GWFont *font
);
extern void SARTextInputDelete(text_input_struct *p);

extern void SARTextInputHandleKey(
	text_input_struct *p, int k, Boolean state
);
extern void SARTextInputHandlePointer(
	text_input_struct *p,
	int x, int y, gw_event_type type, int btn_num
);

extern Boolean SARTextInputIsMapped(text_input_struct *p);
extern void SARTextInputMap(
	text_input_struct *p,
	const char *label, const char *value,
	void (*func_cb)(const char *, void *),
	void *data
);
extern void SARTextInputUnmap(text_input_struct *p);
extern void SARTextInputDraw(text_input_struct *p);

#endif	/* TEXTINPUT_H */

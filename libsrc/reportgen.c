/*
 *  Copyright (C) 2003 SICOM Systems, INC.
 *
 *  Authors: Bob Doan <bdoan@sicompos.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 * 
 * This module generates a report from the information stored in the current
 * report object.
 * The main entry point is called once at report generation time for each
 * report defined in the rlib object.
 *
 */
 
#include <stdlib.h>
#include <string.h>
#include <langinfo.h>

#include "config.h"
#include "rlib.h"
#include "pcode.h"
#include "rlib_input.h"

#define FONTPOINT 	10.0
#define RLIB_GET_LINE(a) ((float)(a/RLIB_PDF_DPI))

//Not used: static struct rlib_rgb COLOR_BLACK = {0, 0, 0};

static gchar *orientations[] = {
	"",
	"portrait",
	"landscape",
	NULL
};

static gchar *aligns[] = {
	"left",
	"right",
	"center",
	NULL
};

static gchar *truefalses[] = {
	"no",
	"yes",
	"false",
	"true",
	NULL
};

static struct rlib_paper paper[] = {
	{RLIB_PAPER_LETTER,612, 792, "LETTER"},
	{RLIB_PAPER_LEGAL, 612, 1008, "LEGAL"},
	{RLIB_PAPER_A4, 595, 842, "A4"},
	{RLIB_PAPER_B5, 499, 708, "B5"},
	{RLIB_PAPER_C5, 459, 649, "C5"},
	{RLIB_PAPER_DL, 312, 624, "DL"},
	{RLIB_PAPER_EXECUTIVE, 522, 756, "EXECUTIVE"},
	{RLIB_PAPER_COMM10, 297, 684, "COMM10"},
	{RLIB_PAPER_MONARCH, 279, 540, "MONARCH"},
	{RLIB_PAPER_FILM35MM, 528, 792, "FILM35MM"},
	{0},
};

struct rlib_paper * rlib_get_paper(rlib *r, gint paper_type) {
	gint i;
	for(i=0;paper[i].type != 0;i++)
		if(paper[i].type == paper_type)
			return &paper[i];
	return NULL;
}

struct rlib_paper * rlib_get_paper_by_name(rlib *r, gchar *paper_name) {
	gint i;
//	r_debug("looking for %s\n", paper_name);
	if(paper_name == NULL)
		return NULL;
		
	for(i=0;paper[i].type != 0;i++)
		if(!strcasecmp(paper[i].name, paper_name))
			return &paper[i];
	return NULL;
}

static gint rlib_execute_as_int(rlib *r, struct rlib_pcode *pcode, gint *result) {
	struct rlib_value val;
	gint isok = FALSE;

	*result = 0;
	if (pcode) {
		rlib_execute_pcode(r, &val, pcode, NULL);
		if (RLIB_VALUE_IS_NUMBER((&val))) {
			*result = RLIB_VALUE_GET_AS_NUMBER((&val)) / RLIB_DECIMAL_PRECISION;
			isok = TRUE;
		} else {
			gchar *whatgot = "don't know";
			gchar *gotval = "";
			if (RLIB_VALUE_IS_STRING((&val))) {
				whatgot = "string";
				gotval = RLIB_VALUE_GET_AS_STRING((&val));
			}
			rlogit("Expecting numeric value from pcode. Got %s=%s", whatgot, gotval);
		}
		rlib_value_free(&val);
	}
	return isok;
}

static gint rlib_execute_as_boolean(rlib *r, struct rlib_pcode *pcode, gint *result) {
	return rlib_execute_as_int(r, pcode, result)? TRUE : FALSE;
}

static gint rlib_execute_as_string(rlib *r, struct rlib_pcode *pcode, gchar *buf, gint buf_len) {
	struct rlib_value val;
	gint isok = FALSE;

	if (pcode) {
		rlib_execute_pcode(r, &val, pcode, NULL);
		if (RLIB_VALUE_IS_STRING((&val))) {
			strncpy(buf, RLIB_VALUE_GET_AS_STRING((&val)), buf_len);
			isok = TRUE;
		} else {
			rlogit("Expecting string value from pcode");
		}
		rlib_value_free(&val);
	}
	return isok;
}

static gint rlib_execute_as_int_inlist(rlib *r, struct rlib_pcode *pcode, gint *result, gchar *list[]) {
	struct rlib_value val;
	gint isok = FALSE;

	*result = 0;
	if (pcode) {
		rlib_execute_pcode(r, &val, pcode, NULL);
		if (RLIB_VALUE_IS_NUMBER((&val))) {
			*result = RLIB_VALUE_GET_AS_NUMBER((&val)) / RLIB_DECIMAL_PRECISION;
			isok = TRUE;
		} else if (RLIB_VALUE_IS_STRING((&val))) {
			gint i;
			gchar * str = RLIB_VALUE_GET_AS_STRING((&val));
			for (i = 0; list[i]; ++i) {
				if (g_strcasecmp(str, list[i])) {
					*result = i;
					isok = TRUE;
					break;
				}
			}
		} else {
			rlogit("Expecting number or specific string from pcode");
		}
		rlib_value_free(&val);
	}
	return isok;
}

static gfloat rlib_layout_get_next_line(rlib *r, struct rlib_part *part, gfloat position, gfloat foint_point) {
	if(part->landscape)
		return ((part->paper->width/RLIB_PDF_DPI) - (position + RLIB_GET_LINE(foint_point)));
	else
		return ((part->paper->height/RLIB_PDF_DPI) - (position + RLIB_GET_LINE(foint_point)));
}

static void rlib_advance_line(rlib *r, gfloat *rlib_position, gint font_point) {
	*rlib_position += RLIB_GET_LINE(font_point);
}

static gfloat rlib_get_estimated_width(rlib *r, gint len) {
	gchar buf[2];
	buf[0] = 'W';
	buf[1] = '\0';
	return (OUTPUT(r)->get_string_width(r, buf)*len);
}

static gfloat advance_margin(rlib *r, gfloat margin, gint chars) {
	gchar buf[2];
	buf[0] = 'W';
	buf[1] = '\0';
	margin += (OUTPUT(r)->get_string_width(r, buf)*chars);
	return margin;
}

#define STATE_NONE		0
#define STATE_BGCOLOR	1

#define STATUS_NONE	0
#define STATUS_START 1
#define STATUS_STOP	2

static gfloat rlib_output_extras_start(rlib *r, struct rlib_part *part, gint backwards, gfloat left_origin, gfloat bottom_orgin, 
struct rlib_line_extra_data *extra_data) {
	if(extra_data->found_link)
		OUTPUT(r)->boxurl_start(r, part, left_origin, bottom_orgin, rlib_get_estimated_width(r, extra_data->width), 
			RLIB_GET_LINE(extra_data->font_point), extra_data->link);

	if(extra_data->running_bgcolor_status & STATUS_START) 
		OUTPUT(r)->draw_cell_background_start(r, left_origin, bottom_orgin, extra_data->running_running_bg_total, 
			RLIB_GET_LINE(extra_data->font_point), &extra_data->bgcolor);

	return extra_data->output_width;
}

static gfloat rlib_output_extras_end(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_orgin, 
struct rlib_line_extra_data *extra_data) {
	if(extra_data->running_bgcolor_status & STATUS_STOP)
		OUTPUT(r)->draw_cell_background_end(r);	

	if(extra_data->found_link)
		OUTPUT(r)->boxurl_end(r);
		
	return extra_data->output_width;
}


static gfloat rlib_output_extras(rlib *r, struct rlib_part *part, gint backwards, gfloat left_origin, gfloat bottom_orgin, 
struct rlib_line_extra_data *extra_data) {
	if(extra_data->found_link)
		OUTPUT(r)->boxurl_start(r, part, left_origin, bottom_orgin, rlib_get_estimated_width(r, extra_data->width), 
			RLIB_GET_LINE(extra_data->font_point), extra_data->link);

	if(extra_data->running_bgcolor_status & STATUS_START) 
		OUTPUT(r)->draw_cell_background_start(r, left_origin, bottom_orgin, extra_data->running_running_bg_total, 
			RLIB_GET_LINE(extra_data->font_point), &extra_data->bgcolor);

	if(extra_data->running_bgcolor_status & STATUS_STOP)
		OUTPUT(r)->draw_cell_background_end(r);	

	if(extra_data->found_link)
		OUTPUT(r)->boxurl_end(r);
		
	return extra_data->output_width;
}
	

/*
 * Convert UTF8 to the desired character encoding, IF specified in rlib object.
 */
static const gchar *encode_text(rlib *r, const gchar *text) {
	gchar *result = "";

	if (text == NULL) {
		r_error("encode_text called with NULL text");
		result = "!ERR_ENC1";
	} else {
		result = (gchar *) rlib_char_encoder_encode(r->current_output_encoder, text);
		if (result == NULL) {
			r_error("encode returned NULL result input was[%s], len=%d", text, strlen(text));
			result = "!ERR_ENC2";
		}
	}
	return result;
}


static gfloat rlib_output_text(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_orgin, struct rlib_line_extra_data *extra_data) {
	gfloat rtn_width;
	gchar *text = extra_data->formatted_string;

	OUTPUT(r)->set_font_point(r, extra_data->font_point);
	if(extra_data->found_color) {
		OUTPUT(r)->set_fg_color(r, extra_data->color.r, extra_data->color.g, extra_data->color.b);
	}
	OUTPUT(r)->print_text(r, left_origin, bottom_orgin+(extra_data->font_point/300.0), (gchar *) encode_text(r, text), backwards, extra_data->col);
	rtn_width = extra_data->output_width;
	if(extra_data->found_color)
		OUTPUT(r)->set_fg_color(r, 0, 0, 0);
	OUTPUT(r)->set_font_point(r, r->font_point);
	return rtn_width;
}


static gfloat rlib_output_text_text(rlib *r, gint backwards, gfloat left_origin, gfloat bottom_orgin, struct rlib_line_extra_data *extra_data, 
gchar *text) {
	gfloat rtn_width;
	OUTPUT(r)->set_font_point(r, extra_data->font_point);
	if(extra_data->found_color)
		OUTPUT(r)->set_fg_color(r, extra_data->color.r, extra_data->color.g, extra_data->color.b);
//TODO: The cast is because it's too much trouble to change the prototype for the rlib_print_text
// but rlib_print_text should have param 4 'const gchar *'.
	OUTPUT(r)->print_text(r, left_origin, bottom_orgin+(extra_data->font_point/300.0), (gchar *) encode_text(r, text), backwards, extra_data->col);
	rtn_width = extra_data->output_width;
	if(extra_data->found_color)
		OUTPUT(r)->set_fg_color(r, 0, 0, 0);
	OUTPUT(r)->set_font_point(r, r->font_point);
	return rtn_width;
}



gchar *align_text(rlib *r, gchar *rtn, gint len, gchar *src, gint align, gint width) {
	g_strlcpy(rtn, src, len);

	if(!OUTPUT(r)->do_align)
		return rtn;

	if(align == RLIB_ALIGN_LEFT || width == -1) {
	} else {
		if(align == RLIB_ALIGN_RIGHT) {        
			gint x = width - r_charcount(src);
			if (x > (len - 1)) x = len - 1;
			if(x > 0) {
				memset(rtn, ' ', x);
				g_strlcpy(rtn+x, src, len - x);
			}
		}
		if(align == RLIB_ALIGN_CENTER) {
			gint x = (width - r_charcount(src))/2;
			if (x > (len - 1)) x = len -1;
			if(x > 0) {
				memset(rtn, ' ', x);
				g_strlcpy(rtn+x, src, len - x);
			}
		}
	}
	return rtn;
}
	
static gint get_font_point(rlib *r, struct rlib_report_lines *rl) {
	if(rl->font_point == -1)
		return r->font_point;
	else
		return rl->font_point;
}	

gfloat estimate_string_width_from_extra_data(rlib *r, struct rlib_line_extra_data *extra_data) {
	gfloat rtn_width;
	OUTPUT(r)->set_font_point(r, extra_data->font_point);
	rtn_width = advance_margin(r, 0, extra_data->width);	
	return rtn_width;
}
	

void execute_pcodes_for_line(rlib *r, struct rlib_report_lines *rl, struct rlib_line_extra_data *extra_data) {
	gint i=0;
	gchar *text;
	struct rlib_report_field *rf;
	struct rlib_report_literal *rt;
	struct rlib_element *e = rl->e;
	struct rlib_value line_rval_color;
	struct rlib_value line_rval_bgcolor;

	RLIB_VALUE_TYPE_NONE(&line_rval_color);
	RLIB_VALUE_TYPE_NONE(&line_rval_bgcolor);
	if(rl->color_code != NULL)
		rlib_execute_pcode(r, &line_rval_color, rl->color_code, NULL);
	if(rl->bgcolor_code != NULL)
		rlib_execute_pcode(r, &line_rval_bgcolor, rl->bgcolor_code, NULL);
	for(; e != NULL; e=e->next) {
		RLIB_VALUE_TYPE_NONE(&extra_data[i].rval_bgcolor);
		if (e->type == RLIB_ELEMENT_FIELD) {
			gchar buf[MAXSTRLEN];
			rf = e->data;
			if (rf == NULL) r_error("report_field is NULL ... will crash");
			else if (rf->code == NULL) r_error("There is no code for field");
			rlib_execute_pcode(r, &extra_data[i].rval_code, rf->code, NULL);	
			if(rf->link_code != NULL) {	
				rlib_execute_pcode(r, &extra_data[i].rval_link, rf->link_code, NULL);
			}
			if(rf->color_code != NULL) {
				rlib_execute_pcode(r, &extra_data[i].rval_color, rf->color_code, NULL);
			} else if(rl->color_code != NULL) {
				extra_data[i].rval_color = line_rval_color;
				rlib_value_dup_contents(&extra_data[i].rval_color);
			}
			if(rf->bgcolor_code != NULL) {
				rlib_execute_pcode(r, &extra_data[i].rval_bgcolor, rf->bgcolor_code, NULL);
			} else if(rl->bgcolor_code != NULL) {
				extra_data[i].rval_bgcolor = line_rval_bgcolor;
				rlib_value_dup_contents(&extra_data[i].rval_bgcolor);
			}
			if (rf->align_code) {
				gint t;
				if (rlib_execute_as_int_inlist(r, rf->align_code, &t, aligns)) {
					if (t < 3) rf->align = t;
				}
			}
			if (rf->width_code) {
				gint t;
			if (rlib_execute_as_int(r, rf->width_code, &t)) { 
					rf->width = t;
				}
			}
			rlib_format_string(r, rf, &extra_data[i].rval_code, buf);
			align_text(r, extra_data[i].formatted_string, MAXSTRLEN, buf, rf->align, rf->width);
			
			extra_data[i].width = rf->width;
			
			rlib_execute_pcode(r, &extra_data[i].rval_col, rf->col_code, NULL);
		} else if(e->type == RLIB_ELEMENT_LITERAL) {
			gchar buf[MAXSTRLEN];
			rt = e->data;
			if(rt->color_code != NULL)	
				rlib_execute_pcode(r, &extra_data[i].rval_color, rt->color_code, NULL);
			else if(rl->color_code != NULL) {
				extra_data[i].rval_color = line_rval_color;
				rlib_value_dup_contents(&extra_data[i].rval_color);
			}
			if(rt->bgcolor_code != NULL)
				rlib_execute_pcode(r, &extra_data[i].rval_bgcolor, rt->bgcolor_code, NULL);
			else if(rl->bgcolor_code != NULL) {
				extra_data[i].rval_bgcolor = line_rval_bgcolor;
				rlib_value_dup_contents(&extra_data[i].rval_bgcolor);				
			}

			if(rt->value == NULL)
				extra_data[i].formatted_string[0] = '\0';
			else
				strcpy(extra_data[i].formatted_string, rt->value);
				
			if (rt->align_code) {
				gint t;
				if (rlib_execute_as_int_inlist(r, rt->align_code, &t, aligns)) {
					if (t < 3) rt->align = t;
				}
			}
			if (rt->width_code) {
				gint t;
				if (rlib_execute_as_int(r, rt->width_code, &t)) {
					rt->width = t;
				}
			}
			strcpy(buf, extra_data[i].formatted_string);
			align_text(r, extra_data[i].formatted_string, MAXSTRLEN, buf, rt->align, rt->width);
				
			extra_data[i].width = rt->width;
			rlib_execute_pcode(r, &extra_data[i].rval_col, rt->col_code, NULL);	
		} else {
			r_error("Line has invalid content");
		}
		if(rl->font_point == -1)
			extra_data[i].font_point = r->font_point;
		else
			extra_data[i].font_point = rl->font_point;
		text = extra_data[i].formatted_string;

		if(text == NULL)
			text = "";
		if(extra_data[i].width == -1)
			extra_data[i].width = r_charcount(text);
		else {
			gint slen = r_charcount(text);
			if(slen > extra_data[i].width)
				*r_ptrfromindex(text, extra_data[i].width) = '\0';
			else if(slen < extra_data[i].width && MAXSTRLEN != slen) {
				gint lim = extra_data[i].width - slen;
				gchar *ptr = r_ptrfromindex(text, slen);
				while (lim-- > 0) {
					*ptr++ = ' ';
				}
				*ptr = '\0';
			}
		}
		extra_data[i].found_bgcolor = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_bgcolor))) {
			gchar *colorstring, *ocolor;
			if(!RLIB_VALUE_IS_STRING((&extra_data[i].rval_bgcolor))) {
				rlogit("RLIB ENCOUNTERED AN ERROR PROCESSING THE BGCOLOR FOR THIS VALUE [%s].. BGCOLOR VALUE WAS NOT OF TYPE STRING\n", text);
			} else {
				gchar *idx;
				ocolor = colorstring = g_strdup(RLIB_VALUE_GET_AS_STRING((&extra_data[i].rval_bgcolor)));
				if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_code)) && RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_code)) && strchr(colorstring, ':')) {
					colorstring = g_strdup(colorstring);
					idx = strchr(colorstring, ':');
					if(RLIB_VALUE_GET_AS_NUMBER((&extra_data[i].rval_code)) >= 0)
						idx[0] = '\0';
					else
						colorstring = idx+1;	
				}
				parsecolor(&extra_data[i].bgcolor, colorstring);
				extra_data[i].found_bgcolor = TRUE;
				g_free(ocolor);
			}

		}
		extra_data[i].found_color = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_color))) {
			gchar *colorstring, *ocolor;
			if(!RLIB_VALUE_IS_STRING((&extra_data[i].rval_color))) {
				rlogit("RLIB ENCOUNTERED AN ERROR PROCESSING THE COLOR FOR THIS VALUE [%s].. COLOR VALUE WAS NOT OF TYPE STRING\n", text);
			} else {
				gchar *idx;
				ocolor = colorstring = g_strdup(RLIB_VALUE_GET_AS_STRING((&extra_data[i].rval_color)));
				if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_code)) && RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_code)) && strchr(colorstring, ':')) {
					idx = strchr(colorstring, ':');
					if(RLIB_VALUE_GET_AS_NUMBER((&extra_data[i].rval_code)) >= 0)
						idx[0] = '\0';
					else
						colorstring = idx+1;	
				}
				parsecolor(&extra_data[i].color, colorstring);
				extra_data[i].found_color = TRUE;
				g_free(ocolor);
			}
		}
		extra_data[i].col = 0;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_col))) {
			if(!RLIB_VALUE_IS_NUMBER((&extra_data[i].rval_col))) {
				rlogit("RLIB EXPECTS A expN FOR A COLUMN... text=[%s]\n", text);
			} else {
				extra_data[i].col = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER((&extra_data[i].rval_col)));
			}
		}
		extra_data[i].found_link = FALSE;
		if(!RLIB_VALUE_IS_NONE((&extra_data[i].rval_link))) {
			if(!RLIB_VALUE_IS_STRING((&extra_data[i].rval_link))) {
				rlogit("RLIB ENCOUNTERED AN ERROR PROCESSING THE LINK FOR THIS VALUE [%s].. LINK VALUE WAS NOT OF TYPE STRING [%d]\n", text, RLIB_VALUE_GET_TYPE((&extra_data[i].rval_link)));
			} else {
				extra_data[i].link = RLIB_VALUE_GET_AS_STRING((&extra_data[i].rval_link));
				if(extra_data[i].link != NULL && strcmp(extra_data[i].link, "")) {
					extra_data[i].found_link = TRUE;
				}
			}
		}
		extra_data[i].output_width = estimate_string_width_from_extra_data(r, &extra_data[i]);		
		i++;
	}
	rlib_value_free(&line_rval_color);
	rlib_value_free(&line_rval_bgcolor);
}	

void find_stuff_in_common(rlib *r, struct rlib_line_extra_data *extra_data, gint count) {
	gint i = 0;
	struct rlib_line_extra_data *e_ptr = NULL, *save_ptr = NULL, *previous_ptr = NULL;
	gint state = STATE_NONE;
	
	previous_ptr = &extra_data[i];
	for(i=0;i<count;i++) {
		e_ptr = &extra_data[i];
		if(e_ptr->found_bgcolor) {
			if(state == STATE_NONE) {
				save_ptr = e_ptr;
				state = STATE_BGCOLOR;
				e_ptr->running_bgcolor_status |= STATUS_START;
				e_ptr->running_running_bg_total = e_ptr->output_width;
			} else {
				if(!memcmp(&save_ptr->bgcolor, &e_ptr->bgcolor, sizeof(struct rlib_rgb))) {
					save_ptr->running_running_bg_total += e_ptr->output_width;
				} else {
					save_ptr = e_ptr;
					previous_ptr->running_bgcolor_status |= STATUS_STOP;
					e_ptr->running_bgcolor_status |= STATUS_START;
					e_ptr->running_running_bg_total = e_ptr->output_width;
				}
			}
		} else {
			if(state == STATE_BGCOLOR) {
				previous_ptr->running_bgcolor_status |= STATUS_STOP;
				state = STATE_NONE;
			}
		}
		previous_ptr = e_ptr;
	}
	if(state == STATE_BGCOLOR) {
 		e_ptr->running_bgcolor_status |= STATUS_STOP;
	}
}


static gint rlib_check_is_not_suppressed(rlib *r, struct rlib_pcode *code) {
	gint result = FALSE;
	if (rlib_execute_as_int_inlist(r, code, &result, truefalses))
		result &= 1;
	return result? FALSE : TRUE;
}


#if 0
/**
 * break the string txt into a RVector of individual strings that will fit
 * the width. Use RVector_size to count the # of lines returned.
 */
RVector *wrap_memo_lines(gchar *txt, gint width, const gchar *wrapchars) {
	gint len;
	gchar *tptr, *endptr, *ptr;
	RVector *v = RVector_new();
	
	do {
		if (r_charcount(txt) < width) {
			RVector_add(v, g_strdup(txt));
			break;
		} else {
			endptr = ptr = txt + width;
			while (ptr > txt) {
				if ((tptr = strchr(wrapchars, *ptr))) {
					len = ptr - txt;
					tptr = g_malloc(len + 1);
					strncpy(tptr, txt, len);
					tptr[len] = '\0';
					RVector_add(v, tptr);
					endptr = ptr;
				}
				--ptr;
			}
			txt = endptr;
		}
	} while (TRUE);
	return v;
}


/**
 * Frees all memory allocated for a memo lines vector
 */
void free_memo_lines(RVector *v) {
	gint i, lim = RVector_size(v);

	for (i = 0; i < lim; ++i) {
		g_free(RVector_get(v, i));
	}
	RVector_free(v);
}
#endif


gint calc_memo_lines(struct rlib_report_lines *rl) {
	struct rlib_element *e;
//	int hasmemo;
	gint nlines = 0;
//	RVector *v;
	
	for (e = rl->e; e != NULL; e = e->next) {
		if (e->type == RLIB_ELEMENT_FIELD) {
			//
//			if (e->xml_maxlines)
//			v = wrapMemoLines(
//			if (e->maxlines != 1) {
//				hasmemo = TRUE;
//			}
		}
	}
	return nlines;
}

static gfloat rlib_layout_get_report_width(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	if(report == NULL) {
		if(part->landscape)
			return (part->paper->height/RLIB_PDF_DPI) - (part->left_margin*2);
		else
			return (part->paper->width/RLIB_PDF_DPI) - (part->left_margin*2);
	} else {
		return report->page_width;
	}
}


static gint rlib_layout_report_output_array(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_report_output_array *roa, gint backwards, gint page) {
	struct rlib_element *e=NULL;
	gint j=0;
	gfloat margin=0, width=0;
	gfloat *rlib_position;
	struct rlib_line_extra_data *extra_data;
	gint output_count = 0;
	gfloat my_left_margin;

	if(roa == NULL)
		return 0;
	
	r->current_line_number++;
	
	if(report != NULL) {
		if(backwards)
			rlib_position = &report->position_bottom[page-1];
		else
			rlib_position = &report->position_top[page-1];	
		my_left_margin = report->left_margin;
	} else {
		if(backwards)
			rlib_position = &part->position_bottom[page-1];
		else
			rlib_position = &part->position_top[page-1];
		my_left_margin = part->left_margin;
	}

	for(j=0;j<roa->count;j++) {
		struct rlib_report_output *ro = roa->data[j];
		margin = my_left_margin;

		if(ro->type == REPORT_PRESENTATION_DATA_LINE) {
			struct rlib_report_lines *rl = ro->data;
			gint count=0;
			
			
			if(rlib_check_is_not_suppressed(r, rl->suppress_code)) {
				output_count++;
				OUTPUT(r)->start_line(r, backwards);

				for(e = rl->e; e != NULL; e=e->next)
					count++;

				extra_data = g_new0(struct rlib_line_extra_data, count);
				execute_pcodes_for_line(r, rl, extra_data);
				find_stuff_in_common(r, extra_data, count);
				count = 0;

				if(OUTPUT(r)->do_grouptext) {
					gchar buf[MAXSTRLEN];
					gfloat fun_width=0;
					gint start_count=-1;
					for(e = rl->e; e != NULL; e=e->next) {
						if(e->type == RLIB_ELEMENT_FIELD) {
							struct rlib_report_field *rf = ((struct rlib_report_field *)e->data);
							rf->rval = &extra_data[count].rval_code;
							width = rlib_output_extras(r, part, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)),  
								&extra_data[count]);
						}

						if(e->type == RLIB_ELEMENT_LITERAL) {
							width = rlib_output_extras(r, part, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);
						}
						margin += width;
						count++;
					}
					count=0;
					margin = my_left_margin;				
					buf[0] = 0;
					width = 0;
					start_count = -1;
					for(e = rl->e; e != NULL; e=e->next) {
						if(!extra_data[count].found_color) {
							if(start_count == -1)
								start_count = count;
							sprintf(buf, "%s%s", buf, extra_data[count].formatted_string);
							fun_width += extra_data[count].output_width;
						} else {
							if(start_count != -1) {
								rlib_output_text_text(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)),  
									&extra_data[start_count], buf);
								start_count = -1;
								margin += fun_width;
								fun_width = 0;
								buf[0] = 0;
							}
							width = rlib_output_text(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)),  
								&extra_data[count]);
							margin += width;

						}
						count++;					
					}
					if(start_count != -1) {
						width += fun_width;
						rlib_output_text_text(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)), 
							&extra_data[start_count], buf);
					}
				} else {
					for(e = rl->e; e != NULL; e=e->next) {
						if(e->type == RLIB_ELEMENT_FIELD) {
							struct rlib_report_field *rf = ((struct rlib_report_field *)e->data);
							rf->rval = &extra_data[count].rval_code;
							rlib_output_extras_start(r, part, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)),  
								&extra_data[count]);
							width = rlib_output_text(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);
							rlib_output_extras_end(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)),  
								&extra_data[count]);
						}

						if(e->type == RLIB_ELEMENT_LITERAL) {
							rlib_output_extras_start(r, part, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);
							width = rlib_output_text(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);				
							rlib_output_extras_end(r, backwards, margin, rlib_layout_get_next_line(r, part, *rlib_position, get_font_point(r, rl)), 
								&extra_data[count]);
						}
						margin += width;
						count++;
					}
				}
				rlib_advance_line(r, rlib_position, get_font_point(r, rl));

				OUTPUT(r)->end_line(r, backwards);	

				count=0;
				for(e = rl->e; e != NULL; e=e->next) {
					rlib_value_free(&extra_data[count].rval_code);
					rlib_value_free(&extra_data[count].rval_link);
					rlib_value_free(&extra_data[count].rval_bgcolor);
					rlib_value_free(&extra_data[count].rval_color);
					rlib_value_free(&extra_data[count].rval_col);
					count++;
				}

				g_free(extra_data);
			}
		} else if(ro->type == REPORT_PRESENTATION_DATA_HR) {
			gchar *colorstring;
			struct rlib_value rval2, *rval=&rval2;
			struct rlib_report_horizontal_line *rhl = ro->data;
			if(rlib_check_is_not_suppressed(r, rhl->suppress_code)) {
				rlib_execute_pcode(r, &rval2, rhl->bgcolor_code, NULL);
				if(!RLIB_VALUE_IS_STRING(rval)) {
					rlogit("RLIB ENCOUNTERED AN ERROR PROCESSING THE BGCOLOR FOR A HR.. COLOR VALUE WAS NOT OF TYPE STRING\n");
				} else {
					gfloat font_point;
					gfloat indent;
					gfloat length;
					struct rlib_rgb bgcolor;
					output_count++;
					colorstring = RLIB_VALUE_GET_AS_STRING(rval);
					parsecolor(&bgcolor, colorstring);
					if(rhl->font_point == -1)
						font_point = r->font_point;
					else
						font_point = rhl->font_point;
					OUTPUT(r)->set_font_point(r, font_point);
					indent = rlib_get_estimated_width(r, rhl->realindent);			
					length = rlib_get_estimated_width(r, rhl->reallength);			
					OUTPUT(r)->set_font_point(r, r->font_point);

					if(length == 0)
						OUTPUT(r)->hr(r, backwards, my_left_margin+indent, rlib_layout_get_next_line(r, part, *rlib_position, 
							rhl->realsize),rlib_layout_get_report_width(r, part, report)-indent, rhl->realsize, &bgcolor, indent, length);
					else
						OUTPUT(r)->hr(r, backwards, my_left_margin+indent, rlib_layout_get_next_line(r, part, *rlib_position, rhl->realsize),
							length, rhl->realsize, &bgcolor, indent, length);

					rlib_advance_line(r, rlib_position, rhl->realsize);
					rlib_value_free(rval);
				}
			}
		} else if(ro->type == REPORT_PRESENTATION_DATA_IMAGE) {
			struct rlib_value rval2, rval3, rval4, rval5, *rval_value=&rval2, *rval_width=&rval3, *rval_height=&rval4, *rval_type=&rval5;
			struct rlib_report_image *ri = ro->data;
			rlib_execute_pcode(r, &rval2, ri->value_code, NULL);
			rlib_execute_pcode(r, &rval5, ri->type_code, NULL);
			rlib_execute_pcode(r, &rval3, ri->width_code, NULL);
			rlib_execute_pcode(r, &rval4, ri->height_code, NULL);
			if(!RLIB_VALUE_IS_STRING(rval_value) || !RLIB_VALUE_IS_NUMBER(rval_width) || !RLIB_VALUE_IS_NUMBER(rval_height)) {
				rlogit("RLIB ENCOUNTERED AN ERROR PROCESSING THE BGCOLOR FOR A IMAGE\n");
			} else {
				gfloat height = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(rval_height));
				gfloat width = RLIB_FXP_TO_NORMAL_LONG_LONG(RLIB_VALUE_GET_AS_NUMBER(rval_width));
				gchar *name = RLIB_VALUE_GET_AS_STRING(rval_value);
				gchar *type = RLIB_VALUE_GET_AS_STRING(rval_type);
				output_count++;
				OUTPUT(r)->drawimage(r, my_left_margin, rlib_layout_get_next_line(r, part, *rlib_position, height), name, 
					type, width, height);
				rlib_value_free(rval_value);
				rlib_value_free(rval_width);
				rlib_value_free(rval_height);
				rlib_value_free(rval_type);
			}
		}
	}
	return output_count;
}	

static gint rlib_layout_report_outputs_accross_pages(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *report_outputs, gint backwards) {
	struct rlib_report_output_array *roa;
	gint page;
	gint i;
	gint output_count = 0;
	for(; report_outputs != NULL; report_outputs=report_outputs->next) {
		roa = report_outputs->data;
		page = roa->page;
		if(page >= 1) {
			OUTPUT(r)->set_working_page(r, part, roa->page);
			output_count += rlib_layout_report_output_array(r, part, report, roa, backwards, roa->page);
		} else {
			for(i=0;i<part->pages_accross;i++) {
				OUTPUT(r)->set_working_page(r, part, i+1);
				output_count = rlib_layout_report_output_array(r, part, report, roa, backwards, i+1);			
			}
		}
	}
	return output_count;
}

gint rlib_layout_report_output(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *e, gint backwards) {
	gint output_count = 0;
	OUTPUT(r)->start_output_section(r);
	output_count = rlib_layout_report_outputs_accross_pages(r, part, report, e, backwards);
	OUTPUT(r)->end_output_section(r);
	return output_count;
}


gint rlib_layout_report_output_with_break_headers(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	struct rlib_element *e;
	gint output_count = 0;
	OUTPUT(r)->start_output_section(r);

	if(report->breaks != NULL) {
		for(e = report->breaks; e != NULL; e=e->next) {
			struct rlib_report_break *rb = e->data;
			output_count += rlib_layout_report_outputs_accross_pages(r, part, report, rb->header, FALSE);
		}		
	}
	rlib_layout_report_outputs_accross_pages(r, part, report, report->detail.fields, FALSE);
	OUTPUT(r)->end_output_section(r);
	return output_count;
}

void rlib_handle_page_footer(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	gint i;

	for(i=0; i < report->pages_accross; i++) {
		report->bottom_size[i] = get_outputs_size(r, report->page_footer, i);
		report->position_bottom[i] -= report->bottom_size[i];
	}

	rlib_layout_report_output(r, part, report, report->page_footer, TRUE);
	
	for(i=0; i<report->pages_accross; i++)
		report->position_bottom[i] -= report->bottom_size[i];
}

/*void rlib_init_page(rlib *r, struct rlib_part *part, struct rlib_report *report, gchar report_header) {
	gint i;
	for(i=0;i<report->pages_accross;i++)
		report->position_top[i] = report->top_margin;
	r->current_font_point = -1;
	OUTPUT(r)->start_new_page(r, part);
	OUTPUT(r)->set_font_point(r, r->font_point);
	if(report_header)
		rlib_layout_report_output(r, part, report, report->report_header, FALSE);	
	
	if(!(r->current_page_number == 1 && report->suppress_page_header_first_page == TRUE))
		rlib_layout_report_output(r, part, report, report->page_header, FALSE);
	rlib_layout_report_output(r, part, report, report->detail.textlines, FALSE);		
	rlib_handle_page_footer(r, part, report);

	OUTPUT(r)->init_end_page(r);
}*/

gfloat get_output_size(rlib *r, struct rlib_report_output_array *roa) {
	gint j;
	gfloat total=0;
	for(j=0;j<roa->count;j++) {
		struct rlib_report_output *rd = roa->data[j];
		if(rd->type == REPORT_PRESENTATION_DATA_LINE) {
			struct rlib_report_lines *rl = rd->data;
			total += RLIB_GET_LINE(get_font_point(r, rl));
			
//Here to adjust size of memo field output.			
		} else if(rd->type == REPORT_PRESENTATION_DATA_HR) {
			struct rlib_report_horizontal_line *rhl = rd->data;
			total += RLIB_GET_LINE(rhl->realsize);		
		}
	}
	return total;
}

gfloat get_outputs_size(rlib *r, struct rlib_element *e, gint page) {
	gfloat total=0;
	struct rlib_report_output_array *roa;

	for(; e != NULL; e=e->next) {
		roa = e->data;
		if(roa->page == -1 || roa->page == page || roa->page == -1)
			total += get_output_size(r, roa);
	}			

	return total;
}


gint will_this_fit(rlib *r, struct rlib_part *part, struct rlib_report *report, gfloat total, gint page) {
	if(OUTPUT(r)->is_single_page(r))
		return TRUE;
//r_error("CHECKING FOR A FIT [%f] > [%f]\n", report->position_top[page-1]+total > report->position_bottom[page-1]);
	if(report->position_top[page-1]+total > report->position_bottom[page-1])
		return FALSE;
	else
		return TRUE;
}

gint will_outputs_fit(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *e, gint page) {
	gfloat size = 0;
	struct rlib_report_output_array *roa;

	if(OUTPUT(r)->is_single_page(r))
		return TRUE;
	if(e == NULL)
		return TRUE;
	for(; e != NULL; e=e->next) {
		roa = e->data;
		if(page == -1 || page == roa->page || roa->page == -1)
			size += get_output_size(r, roa);
	}			
	return will_this_fit(r, part, report, size, page);
}

void rlib_set_report_from_part(rlib *r, struct rlib_part *part, struct rlib_report *report, gfloat top_margin_offset) {
	gint i;
	for(i=0;i<report->pages_accross;i++) {
		report->position_top[i] = part->position_top[0] + top_margin_offset;
		report->bottom_size[i] = part->bottom_size[0];
		report->position_bottom[i] = part->position_bottom[0];
	}

}

gint rlib_end_page_if_line_wont_fit(rlib *r, struct rlib_part *part, struct rlib_report *report, struct rlib_element *e) {
	gint i, fits=TRUE;	
	for(i=0;i<report->pages_accross;i++) {
		if(!will_outputs_fit(r,part, report, e, i+1))
			fits=FALSE;
	}
	if(!fits) {
		if(report->raw_page_number < r->current_page_number) {
			OUTPUT(r)->end_page_again(r, part, report);
			report->raw_page_number++;
			OUTPUT(r)->set_raw_page(r, part, report->raw_page_number);
		} else {
			OUTPUT(r)->end_page(r, part, report);
			rlib_layout_init_part_page(r, part);
			report->raw_page_number++;
		}
		rlib_set_report_from_part(r, part, report, 0);
		
//HERE		
	}
	return !fits;
}

void rlib_print_report_footer(rlib *r, struct rlib_part *part, struct rlib_report *report) {
	gint i;
	if(report->report_footer != NULL) {
		for(i=0;i<report->pages_accross;i++)
			rlib_end_page_if_line_wont_fit(r, part, report, report->report_footer);
		rlib_layout_report_output(r, part, report, report->report_footer, FALSE);
	}
}

gint rlib_fetch_first_rows(rlib *r) {
	gint i;
	gint result = TRUE;
	for(i=0;i<r->queries_count;i++) {
		if(INPUT(r,i)->first(INPUT(r,i), r->results[i].result) == FALSE) {
			result = FALSE;
		}
	}
	return result;
}

static void rlib_init_variables(rlib *r, struct rlib_report *report) {
	struct rlib_element *e;
	for(e = report->variables; e != NULL; e=e->next) {
		struct rlib_report_variable *rv = e->data;
		if(rv->type == REPORT_VARIABLE_EXPRESSION) {
			rv->data = g_malloc(sizeof(struct rlib_count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_COUNT) {
			rv->data = g_malloc(sizeof(struct rlib_count_amount));
			RLIB_VARIABLE_CA(rv)->count = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->count, 0);
		} else if(rv->type == REPORT_VARIABLE_SUM) {
			rv->data = g_malloc(sizeof(struct rlib_count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_AVERAGE) {
			rv->data = g_malloc(sizeof(struct rlib_count_amount));
			RLIB_VARIABLE_CA(rv)->count = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->count, 0);
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_LOWEST) {
			rv->data = g_malloc(sizeof(struct rlib_count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		} else if(rv->type == REPORT_VARIABLE_HIGHEST) {
			rv->data = g_malloc(sizeof(struct rlib_count_amount));
			RLIB_VARIABLE_CA(rv)->amount = *rlib_value_new_number(&RLIB_VARIABLE_CA(rv)->amount, 0);
		}
	}
	
}

static void rlib_process_variables(rlib *r, struct rlib_report *report) {
	struct rlib_element *e;
	for(e = report->variables; e != NULL; e=e->next) {
		struct rlib_report_variable *rv = e->data;
		struct rlib_value *count = &RLIB_VARIABLE_CA(rv)->count;
		struct rlib_value *amount = &RLIB_VARIABLE_CA(rv)->amount;
		struct rlib_value execute_result, *er = &execute_result;
		if(rv->code != NULL)
			 rlib_execute_pcode(r, &execute_result, rv->code, NULL);
		if(rv->type == REPORT_VARIABLE_COUNT) {
			RLIB_VALUE_GET_AS_NUMBER(count) += RLIB_DECIMAL_PRECISION;
		} else if(rv->type == REPORT_VARIABLE_EXPRESSION) {
			if(RLIB_VALUE_IS_NUMBER(er)) {
				rlib_value_free(amount);
				rlib_value_new_number(amount, RLIB_VALUE_GET_AS_NUMBER(er));
			} else if (RLIB_VALUE_IS_STRING(er)) {
				rlib_value_free(amount);
				rlib_value_new_string(amount, RLIB_VALUE_GET_AS_STRING(er));
			} else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER OR STRING FOR REPORT_VARIABLE_EXPRESSION\n");
		} else if(rv->type == REPORT_VARIABLE_SUM) {
			if(RLIB_VALUE_IS_NUMBER(er))
				RLIB_VALUE_GET_AS_NUMBER(amount) += RLIB_VALUE_GET_AS_NUMBER(er);
			else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_SUM\n");
		} else if(rv->type == REPORT_VARIABLE_AVERAGE) {
			RLIB_VALUE_GET_AS_NUMBER(count) += RLIB_DECIMAL_PRECISION;
			if(RLIB_VALUE_IS_NUMBER(er))
				RLIB_VALUE_GET_AS_NUMBER(amount) += RLIB_VALUE_GET_AS_NUMBER(er);
			else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_AVERAGE\n");
		} else if(rv->type == REPORT_VARIABLE_LOWEST) {
			if(RLIB_VALUE_IS_NUMBER(er)) {
				if(RLIB_VALUE_GET_AS_NUMBER(er) < RLIB_VALUE_GET_AS_NUMBER(amount) || RLIB_VALUE_GET_AS_NUMBER(amount) == 0) //TODO: EVIL HACK
					RLIB_VALUE_GET_AS_NUMBER(amount) = RLIB_VALUE_GET_AS_NUMBER(er);
			} else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_LOWEST\n");
		} else if(rv->type == REPORT_VARIABLE_HIGHEST) {
			if(RLIB_VALUE_IS_NUMBER(er)) {
				if(RLIB_VALUE_GET_AS_NUMBER(er) > RLIB_VALUE_GET_AS_NUMBER(amount) || RLIB_VALUE_GET_AS_NUMBER(amount) == 0) //TODO: EVIL HACK
					RLIB_VALUE_GET_AS_NUMBER(amount) = RLIB_VALUE_GET_AS_NUMBER(er);
			} else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER FOR REPORT_VARIABLE_HIGHEST\n");
		}
	}
	
}

static void rlib_evaluate_report_attributes(rlib *r, struct rlib_report *report) {
	gint t;
	
	if (rlib_execute_as_int_inlist(r, report->orientation_code, &t, orientations))
		if ((t == RLIB_ORIENTATION_PORTRAIT) || (t == RLIB_ORIENTATION_LANDSCAPE))
			report->orientation = t;
	if (rlib_execute_as_int(r, report->font_size_code, &t))
		report->font_size = t;
	if (rlib_execute_as_int(r, report->top_margin_code, &t))
		report->top_margin = t;
	if (rlib_execute_as_int(r, report->left_margin_code, &t))
		report->left_margin = t;
	if (rlib_execute_as_int(r, report->bottom_margin_code, &t))
		report->bottom_margin = t;
	if (rlib_execute_as_int(r, report->pages_across_code, &t))
		report->pages_accross = t;
	if (rlib_execute_as_int(r, report->suppress_page_header_first_page_code, &t))
		report->suppress_page_header_first_page = t;
}

static void rlib_evaluate_break_attributes(rlib *r, struct rlib_report *report) {
	struct rlib_report_break *rb;
	struct rlib_element *e;
	gint t;
	
	for(e = report->breaks; e != NULL; e = e->next) {
		rb = e->data;
		if (rlib_execute_as_boolean(r, rb->newpage_code, &t))
			rb->newpage = t;
		if (rlib_execute_as_boolean(r, rb->headernewpage_code, &t))
			rb->headernewpage = t;
		if (rlib_execute_as_boolean(r, rb->suppressblank_code, &t))
			rb->suppressblank = t;
	}
}

void rlib_process_expression_variables(rlib *r, struct rlib_report *report) {
	struct rlib_element *e;
	for(e = report->variables; e != NULL; e=e->next) {
		struct rlib_report_variable *rv = e->data;
		struct rlib_value *amount = &RLIB_VARIABLE_CA(rv)->amount;
		struct rlib_value execute_result, *er = &execute_result;
		if(rv->code != NULL)
			 rlib_execute_pcode(r, &execute_result, rv->code, NULL);
		if(rv->type == REPORT_VARIABLE_EXPRESSION) {
			if(RLIB_VALUE_IS_NUMBER(er)) {
				rlib_value_free(amount);
				rlib_value_new_number(amount, RLIB_VALUE_GET_AS_NUMBER(er));
			} else if (RLIB_VALUE_IS_STRING(er)) {
				rlib_value_free(amount);
				rlib_value_new_string(amount, RLIB_VALUE_GET_AS_STRING(er));
			} else
				rlogit("rlib_process_variables EXPECTED TYPE NUMBER OR STRING FOR REPORT_VARIABLE_EXPRESSION\n");
		}
	}
	
}

static void rlib_evaluate_part_attributes(rlib *r, struct rlib_part *part) {
	gint t;
	char buf[MAXSTRLEN];
	
	if (rlib_execute_as_int_inlist(r, part->orientation_code, &t, orientations))
		if ((t == RLIB_ORIENTATION_PORTRAIT) || (t == RLIB_ORIENTATION_LANDSCAPE))
			part->orientation = t;
	if (rlib_execute_as_int(r, part->font_size_code, &t))
		part->font_size = t;
	if (rlib_execute_as_int(r, part->top_margin_code, &t))
		part->top_margin = t;
	if (rlib_execute_as_int(r, part->left_margin_code, &t))
		part->left_margin = t;
	if (rlib_execute_as_int(r, part->bottom_margin_code, &t))
		part->bottom_margin = t;
	if (rlib_execute_as_int(r, part->pages_across_code, &t))
		part->pages_accross = t;
	if (rlib_execute_as_string(r, part->paper_type_code, buf, MAXSTRLEN)) {
		struct rlib_paper *paper = rlib_get_paper_by_name(r, buf);
		if(paper != NULL)
			part->paper = paper;
	}
}

void rlib_layout_report(rlib *r, struct rlib_part *part, struct rlib_report *report, gfloat left_margin_offset, gfloat top_margin_offset) {
	gint processed_variables;
	gint i;
	char query[MAXSTRLEN];

	report->query_code = rlib_infix_to_pcode(r, report, report->xml_query);
	if(report->query_code != NULL) {
		rlib_execute_as_string(r, report->query_code, query, MAXSTRLEN);
		for(i=0;i<r->queries_count;i++) {
			if(!strcmp(r->results[i].name, query)) {
				r->current_result = i;		
				break;
			}
		}
	} else {
		r->current_result = 0;
	}

	rlib_resolve_report_fields(r, report);
	rlib_init_variables(r, report);
	rlib_process_variables(r, report);
	processed_variables = TRUE;
	rlib_evaluate_report_attributes(r, report);

	rlib_set_report_from_part(r, part, report, top_margin_offset);

	report->left_margin += left_margin_offset;

	rlib_layout_report_output(r, part, report, report->report_header, FALSE);	


	if(report->font_size != -1)
		r->font_point = report->font_size;

/*		if (!first_result) {
			rlib_layout_report_output(r, rr->alternate.nodata, FALSE);
*/			
		INPUT(r,r->current_result)->first(INPUT(r,r->current_result), r->results[r->current_result].result);

		if(!INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result)) {
				while (1) {
					gint page;
					gint output_count = 0;

					if(!processed_variables) {
						rlib_process_variables(r, report);
						processed_variables = TRUE;
					}
					rlib_evaluate_break_attributes(r, report);
					rlib_handle_break_headers(r, part, report);

					if(rlib_end_page_if_line_wont_fit(r, part, report, report->detail.fields))
						for (page = 0; page < report->pages_accross; page++)
							rlib_force_break_headers(r, part, report);

					if(OUTPUT(r)->do_break)
						output_count = rlib_layout_report_output(r, part, report, report->detail.fields, FALSE);
					else
						output_count = rlib_layout_report_output_with_break_headers(r, part, report);

					if(output_count > 0)
						r->detail_line_count++;

					if(rlib_navigate_next(r, r->current_result) == FALSE) {
						rlib_navigate_last(r, r->current_result);
						rlib_handle_break_footers(r, part, report);
						break;
					} 

					rlib_evaluate_break_attributes(r, report);
					rlib_handle_break_footers(r, part, report);
					processed_variables = FALSE;
				}
			}

//			rlib_navigate_last(r, r->current_result);

//			rlib_print_report_footer(r, report);
//		OUTPUT(r)->end_report(r);


}

struct rlib_report_position {
	long page;
	gfloat position_top;
};

void rlib_layout_part_td(rlib *r, struct rlib_part *part, struct rlib_element *e_td, long page_number, gfloat position_top, struct rlib_report_position *rrp) {
	struct rlib_element *e, *td_contents;
	
	gfloat paper_width = rlib_layout_get_page_width(r, part) - (part->left_margin * 2);
	gfloat running_left_margin = 0;
	
	for(e=e_td;e != NULL;e=e->next) {
		gfloat running_top_margin = 0;
		struct rlib_part_td *td = e->data;
		rlogit("    TD %s\n", td->width);
		for(td_contents=td->e;td_contents != NULL;td_contents=td_contents->next) {
			if(td_contents->type == RLIB_ELEMENT_REPORT) {
				struct rlib_report *report = td_contents->data;
				report->page_width = ((atof(td->width)/100) * paper_width);
				rlogit("-- REPORT -- SETTING PAGE NUMBER TO %d\n", page_number);
				OUTPUT(r)->set_raw_page(r, part, page_number);
				report->raw_page_number = page_number;
				rlib_layout_report(r, part, report, running_left_margin, running_top_margin+position_top);
				running_top_margin += report->position_top[0] - part->position_top[0];
				if(report->raw_page_number > rrp->page) {
					rrp->page = report->raw_page_number;
					rrp->position_top = report->position_top[0];				
				} else if(report->raw_page_number == rrp->page) {
					if(report->position_top[0] > rrp->position_top)
						rrp->position_top = report->position_top[0];
				}
			} else {
				rlogit("UNKNOWN\n");
			}
		}
		running_left_margin += ((atof(td->width)/100) * paper_width);
	}	
}

void rlib_layout_part_tr(rlib *r, struct rlib_part *part, struct rlib_element *e_tr) {
	struct rlib_report_position rrp;
	struct rlib_element *e;
	
	bzero(&rrp, sizeof(rrp));
	
	for(e=e_tr;e != NULL;e=e->next) {
		struct rlib_part_tr *tr = e->data;
		gfloat save_position_top = 0;
		long save_page_number = r->current_page_number;
		if(rrp.position_top > 0)
			save_position_top = rrp.position_top - part->position_top[0];
		rlogit("  TR %s + [%f]\n", tr->height, rrp.position_top);
		bzero(&rrp, sizeof(rrp));
		rlib_layout_part_td(r, part, tr->e, save_page_number, save_position_top, &rrp);
	}	
}


gint make_report(rlib *r) {
	gint i = 0;
//	gint processed_variables = FALSE;
	gint first_result;
	if(r->format == RLIB_FORMAT_HTML)
		rlib_html_new_output_filter(r);
	else if(r->format == RLIB_FORMAT_TXT)
		rlib_txt_new_output_filter(r);
	else if(r->format == RLIB_FORMAT_CSV)
		rlib_csv_new_output_filter(r);
#ifdef HAVE_LIBCPDF
	else
		rlib_pdf_new_output_filter(r);
#endif
	r->current_font_point = -1;

	OUTPUT(r)->set_fg_color(r, -1, -1, -1);
	OUTPUT(r)->set_bg_color(r, -1, -1, -1);

	r->current_page_number = 1;
	r->current_result = 0;
	r->start_of_new_report = TRUE;

	OUTPUT(r)->init_output(r);
	first_result = rlib_fetch_first_rows(r);

	r->current_output_encoder = NULL;
	for(i=0;i<r->parts_count;i++) {
		struct rlib_part *part = r->parts[i];
		OUTPUT(r)->start_report(r, part);
		rlib_resolve_part_fields(r, part);
		rlib_evaluate_part_attributes(r, part);
		if(part->font_size != -1)
			r->font_point = part->font_size;

		rlib_layout_init_part_page(r, part);
		rlib_layout_part_tr(r, part, part->tr_elements);
		
	}
	
	
/*	for(report=0;report<r->reports_count;report++) {
		struct rlib_report *rr;
				
		processed_variables = FALSE;
		r->current_report = report;
		rr = r->reports[report];
		if(report > 0) {
			if(rr->mainloop_query != -1) {
				r->current_result = rr->mainloop_query;
				// Reset the query to be used again
				first_result = INPUT(r,r->current_result)->first(INPUT(r,r->current_result), r->results[r->current_result].result);
			}
		}
// If this report has a specific output converter, use it otherwise use the reports encoder.
		r->current_output_encoder = (rr->output_encoder)? rr->output_encoder : r->output_encoder;
//		r->current_db_encoder = (rr->db_encoder)? rr->db_encoder : r->db_encoder;
//		r->current_param_encoder = (rr->param_encoder)? rr->param_encoder : r->param_encoder;
		rlib_resolve_report_fields(r);
		rlib_init_variables(r);
		rlib_process_variables(r);
		processed_variables = TRUE;
		rlib_evaluate_report_attributes(r);
		if(rr->font_size != -1)
			r->font_point = rr->font_size;
		OUTPUT(r)->start_report(r);
		rlib_init_page(r, TRUE);		
		if (!first_result) {
			rlib_layout_report_output(r, rr->alternate.nodata, FALSE);
		} else {
			if(!INPUT(r, r->current_result)->isdone(INPUT(r, r->current_result), r->results[r->current_result].result)) {
				while (1) {
					gint page;
					gint output_count = 0;
					if(!processed_variables) {
						rlib_process_variables(r);
						processed_variables = TRUE;
					}
					rlib_evaluate_break_attributes(r);
					rlib_handle_break_headers(r);

					if(rlib_end_page_if_line_wont_fit(r, rr->detail.fields))
						for (page = 0; page < rr->pages_accross; page++)
							rlib_force_break_headers(r);

					if(OUTPUT(r)->do_break)
						output_count = rlib_layout_report_output(r, rr->detail.fields, FALSE);
					else
						output_count = rlib_layout_report_output_with_break_headers(r);

					if(output_count > 0)
						r->detail_line_count++;
					i++;

					if(rlib_navigate_next(r, r->current_result) == FALSE) {
						rlib_navigate_last(r, r->current_result);
						rlib_handle_break_footers(r);
						break;
					} 

					rlib_evaluate_break_attributes(r);
					rlib_handle_break_footers(r);
					processed_variables = FALSE;
				}
			}

			rlib_navigate_last(r, r->current_result);

			rlib_print_report_footer(r);
		}
		OUTPUT(r)->end_report(r);

		if(report+1 < r->reports_count) {
			r->current_page_number++;
			r->start_of_new_report = TRUE;
			r->current_line_number = 1;
			r->detail_line_count = 0;
			r->font_point = FONTPOINT;
		}
		rlib_char_encoder_destroy(&rr->output_encoder); //Destroy if was one.
//		rlib_char_encoder_destroy(&rr->db_encoder); //Destroy if was one.
//		rlib_char_encoder_destroy(&rr->param_encoder); //Destroy if was one.
	}
	*/
	return 0;
}

gint rlib_finalize(rlib *r) {
	OUTPUT(r)->finalize_private(r);
	return 0;
}


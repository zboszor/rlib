/*
 *  Copyright (C) 2003-2016 SICOM Systems, INC.
 *
 *  Authors: Chet Heilman <cheilman@sicompos.com>
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
 */

#include <config.h>

#include <glib.h>
#include <time.h>
#include <string.h>

#include "util.h"
#include "datetime.h"
#include "rlib-internal.h"
 
gboolean rlib_datetime_valid_date(struct rlib_datetime *dt) {
	return g_date_valid(&dt->date);
}

gboolean rlib_datetime_valid_time(struct rlib_datetime *dt) {
	return (dt->ltime > 0);
}

void rlib_datetime_clear_time(struct rlib_datetime *t) {
	t->ltime = 0LL;
}

void rlib_datetime_clear_date(struct rlib_datetime *t) {
	g_date_clear(&t->date, 1);
}

void rlib_datetime_clear(struct rlib_datetime *t1) {
	rlib_datetime_clear_time(t1);
	rlib_datetime_clear_date(t1);
}

void rlib_datetime_makesamedate(struct rlib_datetime *target, struct rlib_datetime *chgto) {
	target->date = chgto->date;
}

void rlib_datetime_makesametime(struct rlib_datetime *target, struct rlib_datetime *chgto) {
	target->ltime = chgto->ltime;
}

gint rlib_datetime_compare(struct rlib_datetime *t1, struct rlib_datetime *t2) {
	gint result = 0;
	if (rlib_datetime_valid_date(t1) && rlib_datetime_valid_date(t2)) {
		result = g_date_compare(&t1->date, &t2->date);
	}
	if (result == 0 && rlib_datetime_valid_time(t1) && rlib_datetime_valid_time(t2)) {
		result = t1->ltime - t2->ltime;
	}
	return result;
}

void rlib_datetime_set_date(struct rlib_datetime *dt, gint y, gint m, gint d) {
	GDate *t;
	if (d && m && y) {
		t = g_date_new_dmy(d, m, y);
		if(t != NULL) {
			dt->date = *t;
			g_date_free(t);
		} else {
			memset(&dt->date, 0, sizeof(dt->date));
		}
	} else
		memset(&dt->date, 0, sizeof(dt->date));
	dt->ltime = 0;
}

void rlib_datetime_set_time(struct rlib_datetime *dt, guchar h, guchar m, guchar s) {
	dt->ltime = (1 << 24) || ((h * 60) + m) * 60 + s;
}

void rlib_datetime_set_time_from_long(struct rlib_datetime *dt, long t) {
	gint h, m, s;
	s = t % 60;
	t /= 60;
	m = t % 60;
	t /= 60;
	h = t % 24;
	rlib_datetime_set_time(dt, h, m, s);
}

static void rlib_datetime_format_date(struct rlib_datetime *dt, char *buf, gint max, const char *fmt) {
	if (rlib_datetime_valid_date(dt)) {
		g_date_strftime(buf, max, fmt, &dt->date);
	} else {
		strcpy(buf, "");
/*		r_error("Invalid date in format date"); */
	}
}

static void rlib_datetime_format_time(struct rlib_datetime *dt, char *buf, gint max, const char *fmt) {
	time_t now = time(NULL);
	struct tm *tmp = localtime(&now);
	if (rlib_datetime_valid_time(dt)) {
		tmp->tm_hour = ((dt->ltime & 0x00FF0000) >> 16);
		tmp->tm_min = ((dt->ltime & 0x0000FF00) >> 8);
		tmp->tm_sec = (dt->ltime & 0x000000FF);
		strftime(buf, max, fmt, tmp);
	} else {
		strcpy(buf, "!ERR_DT_T");
		r_error(NULL, "Invalid time in format time");
	}
}

/* separate format string into 2 pcs. one with date, other with time. */
static gchar datechars[] = "aAbBcCdDeFgGhJmuUVwWxyY";
static gchar timechars[] = "HIklMpPrRsSTXzZ";
static void split_tdformat(gchar **datefmt, gchar **timefmt, gint *order, const gchar *fmtstr) {
	gchar *splitpoint = NULL;
	gchar *s, *t = NULL;
	gchar *pctptr;
	gint mode = 0;

	*timefmt = *datefmt = NULL;
	*order = 0;
	t = (gchar *) fmtstr;
	while (!splitpoint && (t = r_strchr(t, r_strlen(t), '%'))) {
		pctptr = t;

		t = r_nextchr(t);
		switch (r_getchr(t)) {
		case '%':
			t = r_nextchr(t);
			break;
		case 'E': /* These are prefixes that moderate the next char */
		case 'O':
			t = r_nextchr(t);
			/* supposed to fall thru - break intentionally missing */
		default:
			if ((s = r_strchr(datechars, r_strlen(datechars), r_getchr(t)))) {
				if (mode && (mode != 1)) splitpoint = pctptr;
				if (!mode) mode = 1; /* date first */
			} else if ((s = r_strchr(timechars, r_strlen(timechars), r_getchr(t)))) {
				if (mode && (mode != 2)) splitpoint = pctptr;
				if (!mode) mode = 2; /* time first */
			}
			t = r_nextchr(t);
			break;
		}
	}
	switch (mode) {
	case 1: /* date first */
		if (splitpoint) {
			*timefmt = g_strdup(splitpoint);
			*datefmt = g_strndup(fmtstr, splitpoint - fmtstr);
		} else {
			*datefmt = g_strdup(fmtstr);
		}
		break;
	case 2: /* time first */
		if (splitpoint) {
			*timefmt = g_strndup(fmtstr, splitpoint - fmtstr);
			*datefmt = g_strdup(splitpoint);
		} else {
			*timefmt = g_strdup(fmtstr);
		}
		break;
	}
	*order = mode;
}


void rlib_datetime_format(rlib *r, gchar **woot_dest, struct rlib_datetime *dt, const gchar *fmt) {
	GString *str, *date = NULL, *last_literal = NULL;
	gint advance;
	gboolean found_date = FALSE;
	gint types[2] = { RLIB_FORMATSTR_DATE, RLIB_FORMATSTR_LITERAL };
	gint type_idx, type;
	gchar *datefmt, *timefmt;
	gint order;
	gchar datebuf[128];
	gchar timebuf[128];
	gint havedate = FALSE, havetime = FALSE;
	gint max = MAXSTRLEN;
	gchar *dest = g_malloc(MAXSTRLEN);

	str = g_string_new("");
	advance = 0;
	type_idx = 0;
	while (fmt[advance]) {
		GString *tmp;
		gint adv;
		gboolean error;

		tmp = get_next_format_string(r, fmt + advance, types[type_idx], &type, &adv, &error);
		if (error) {
			g_string_free(str, TRUE);
			*woot_dest = g_string_free(tmp, FALSE);
			return;
		}

		if (types[type_idx] == type)
			type_idx++;

		switch (type) {
		case RLIB_FORMATSTR_LITERAL:
			if (!found_date) {
				g_string_append(str, tmp->str);
				g_string_free(tmp, TRUE);
			} else
				last_literal = tmp;
			break;
		case RLIB_FORMATSTR_DATE:
			found_date = TRUE;
			date = tmp;
			break;
		default:
			g_strlcpy(dest, "!ERR_DT_NO", max);
			g_string_free(str, TRUE);
			g_string_free(date, TRUE);
			g_string_free(last_literal, TRUE);
			*woot_dest = dest;
			return;
		}

		advance += adv;
	}

	if (!found_date) {
		g_strlcpy(dest, "!ERR_DT_NO", max);
		*woot_dest = dest;
		g_string_free(str, TRUE);
		g_string_free(date, TRUE);
		g_string_free(last_literal, TRUE);
		return;
	}

	split_tdformat(&datefmt, &timefmt, &order, date->str);
	*datebuf = *timebuf = '\0';
	if (datefmt && rlib_datetime_valid_date(dt)) {
		rlib_datetime_format_date(dt, datebuf, 127, datefmt);	
		havedate = TRUE;
	} 
	if (timefmt && rlib_datetime_valid_time(dt)) {
		rlib_datetime_format_time(dt, timebuf, 127, timefmt);
		havetime = TRUE;
	}
	if (timefmt && !havetime) {
		r_warning(r, "Attempt to format time with NULL time value");
	}
	if (datefmt && !havedate) {
		r_warning(r, "Attempt to format date with NULL date value");
	}
	switch (order) {
	case 1:
		g_strlcpy(dest, datebuf, max);
		g_strlcat(dest, timebuf, max - r_strlen(datebuf));
		break;
	case 2:
		g_strlcpy(dest, timebuf, max);
		g_strlcat(dest, datebuf, max - r_strlen(timebuf));
		break;
	default:
		g_strlcpy(dest, "!ERR_DT_NO", max);
		r_error(r, "Datetime format has no date or no format");
		break; /* format has no date or time codes ??? */
	}
	if (datefmt)
		g_free(datefmt);
	if (timefmt)
		g_free(timefmt);

	g_string_append(str, dest);
	g_free(dest);
	g_string_free(date, TRUE);
	if (last_literal) {
		g_string_append(str, last_literal->str);
		g_string_free(last_literal, TRUE);
	}

	*woot_dest = g_string_free(str, FALSE);
}


gint rlib_datetime_daysdiff(struct rlib_datetime *dt, struct rlib_datetime *dt2) {
	return g_date_days_between(&dt->date, &dt2->date);
}


void rlib_datetime_addto(struct rlib_datetime *dt, gint amt) {
	long ndays = amt;
	long nsecs = 0;
	if (rlib_datetime_valid_time(dt)) {
		nsecs = rlib_datetime_time_as_long(dt) + amt;
		ndays = nsecs / RLIB_DATETIME_SECSPERDAY;
		rlib_datetime_set_time_from_long(dt, nsecs % RLIB_DATETIME_SECSPERDAY);
	}
	if (rlib_datetime_valid_date(dt)) {
		if (ndays != 0) {
			if (ndays >= 0) {
				g_date_add_days(&dt->date, ndays);
			} else {
				g_date_subtract_days(&dt->date, -ndays);
			}
		}
	}
}


gint rlib_datetime_secsdiff(struct rlib_datetime *dt, struct rlib_datetime *dt2) {
	return dt->ltime - dt2->ltime;
}


/* END of rlib_datetime */


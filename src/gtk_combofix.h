/* ComboFix Widget
 * Copyright (C) 2001-2006 Cédric Auger
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifndef __GTK_COMBOFIX__H__
#define __GTK_COMBOFIX__H__

#include <gtk/gtk.h>

#define GTK_TYPE_COMBOFIX           (gtk_combofix_get_type ())
#define GTK_COMBOFIX(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_COMBOFIX, GtkComboFix))
#define GTK_IS_COMBOFIX(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_COMBOFIX))

typedef struct _GtkComboFix         GtkComboFix;
typedef struct _GtkComboFixClass    GtkComboFixClass;

/* structure of the ComboFix */
struct _GtkComboFix
{
    GtkBox			parent;

    /* entry of the combofix */
    GtkWidget *		entry;
};

struct _GtkComboFixClass
{
    GtkBoxClass		parent_class;
};

/* construction */
GType 			gtk_combofix_get_type 					(void) G_GNUC_CONST;
GtkWidget *		gtk_combofix_new 						(GSList *list);
GtkWidget *		gtk_combofix_new_with_properties 		(GSList *list,
											 			 gboolean force_text,
							                             gboolean max_items,
							                             gboolean case_sensitive,
							                             gboolean mixed_sort);
/* text */
const gchar *	gtk_combofix_get_text					(GtkComboFix *combofix);
void 			gtk_combofix_set_text					(GtkComboFix *combofix,
								                         const gchar *text);
void 			gtk_combofix_set_force_text				(GtkComboFix *combofix,
							                             gboolean value);
void 			gtk_combofix_set_case_sensitive			(GtkComboFix *combofix,
					                                     gboolean case_sensitive);

/* popup */
gboolean 		gtk_combofix_show_popup					(GtkComboFix *combofix);
gboolean 		gtk_combofix_hide_popup					(GtkComboFix *combofix);

/* list of items */
void 			gtk_combofix_append_report				(GtkComboFix *combofix,
														 const gchar *report_name);
void 			gtk_combofix_append_text				(GtkComboFix *combofix,
							                             const gchar *text);
void 			gtk_combofix_remove_report				(GtkComboFix *combofix,
							                             const gchar *report_name);
void 			gtk_combofix_remove_text				(GtkComboFix *combofix,
							                             const gchar *text);
gboolean 		gtk_combofix_set_list					(GtkComboFix *combofix,
							                             GSList *list);
void 			gtk_combofix_set_max_items				(GtkComboFix *combofix,
							                             gint max_items);
void 			gtk_combofix_set_mixed_sort				(GtkComboFix *combofix,
							                             gboolean mixed_sort);

/* set callback */
void 			gtk_combofix_set_selection_callback		(GtkComboFix *combofix,
							                             GCallback func,
							                             gpointer data);

#endif /* __GTK_COMBOFIX_H__ */

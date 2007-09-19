/* ************************************************************************** */
/* Fichier qui s'occupe de la page d'accueil ( de démarrage lors de           */
/* l'ouverture d'un fichier de comptes                                        */
/*                                                                            */
/*                                  barre_outis.c                             */
/*                                                                            */
/*     Copyright (C)	2000-2006 Cédric Auger (cedric@grisbi.org)	      */
/*			2004-2006 Benjamin Drieu (bdrieu@april.org)	      */
/*			1995-1997 Peter Mattis, Spencer Kimball and	      */
/*			          Jsh MacDonald				      */
/* 			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */


#include "include.h"

/*START_INCLUDE*/
#include "barre_outils.h"
#include "./gsb_scheduler_list.h"
#include "./gsb_transactions_list.h"
#include "./gsb_automem.h"
#include "./gsb_data_account.h"
#include "./navigation.h"
#include "./gsb_reconcile.h"
#include "./menu.h"
#include "./traitement_variables.h"
#include "./utils_buttons.h"
#include "./include.h"
#include "./structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *creation_barre_outils ( void );
static gboolean popup_scheduled_view_mode_menu ( GtkWidget * button );
static gboolean popup_transaction_view_mode_menu ( GtkWidget * button );
/*END_STATIC*/



/** Used to display/hide comments in scheduler list */
GtkWidget *scheduler_display_hide_comments;

GtkWidget *bouton_affiche_cache_formulaire_echeancier;
GtkWidget *bouton_affiche_commentaire_echeancier;
GtkWidget *fleche_bas_echeancier;
GtkWidget *fleche_haut_echeancier;
GtkWidget *bouton_ope_lignes[4];
GtkWidget *bouton_affiche_r;
GtkWidget *bouton_enleve_r;
GtkWidget *bouton_grille;
GtkWidget *bouton_grille_echeancier;

/** here are the 3 buttons on the scheduler toolbar
 * which can been unsensitive or sensitive */
GtkWidget *scheduler_button_execute;
GtkWidget *scheduler_button_delete;
GtkWidget *scheduler_button_edit;

/* widgets du bouton pour afficher/cacher le transaction_form */

GtkWidget *bouton_affiche_cache_formulaire;
GtkWidget *fleche_haut;
GtkWidget *fleche_bas;




/*START_EXTERN*/
extern GtkWidget *barre_outils;
extern gboolean block_menu_cb ;
extern GtkTooltips *tooltips_general_grisbi;
extern GtkUIManager * ui_manager;
/*END_EXTERN*/



/*******************************************************************************************/
GtkWidget *creation_barre_outils ( void )
{
    GtkWidget *hbox, *menu, *button;

    /* Hbox */
    hbox = gtk_hbox_new ( FALSE, 0 );

    /* Add various icons */
    button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
					       _("New transaction"),
					       "new-transaction.png",
					       G_CALLBACK ( new_transaction ),
					       GINT_TO_POINTER(-1) );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Blank the form to create a new transaction"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    button = gsb_automem_stock_button_new ( etat.display_toolbar,
					   GTK_STOCK_DELETE, 
					   _("Delete"),
					   G_CALLBACK ( remove_transaction ),
					   NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Delete selected transaction"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    button = gsb_automem_stock_button_new ( etat.display_toolbar,
					   GTK_STOCK_PROPERTIES, 
					   _("Edit"),
					   G_CALLBACK ( gsb_transactions_list_edit_current_transaction ),
					   NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Edit current transaction"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
					       _("Reconcile"),
					       "reconciliation.png",
					       G_CALLBACK (gsb_reconcile_run_reconciliation),
					       GINT_TO_POINTER(-1) );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Start account reconciliation"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    menu = gsb_automem_stock_button_menu_new ( etat.display_toolbar,
					      GTK_STOCK_SELECT_COLOR, _("View"),
					      G_CALLBACK(popup_transaction_view_mode_menu),
					      NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), menu,
			   _("Change display mode of the list"), "" );
    gtk_box_pack_start ( GTK_BOX(hbox), menu, FALSE, FALSE, 0 );

    gtk_widget_show_all ( hbox );

    return ( hbox );
}



/**
 *
 *
 *
 */
void gsb_gui_update_transaction_toolbar ( void )
{
    GList * list = NULL;

    list = gtk_container_get_children ( GTK_CONTAINER ( barre_outils ) );
    
    if ( list )
    {
	gtk_container_remove ( GTK_CONTAINER ( barre_outils ),
			       GTK_WIDGET ( list -> data ) );
	g_list_free ( list );
    }
    gtk_container_add ( GTK_CONTAINER ( barre_outils ), creation_barre_outils () );
}



/**
 *
 *
 */
gboolean popup_transaction_view_mode_menu ( GtkWidget * button )
{
    GtkWidget *menu, *menu_item;

    menu = gtk_menu_new ();

    menu_item = gtk_menu_item_new_with_label ( _("Simple view") );
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    g_signal_connect_swapped ( G_OBJECT(menu_item), "activate", 
			       G_CALLBACK (change_aspect_liste), GINT_TO_POINTER (1) );

    menu_item = gtk_menu_item_new_with_label ( _("Two lines view") );
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    g_signal_connect_swapped ( G_OBJECT(menu_item), "activate", 
			       G_CALLBACK (change_aspect_liste), GINT_TO_POINTER (2) );

    menu_item = gtk_menu_item_new_with_label ( _("Three lines view") );
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    g_signal_connect_swapped ( G_OBJECT(menu_item), "activate", 
			       G_CALLBACK (change_aspect_liste), GINT_TO_POINTER (3) );

    menu_item = gtk_menu_item_new_with_label ( _("Complete view") );
    gtk_menu_append ( GTK_MENU ( menu ), menu_item );
    g_signal_connect_swapped ( G_OBJECT(menu_item), "activate", 
			       G_CALLBACK (change_aspect_liste), GINT_TO_POINTER (4) );

    gtk_menu_set_active ( GTK_MENU(menu), 
			  gsb_data_account_get_nb_rows ( gsb_gui_navigation_get_current_account () ) );

    gtk_widget_show_all ( menu );
    gtk_menu_popup ( GTK_MENU(menu), NULL, button, set_popup_position, button, 1, 
		     gtk_get_current_event_time());

    return FALSE;
}




/****************************************************************************************************/
gboolean change_aspect_liste ( gint demande )
{
    GSList *list_tmp;

    block_menu_cb = TRUE;

    switch ( demande )
    {
	case 0:
	    /* 	    changement de l'affichage de la grille */

	  etat.affichage_grille = ! etat.affichage_grille;

	    if ( etat.affichage_grille )
	    {
		/* 		on affiche les grilles */

		g_signal_connect_after ( G_OBJECT ( gsb_scheduler_list_get_tree_view () ),
					 "expose-event",
					 G_CALLBACK ( affichage_traits_liste_echeances ),
					 NULL );

		list_tmp = gsb_data_account_get_list_accounts ();

		while ( list_tmp )
		{
		    gint i;

		    i = gsb_data_account_get_no_account ( list_tmp -> data );

		    g_signal_connect_after ( G_OBJECT ( gsb_transactions_list_get_tree_view()),
					     "expose-event",
					     G_CALLBACK ( affichage_traits_liste_operation ),
					     NULL );

		    list_tmp = list_tmp -> next;
		}
	    }
	    else
	    {
		GSList *list_tmp;

		g_signal_handlers_disconnect_by_func ( G_OBJECT ( gsb_scheduler_list_get_tree_view () ),
						       G_CALLBACK ( affichage_traits_liste_echeances ),
						       NULL );

		list_tmp = gsb_data_account_get_list_accounts ();

		while ( list_tmp )
		{
		    gint i;

		    i = gsb_data_account_get_no_account ( list_tmp -> data );

		    g_signal_handlers_disconnect_by_func ( G_OBJECT ( gsb_transactions_list_get_tree_view()  ),
							   G_CALLBACK ( affichage_traits_liste_operation ),
							   NULL );

		    list_tmp = list_tmp -> next;
		}
	    }
	    gtk_widget_queue_draw ( gsb_transactions_list_get_tree_view());
	    gtk_widget_queue_draw ( gsb_scheduler_list_get_tree_view () );

	    block_menu_cb = TRUE;
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowGrid", NULL ) ) ), 
					   TRUE );
	    block_menu_cb = FALSE;

	    break;

	/* 	1, 2, 3 et 4 sont les nb de lignes qu'on demande à afficher */

	case 1 :
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowOneLine", NULL ) ) ), 
					   TRUE );
	    gsb_transactions_list_set_visible_rows_number ( demande );
	    modification_fichier ( TRUE );
	    break;
	case 2 :
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowTwoLines", NULL ) ) ), 
					   TRUE );
	    gsb_transactions_list_set_visible_rows_number ( demande );
	    modification_fichier ( TRUE );
	    break;
	case 3 :
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowThreeLines", NULL ) ) ), 
					   TRUE );
	    gsb_transactions_list_set_visible_rows_number ( demande );
	    modification_fichier ( TRUE );
	    break;
	case 4 :
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowFourLines", NULL ) ) ), 
					   TRUE );
	    gsb_transactions_list_set_visible_rows_number ( demande );
	    modification_fichier ( TRUE );
	    break;

	case 5 :

	    /* ope avec r */

	    mise_a_jour_affichage_r ( 1 );
	    modification_fichier ( TRUE );

	    block_menu_cb = TRUE;
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowReconciled", NULL ) ) ), 
					   TRUE );
	    block_menu_cb = FALSE;

	    break;

	case 6 :

	    /* ope sans r */

	    mise_a_jour_affichage_r ( 0 );
	    modification_fichier ( TRUE );

	    block_menu_cb = TRUE;
	    gtk_toggle_action_set_active ( GTK_TOGGLE_ACTION (gtk_ui_manager_get_action ( ui_manager, 
											  menu_name ( "ViewMenu", "ShowReconciled", NULL ) ) ), 
					   FALSE );
	    block_menu_cb = FALSE;

	    break;
    }

    block_menu_cb = FALSE;

    return ( TRUE );
}



/**
 *
 *
 *
 */
gboolean popup_scheduled_view_mode_menu ( GtkWidget * button )
{
    GtkWidget *menu, *item;
    gchar * names[] = { _("Unique view"), _("Week view"), _("Month view"), 
			_("Two months view"), _("Quarter view"), 
			_("Year view"), _("Custom view"), NULL, };
    int i;

    menu = gtk_menu_new ();
    
    for ( i = 0 ; names[i] ; i++ )
    {
	item = gtk_menu_item_new_with_label ( names[i] );
	gtk_signal_connect_object ( GTK_OBJECT ( item ), "activate",
				    GTK_SIGNAL_FUNC ( gsb_scheduler_list_change_scheduler_view ),
				    GINT_TO_POINTER(i) );
	gtk_menu_append ( GTK_MENU ( menu ), item );
    }

    gtk_widget_show_all ( menu );

    gtk_menu_popup ( GTK_MENU(menu), NULL, button, set_popup_position, button, 1, 
		     gtk_get_current_event_time());

    return FALSE;
}



/**
 * Create the toolbar that contains all elements needed to manipulate
 * the scheduler.
 *
 * \param 
 *
 * \return A newly created hbox.
 */
GtkWidget *creation_barre_outils_echeancier ( void )
{
    GtkWidget *hbox, *handlebox, *button;

    /* HandleBox + inner hbox */
    handlebox = gtk_handle_box_new ();
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(handlebox), hbox );

    /* Common actions */
    button = gsb_automem_imagefile_button_new ( etat.display_toolbar,
					       _("_New scheduled"),
					       "new-scheduled.png",
					       G_CALLBACK (gsb_scheduler_list_edit_transaction),
					       GINT_TO_POINTER(-1) );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Prepare form to create a new scheduled transaction"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    scheduler_button_delete = gsb_automem_stock_button_new ( etat.display_toolbar,
							    GTK_STOCK_DELETE, 
							    _("Delete"),
							    G_CALLBACK ( gsb_scheduler_list_delete_scheduled_transaction ),
							    NULL );
    gtk_widget_set_sensitive ( scheduler_button_delete,
			       FALSE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), scheduler_button_delete,
			   _("Delete selected scheduled transaction"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scheduler_button_delete, FALSE, FALSE, 0 );

    scheduler_button_edit = gsb_automem_stock_button_new ( etat.display_toolbar,
							  GTK_STOCK_PROPERTIES, 
							  _("Edit"),
							  G_CALLBACK ( gsb_scheduler_list_edit_transaction ),
							  0 );
    gtk_widget_set_sensitive ( scheduler_button_edit,
			       FALSE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), scheduler_button_edit,
			   _("Edit selected transaction"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scheduler_button_edit, FALSE, FALSE, 0 );

    /* Display/hide comments */
    scheduler_display_hide_comments = gsb_automem_imagefile_button_new ( etat.display_toolbar,
									_("Comments"),
									"comments.png",
									G_CALLBACK ( gsb_scheduler_list_show_notes ),
									0 );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), 
			   scheduler_display_hide_comments,
			   _("Display scheduled transactions comments"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scheduler_display_hide_comments, 
			 FALSE, FALSE, 0 );

    /* Execute transaction */
    scheduler_button_execute = gsb_automem_stock_button_new ( etat.display_toolbar,
							     GTK_STOCK_EXECUTE, 
							     _("Execute"),
							     G_CALLBACK ( gsb_scheduler_list_execute_transaction ),
							     NULL ); 
    gtk_widget_set_sensitive ( scheduler_button_execute,
			       FALSE );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), 
			   scheduler_button_execute,
			   _("Execute current scheduled transaction if it is at maturity date"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scheduler_button_execute, FALSE, FALSE, 0 );

    button = gsb_automem_stock_button_menu_new ( etat.display_toolbar,
						GTK_STOCK_SELECT_COLOR, _("View"),
						G_CALLBACK(popup_scheduled_view_mode_menu),
						NULL );
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips_general_grisbi ), button,
			   _("Change display mode of scheduled transaction list"), "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    gtk_widget_show_all ( handlebox );

    return ( handlebox );
}
/*******************************************************************************************/

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

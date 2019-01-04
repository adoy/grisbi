#ifndef _ETATS_CALCULS_H
#define _ETATS_CALCULS_H (1)

/* START_INCLUDE_H */
#include "etats_affiche.h"
/* END_INCLUDE_H */



/* START_DECLARATION */
gboolean	affichage_etat 							(gint report_number,
													 struct EtatAffichage *affichage,
													 gchar *filename);
gint 		cherche_string_equivalente_dans_slist 	(gchar *string_list,
													 gchar *string_cmp);
void 		denote_struct_sous_jaccentes 			(gint origine);
gboolean	rafraichissement_etat 					(gint report_number);
GSList *	recupere_opes_etat 						(gint report_number);
void 		affichage_empty_report 					(gint report_number);

/* END_DECLARATION */
#endif

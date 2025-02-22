/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2006-2006 Benjamin Drieu (bdrieu@april.org)           */
/*          http://www.grisbi.org                                             */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"

#include <glib/gi18n.h>
#include <libxml/parser.h>
#include <glib/gstdio.h>

/*START_INCLUDE*/
#include "gnucash.h"
#include "dialog.h"
#include "gsb_data_transaction.h"
#include "gsb_real.h"
#include "utils_str.h"
#include "utils_files.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef enum _GnucashCategoryType		GnucashCategoryType;

 enum _GnucashCategoryType
{
	GNUCASH_CATEGORY_INCOME,
	GNUCASH_CATEGORY_EXPENSE,
};

/* Structures */
struct GnucashCategory
{
	gchar *					name;
	GnucashCategoryType 	type;
	gchar *					guid;
};

struct GnucashSplit
{
	GsbReal				amount;
	gchar *				category;
	gchar *				account;
	gchar *				contra_account;
	gchar *				notes;
	gint					p_r;
};

/*START_STATIC*/
static GSList *			gnucash_accounts = NULL;
static GSList *			gnucash_categories = NULL;
static gchar *			gnucash_filename = NULL;
/*END_STATIC*/

/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean node_strcmp (xmlNodePtr node,
							 const gchar *name)
{
	if (node->name && strcmp ((gchar *) node->name, name) == 0)
		return TRUE;
	else
		return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static xmlNodePtr get_child (xmlNodePtr node,
							 const gchar *child_name)
{
	xmlNodePtr iter_node;

	if (!node)
		return NULL;

	iter_node = node->children;
	while (iter_node)
	{
		if (node_strcmp(iter_node, child_name))
			return iter_node;

		iter_node = iter_node->next;
	}

	return NULL;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gchar *child_content (xmlNodePtr node,
							 const gchar *child_name)
{
	xmlNodePtr child_node;

	if (!node)
		return NULL;

	child_node = get_child (node, child_name);
	if (child_node)
		return (gchar *) xmlNodeGetContent (child_node);

	return NULL;
}

/**
 * Utility functions that returns currency value of a currency node.
 *
 * \param currency_node		XML node to parse.
 *
 * \return string		Representation of currency
 **/
static gchar *get_currency (xmlNodePtr currency_node)
{
	return child_content (currency_node, "id");
}

/**
 * Find currently imported accounts according to their gnucash uid
 * (guid).
 *
 * \param guid		Textual guid of account to search.
 *
 * \return		A pointer to a ImportAccount or NULL upon failure.
 **/
static struct ImportAccount *find_imported_account_by_uid (gchar *guid)
{
	GSList *tmp_list;

	if (!guid)
		return NULL;

	tmp_list = gnucash_accounts;
	while (tmp_list)
	{
		struct ImportAccount *account = tmp_list->data;

		if (account->guid && !strcmp (account->guid, guid))
		{
			return account;
		}

		tmp_list = tmp_list->next;
	}

	return NULL;
}

/**
 * Find currently imported accounts according to their name.
 *
 * \param guid		Name of account to search.
 *
 * \return		A pointer to a ImportAccount or NULL upon failure.
 **/
static struct ImportAccount *find_imported_account_by_name (gchar *name)
{
	GSList *tmp_list;

	if (!name)
		return NULL;

	tmp_list = gnucash_accounts;
	while (tmp_list)
	{
		struct ImportAccount *account = tmp_list->data;

		if (!strcmp (account->nom_de_compte, name))
		{
			return account;
		}

		tmp_list = tmp_list->next;
	}

	return NULL;
}

/**
 * Find currently imported categories according to their gnucash uid
 * (guid).
 *
 * \param guid		Textual guid of category to search.
 *
 * \return		A pointer to a gnucah_category or NULL upon failure.
 **/
static struct GnucashCategory *find_imported_categ_by_uid (gchar *guid)
{
	GSList *tmp_list;

	tmp_list = gnucash_categories;
	while (tmp_list)
	{
		struct GnucashCategory *categ = tmp_list->data;

		if (!strcmp (categ->guid, guid))
		{
			return categ;
		}

		tmp_list = tmp_list->next;
	}

	return NULL;
}

/**
 * Parse a gnucash value representation and construct a numeric
 * representation of it. Gnucash values are always of the form:
 * "integer/number". Integer value is to be divided by another
 * integer, which eliminates float approximations.
 *
 * \param value		Number textual representation to parse.
 *
 * \return		Numeric value of argument 'value'.
 **/
static GsbReal gnucash_value (gchar *value)
{
	gchar **tab_value;
	gdouble mantisse;
	gdouble number;

	tab_value = g_strsplit (value, "/", 2);

	number = utils_str_atoi (tab_value[0]);
	mantisse = utils_str_atoi (tab_value[1]);

	g_strfreev (tab_value);

	return gsb_real_double_to_real (number / mantisse);
}

/**
 * Manually parse a gnucash file, tidy it, put result in a temporary
 * file and parse it via libxml.
 *
 * \param filename	Filename to parse.
 *
 * \return		A pointer to a xmlDocPtr containing XML representation of file.
 **/
static xmlDocPtr parse_gnucash_file (gchar *filename)
{
	FILE *file_in;
	FILE *tmp_file;
	gchar buffer[1024];
	gchar *tmp_filename;
	gchar *tmp_name;
	xmlDocPtr doc;

	file_in = utils_files_utf8_fopen (filename, "r");
	if (!file_in)
	{
		gchar *tmp_str;
		gchar *tmp_str2;

		tmp_str = g_strdup_printf (_("Either file \"%s\" does not exist or it is not a regular file."),
								   filename);
		tmp_str2 = g_strdup_printf (_("Error opening file '%s'."), filename);
		dialogue_error_hint (tmp_str, tmp_str2);

		g_free (tmp_str);
		g_free (tmp_str2);

		return NULL;
	}

	tmp_name = g_strdup_printf ("gsbgnc%05d", g_random_int_range (0,99999));
	tmp_file = utils_files_utf8_fopen (tmp_name, "w");
	if (!tmp_file)
	{
		gchar *tmp_str;
		gchar *tmp_str2;

		tmp_str = g_strdup (_("Grisbi needs to open a temporary file in order to import Gnucash data "
							  "but file can't be created.\n"
							  "Check that you have permission to do that."));
		tmp_str2 = g_strdup_printf (_("Error opening temporary file '%s'."), tmp_name);
		dialogue_error_hint (tmp_str, tmp_str2);

		g_free (tmp_str);
		g_free (tmp_str2);
		g_free (tmp_name);
		fclose (file_in);

		return NULL;
	}

	/* We need to create a temporary file because Gnucash writes XML
	 * files that do not respect the XML specification regarding
	 * namespaces.	We need to tidy XML file in order to let libxml
	 * handle it gracefully.
	 */
	while (fgets (buffer, 1024, file_in))
	{
		gchar *tag;

		tag = g_strrstr (buffer, "<gnc-v2>");
		if (tag)
		{
			const gchar **iter;
			const gchar *ns[14] = {"gnc", "cd", "book", "act", "trn", "split",
								   "cmdty", "ts", "slots", "slot", "price", "sx", "fs", NULL};

			tag += 7;
			*tag = 0;
			tag++;

			fputs (buffer, tmp_file);
			for (iter = ns ; *iter != NULL ; iter++)
			{
				gchar *header;

				header = g_strdup_printf (" xmlns:%s=\"http://www.gnucash.org/lxr/gnucash/source/src/doc/xml/%s-v1.dtd#%s\"\n",
										  *iter,
										  *iter,
										  *iter);
				fputs (header, tmp_file);
				g_free (header);
			}
			fputs (">\n", tmp_file);
		}
		else
		{
			fputs (buffer, tmp_file);
		}
	}

	fclose (file_in);
	fclose (tmp_file);

	tmp_filename = g_filename_from_utf8 (tmp_name, -1, NULL, NULL, NULL);
	doc = xmlParseFile(tmp_filename);
	g_free(tmp_filename);

	/* Once parsed, the temporary file is removed as it is useless. */
	if (g_unlink (tmp_name))
		important_debug("Unable to remove file");
	g_free(tmp_name);

	return doc;
}

/**
 * Find a split in a splits list according to a specific amount and
 * account or category. This is used to find splits pairs.
 *
 * \param split_list	Split list to lookup.
 * \param amount	Split amount to match against.
 * \param account	Account to match against.
 * \param categ		Category to match against.
 *
 * \return		A GnucashSplit upon success. NULL otherwise.
 **/
static struct GnucashSplit *find_split (GSList *split_list,
										GsbReal amount,
										struct ImportAccount *account,
										struct GnucashCategory *categ)
{
	GSList *tmp_list;

	(void)account;
	tmp_list = split_list;
	while (tmp_list)
	{
		struct GnucashSplit *split;

		split = tmp_list->data;
		if (!gsb_real_cmp (amount, gsb_real_opposite (split->amount))
			&& !(split->account && split->category)
			&& !(split->category && categ))
		{
			return split;
		}

		tmp_list = tmp_list->next;
	}

	return NULL;
}

/**
 * Update a split with arbitrary information according to their
 * correctness. If an account is specified and split already has an
 * account, this means it is the second split of the pair and first
 * split was an account split, so this is transfer split and we set
 * split's contra_account to 'account'.
 *
 * \param split		Split to update.
 * \param amount	Amount to set if account is not NULL.
 * \param account	Account to set if not NULL.
 * \param categ		Category to set if not NULL.
 *
 * \return
 **/
static void update_split (struct GnucashSplit *split,
						  GsbReal amount,
						  gchar *account,
						  gchar *categ)
{
	if (categ)
	{
		split->category = my_strdup (categ);
	}

	if (account)
	{
		if (!split->account)
		{
			split->account = my_strdup (account);
			split->amount = amount;
		}
		else
		{
			split->contra_account = my_strdup (account);
		}
	}
}

/**
 * Allocate a new split and set its values to arguments passed.
 *
 * \param amount	Split amount.
 * \param account	Split account.
 * \param categ		Split category.
 *
 * \return		A newly created GnucashSplit.
 **/
static struct GnucashSplit *new_split (GsbReal amount,
									   gchar *account,
									   gchar *categ)
{
	struct GnucashSplit *split;

	split = calloc (1, sizeof (struct GnucashSplit));

	split->amount = amount;
	if (account)
		split->account = my_strdup (account);
	else
		split->account = NULL;

	if (categ)
		split->category = my_strdup (categ);
	else
		split->category = NULL;

	return split;
}

/**
 * Allocate and return a ImportTransaction created from a
 * GnucashSplit and some arguments.
 *
 * \param split		Split to use as a base.
 * \param tiers		Transaction payee name.
 * \param date		Transaction date.
 *
 * \return 		A newly allocated ImportTransaction.
 **/
static struct ImportTransaction *new_transaction_from_split (struct GnucashSplit *split,
															 gchar *tiers,
															 GDate *date)
{
	struct ImportTransaction *transaction;

	/** Basic properties are set according to split. */
	transaction = calloc (1, sizeof (struct ImportTransaction));
	transaction->montant = split->amount;
	transaction->notes = split->notes;
	transaction->p_r = split->p_r;
	transaction->tiers = tiers;
	transaction->date = date;

	if (split->contra_account)
	{
		/** If split contains a contra account, then this is a transfer. */
		struct ImportAccount *contra_account;
		struct ImportTransaction *contra_transaction;

		contra_account = find_imported_account_by_name (split->contra_account);
		if (contra_account)
		{
			contra_transaction = calloc (1, sizeof (struct ImportTransaction));
			contra_transaction->montant = gsb_real_opposite (split->amount);
			contra_transaction->notes = split->notes;
			contra_transaction->tiers = tiers;
			contra_transaction->date = date;
			contra_transaction->p_r = split->p_r;

			transaction->categ = g_strconcat ("[", split->contra_account, "]", NULL);
			contra_transaction->categ = g_strconcat ("[", split->account, "]", NULL);

			contra_account->operations_importees = g_slist_append (contra_account->operations_importees,
																   contra_transaction);
		}
	}
	else
	{
		transaction->categ = split->category;
	}

	return transaction;
}

/**
 * Parse XML account node and fill a ImportAccount with
 * results. Add account to the global accounts list
 * gnucash_accounts.
 *
 * \param compte_node	XML account node to parse.
 *
 * \return
 **/
static void recuperation_donnees_gnucash_compte (xmlNodePtr compte_node)
{
	gchar *type;
	struct ImportAccount *compte;

	type = child_content (compte_node, "type");
	compte = calloc (1, sizeof (struct ImportAccount));

	/* Gnucash import */
	compte->origine = my_strdup ("Gnucash");

	if (!strcmp(type, "BANK") || !strcmp(type, "CREDIT"))
	{
		compte->type_de_compte = 0; /* Bank */
	}
	else if (!strcmp(type, "CASH") || !strcmp(type, "CURRENCY"))
	{
		compte->type_de_compte = 1; /* Currency */
	}
	else if (!strcmp(type, "ASSET") || !strcmp(type, "STOCK") || !strcmp(type, "MUTUAL"))
	{
		compte->type_de_compte = 0; /* Asset */
	}
	else if (!strcmp(type, "LIABILITY"))
	{
		compte->type_de_compte = 0; /* Liability */
	}

	compte->nom_de_compte = child_content (compte_node, "name");
	compte->filename = gnucash_filename;
	compte->solde = null_real;
	compte->devise = get_currency (get_child(compte_node, "commodity"));
	compte->guid = child_content (compte_node, "id");
	compte->operations_importees = NULL;

	compte->nom_de_compte = gsb_import_unique_imported_name (compte->nom_de_compte);

	gsb_import_register_account (compte);

	gnucash_accounts = g_slist_append (gnucash_accounts, compte);
}

/**
 * Parse XML category node and fill a GnucashCategory with results.
 * Add category to the global category list.
 *
 * \param categ_node	XML category node to parse.
 *
 * \return
 **/
static void recuperation_donnees_gnucash_categorie (xmlNodePtr categ_node)
{
	struct GnucashCategory *categ;

	categ = calloc (1, sizeof (struct GnucashCategory));

	/* Find name, could be tricky if there is a parent. */
	categ->name = child_content (categ_node, "name");
	if (child_content (categ_node, "parent"))
	{
		gchar *parent_guid;
		GSList *tmp_list;

		parent_guid = child_content (categ_node, "parent");
		tmp_list = gnucash_categories;
		while (tmp_list)
		{
			struct GnucashCategory *iter;

			iter = tmp_list->data;
			if (!strcmp (iter->guid, parent_guid))
			{
				categ->name = g_strconcat (iter->name, " : ", categ->name, NULL);
				break;
			}

			tmp_list = tmp_list->next;
		}
	}

	categ->guid = child_content (categ_node, "id");

	/* Find if this is an expense or income category. */
	if (!strcmp (child_content (categ_node, "type"), "INCOME"))
	{
		categ->type = GNUCASH_CATEGORY_INCOME;
	}
	else
	{
		categ->type = GNUCASH_CATEGORY_EXPENSE;
	}

	gnucash_categories = g_slist_append (gnucash_categories, categ);
}

/**
 * Parse XML transaction node and fill a ImportTransaction with results.
 *
 * \param transaction_node	XML transaction node to parse.
 *
 * \return
 **/
static void recuperation_donnees_gnucash_transaction (xmlNodePtr transaction_node)
{
	GSList *split_list = NULL;
	GDate *date;
	gchar *date_string;
	gchar *space;
	gchar *tiers;
	xmlNodePtr date_node;
	xmlNodePtr split_node;
	xmlNodePtr splits;
	struct ImportTransaction *transaction;
	struct ImportAccount *account = NULL;
	struct GnucashSplit *split;
	GsbReal total = { 0 , 0 };

	/* Transaction amount, category, account, etc.. */
	splits = get_child (transaction_node, "splits");
	split_node = splits->children;

	while (split_node)
	{
		gint p_r = OPERATION_NORMALE;
		struct ImportAccount *split_account = NULL;
		struct GnucashCategory *categ = NULL;
		GsbReal amount;

		/* Gnucash transactions are in fact "splits", much like grisbi's
		 * splits of transactions. We need to parse all splits and
		 * see whether they are transfers to real accounts or transfers
		 * to category accounts. In that case, we only create one
		 * transactions. The other is discarded as grisbi is not a
		 * double part financial engine.
		 */
		if (node_strcmp (split_node, "split"))
		{
			gchar *account_name = NULL;
			gchar *categ_name = NULL;

			split_account = find_imported_account_by_uid (child_content (split_node, "account"));
			categ = find_imported_categ_by_uid (child_content (split_node, "account"));
			amount = gnucash_value (child_content(split_node, "value"));

			if (categ)
				categ_name = categ->name;
			if (split_account)
			{
				/* All of this stuff is here since we are dealing with
				the account split, not the category one */
				account_name = split_account->nom_de_compte;
				total = gsb_real_add (total, amount);
				if (strcmp(child_content(split_node, "reconciled-state"), "n"))
					p_r = OPERATION_RAPPROCHEE;
			}

			split = find_split (split_list, amount, split_account, categ);
			if (split)
			{
				update_split (split, amount, account_name, categ_name);
			}
			else
			{
				split = new_split (amount, account_name, categ_name);
				split_list = g_slist_append (split_list, split);
				split->notes = child_content(split_node, "memo");
			}
			if (p_r != OPERATION_NORMALE)
				split->p_r = p_r;
		}

		split_node = split_node->next;
	}

	if (!split_list)
		return;

	/* Transaction date */
	date_node = get_child (transaction_node, "date-posted");
	date_string = child_content (date_node, "date");
	space = strchr (date_string, ' ');
	if (space)
		*space = 0;
	date = g_date_new ();
	g_date_set_parse (date, date_string);
	if (!g_date_valid (date))
		fprintf (stderr, "grisbi: Can't parse date %s\n", date_string);

	/* Tiers */
	tiers = child_content (transaction_node, "description");

	/* Create transaction */
	split = split_list->data;
	transaction = new_transaction_from_split (split, tiers, date);
	transaction->operation_ventilee = 0;
	transaction->ope_de_ventilation = 0;
	account = find_imported_account_by_name (split->account);
	if (account)
		account->operations_importees = g_slist_append (account->operations_importees, transaction);
	else
	{
		gsb_import_free_transaction (transaction);
		transaction = NULL;
	}

	/* Splits of transactions are handled the same way, we process
	   them if we find more than one split in transaction node. */
	if (g_slist_length (split_list) > 1)
	{
		if (transaction)
		{
			transaction->operation_ventilee = 1;
			transaction->montant = total;
		}

		while (split_list)
		{
			split = split_list->data;
			account = NULL;

			transaction = new_transaction_from_split (split, tiers, date);
			transaction->ope_de_ventilation = 1;

			account = find_imported_account_by_name (split->account);
			if (account)
				account->operations_importees = g_slist_append (account->operations_importees, transaction);
			else
				gsb_import_free_transaction (transaction);

			split_list = split_list->next;
		}
	}
}

/**
 * Parse XML book nodes from a gnucash file.
 *
 * Main role of this function is to iterate over XML nodes of the file
 * and determine which account nodes are category nodes and which are
 * real accounts, as in Gnucash, accounts and categories are mixed.
 *
 * \param book_node	Pointer to current XML node.
 *
 * \return
 **/
static void recuperation_donnees_gnucash_book (xmlNodePtr book_node)
{
	xmlNodePtr child_node;

	child_node = book_node->children;

	while (child_node)
	{
		/* Books are subdivisions of gnucash files */
		if (node_strcmp (child_node, "book"))
		{
			recuperation_donnees_gnucash_book (child_node);
		}

		if (node_strcmp (child_node, "account"))
		{
			gchar *type;

			type = child_content (child_node, "type");
			if (strcmp (type, "INCOME") && strcmp (type, "EXPENSE")
				&& strcmp (type, "EXPENSES") && strcmp (type, "EQUITY"))
			{
				recuperation_donnees_gnucash_compte (child_node);
			}
			else
			{
				recuperation_donnees_gnucash_categorie (child_node);
			}
		}

		if (node_strcmp (child_node, "transaction"))
		{
			recuperation_donnees_gnucash_transaction (child_node);
		}

		child_node = child_node->next;
	}
}

/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/**
 * Parse specified file as a Gnucash file and construct necessary data
 * structures with result. Use a custom XML parser.
 *
 * \param filename	File to parse.
 *
 * \return TRUE upon success. FALSE otherwise.
 **/
gboolean recuperation_donnees_gnucash (GtkWidget *assistant,
									   struct ImportFile *imported)
{
	xmlDocPtr doc;
	struct ImportAccount *account;

	(void)assistant;
	gnucash_filename = my_strdup (imported->name);
	doc = parse_gnucash_file (gnucash_filename);

	gnucash_accounts = NULL;
	if (doc)
	{
		xmlNodePtr root;

		root = xmlDocGetRootElement(doc);
		if (root)
		{
			recuperation_donnees_gnucash_book (root);

			return TRUE;
		}
	}

	/* So, we failed to import file. */
	account = g_malloc0 (sizeof (struct ImportAccount));
	account->origine = _("Gnucash");
	account->nom_de_compte = _("Invalid Gnucash account, please check gnucash file is not compressed.");
	account->filename = my_strdup (imported->name);

	gsb_import_register_account_error (account);

	return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

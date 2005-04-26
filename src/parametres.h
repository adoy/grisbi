/*START_DECLARATION*/
void preferences ( gint page );
/*END_DECLARATION*/


/* Preference pages */
enum preference_page  {
    NOT_A_PAGE = -1,
    FILES_PAGE,
    IMPORT_PAGE,
    SOFTWARE_PAGE,
    FONTS_AND_LOGO_PAGE,
    MESSAGES_AND_WARNINGS_PAGE,
    ADDRESSES_AND_TITLES_PAGE,
    TOTALS_PAGE,
    TOOLBARS_PAGE,
    TRANSACTIONS_LIST_PAGE,
    TRANSACTIONS_PAGE,
    TRANSACTION_FORM_PAGE,
    TRANSACTION_FORM_BEHAVIOR_PAGE,
    RECONCILIATION_PAGE,
    CURRENCIES_PAGE,
    BANKS_PAGE,
    FINANCIAL_YEARS_PAGE,
    METHODS_OF_PAYMENT_PAGE,
    NUM_PREFERENCES_PAGES
};


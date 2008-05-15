/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Joerg Langenberg <joerg.langenberg@gmx.net>
 *
 */
#include <main/argopts.h>

#define ARGOPTS_MAX_ENTRIES 127

//#define ARG_DEBUG
#ifdef ARG_DEBUG
# define ADBG(fmt,args...) printf("ARGOPTS: "fmt,##args)
#else
# define ADBG(fmt,args...)
#endif

struct argTab_longTab {
    int             index;
    argTable_t*     tab;
};

static char arg_classname[50];

void printAllArgs(argDescriptor_t *p_argdesc)
{
    argTable_t *descTab;
    int table = 0;
    int tabidx = 0;

    while (p_argdesc[table].tab)
    {
        descTab = p_argdesc[table].tab;
        tabidx = 0;

        while (descTab[tabidx].name.length() != 0)
        {
            if (descTab[tabidx].has_val != ARGOPT_NOVAL)  // print out values
                switch (descTab[tabidx].val_type)
                {
                    case ARGOPT_VAL_INT:
                        printf(" %-18s: %d\n", descTab[tabidx].name.c_str(),
                               descTab[tabidx].val.i);
                        break;

                    case ARGOPT_VAL_STR:
                        printf(" %-18s: %s\n", descTab[tabidx].name.c_str(),
                               descTab[tabidx].val.s);
                        break;

                    case ARGOPT_VAL_FLT:
                        printf(" %-18s: %f\n", descTab[tabidx].name.c_str(),
                               descTab[tabidx].val.f);
                        break;
                }
            else    // arguments no not have values
                printf(" %-18s\n", descTab[tabidx].name.c_str());

            tabidx++;
        }
        table++;
    }
    printf("\n");
}

void argUsage(argDescriptor_t *p_argdesc)
{
    const char *val_type_name[3] = { "int", "str", "flt" };
    argTable_t *descTab;
    int table = 0;
    int tabidx = 0;

    printf("\n");
    printf(" --> start this driver with the following options\n");
    printf("\n");
    printf("R = Option Required, A = Option needs Argument\n");
    printf("n = no, y = yes, o = optional\n");
    printf("\n");
//         |        20          |  5  | 3 | 3 |
    printf("      Option        | Typ | R | A | Help string \n");
    printf("----------------------------------------------------------------\n");
    while (p_argdesc[table].tab)
    {
        descTab = p_argdesc[table].tab;
        tabidx = 0;

        while (descTab[tabidx].name.length() != 0)
        {

            printf(" %-18s | %3s | %1s | %1s | %s \n",
                   descTab[tabidx].name.c_str(),
                   val_type_name[descTab[tabidx].val_type],
                   descTab[tabidx].arg_type == ARGOPT_REQ ?
                                               "y" : "n",
                   descTab[tabidx].has_val  == ARGOPT_NOVAL  ?
                                               "n" :
                               descTab[tabidx].has_val  == ARGOPT_REQVAL ?
                                               "y" :
                                               "o",
                   descTab[tabidx].help.c_str());
              tabidx++;
        }
        table++;
    }
    printf("\n");
}

struct option         long_option[ARGOPTS_MAX_ENTRIES];
struct argTab_longTab atlt[ARGOPTS_MAX_ENTRIES];

int argScan(int argc, char *argv[], argDescriptor_t *p_argdesc,
            const char *classname)
{
    int option_index = 0;
    int tab;
    int tab_entries = 0;
    int tab_entry   = 0;
    int tabidx = 0;
    int tables = 0;
    int c;

    strncpy(arg_classname, classname, 50);

    printf("\n");
    printf(" ---=== %s ===---\n", arg_classname);
    printf("\n");

    // get number of tables
    tables = 0;
    while (p_argdesc[tables].tab)
    {
        ADBG("table @ %p\n", p_argdesc[tables].tab);
        tables++;
    }
    ADBG("%d tables found\n", tables);

    // get complete number of table entries
    tab_entries = 0;
    for (tab=0; tab <tables; tab++)
    {
        argTable_t *descTab = p_argdesc[tab].tab;
        tabidx = 0;

        while (descTab[tabidx++].name.length() != 0)
          tab_entries++;
    }

    ADBG("%d entries found in all tables\n", tab_entries);

    // create long_option table
    if(tab_entries > ARGOPTS_MAX_ENTRIES)
    {
        printf("reduce arg table to %d entries\n", ARGOPTS_MAX_ENTRIES);
        tab_entries = ARGOPTS_MAX_ENTRIES;
    }

    tab_entry = 0;
    for (tab=0; tab <tables; tab++)
    {
        argTable_t *descTab = p_argdesc[tab].tab;
        tabidx = 0;

        while (descTab[tabidx].name.length() != 0)
        {
            long_option[tab_entry].name    = descTab[tabidx].name.c_str();
            long_option[tab_entry].has_arg = descTab[tabidx].has_val;
            long_option[tab_entry].flag    = NULL;
            long_option[tab_entry].val     = 0;

            atlt[tab_entry].index = tabidx;
            atlt[tab_entry].tab   = descTab;

            ADBG("*** NEW OPTION *** \n");
            ADBG("index table    : %d \n", tabidx);
            ADBG("index complete : %d \n", tab_entry);
            ADBG("required       : %s \n", descTab[tabidx].arg_type == ARGOPT_REQ ?
                                          "yes" : "no");
            ADBG("option name    : %s \n", descTab[tabidx].name.c_str());
            ADBG("help string    : %s \n", descTab[tabidx].help.c_str());
            ADBG("value          : %s \n", descTab[tabidx].has_val == ARGOPT_NOVAL  ?
                                           "not needed" :
                                           descTab[tabidx].has_val == ARGOPT_REQVAL ?
                                           "required" : "optional");
            ADBG("value type     : %s \n", descTab[tabidx].val_type == ARGOPT_VAL_INT ?
                                           "Integer" : "String");
            ADBG("value default  : %d \n", descTab[tabidx].val.i);
            ADBG("value address  : %p \n", &descTab[tabidx].val);

            tab_entry++;
            tabidx++;
        }
    }

    // add --help
    long_option[tab_entry].name    = "help";
    long_option[tab_entry].has_arg = ARGOPT_NOVAL;
    long_option[tab_entry].flag    = NULL;
    long_option[tab_entry].val     = 0;
    tab_entry++;

    // last table entry
    long_option[tab_entry].name    = NULL;
    long_option[tab_entry].has_arg = 0;
    long_option[tab_entry].flag    = NULL;
    long_option[tab_entry].val     = 0;

    while (1)
    {
        c = getopt_long_only(argc, argv, "", long_option, &option_index);

        if (c == -1)        // no more options
        {
            // check all required values
            int i = 0;
            int error = 0;
            while ( i < tab_entries)
            {
                argTable_t *descTab = atlt[i].tab;
                int index = atlt[i].index;
                if (descTab[index].arg_type == ARGOPT_REQ)
                {
                    printf("ERROR, option --%s is missing\n",
                           descTab[index].name.c_str());
                    error++;
                }
                i++;
            }
            if (error)
            {
                argUsage(p_argdesc);
                return -EARGOPT_MISSED;
            }

            // success -> print out all values
            printAllArgs(p_argdesc);
            return 0;
        }

        if (c != 0)
        {
            printf("Invalid option -> exit \n");
            argUsage(p_argdesc);
            return -EARGOPT_INVAL;
        }

        // c == 0
        ADBG("option %s found, index = %d, value = %s\n",
             long_option[option_index].name, option_index,
             optarg ? optarg : "<no value>");

        // check help string
        if (!strcmp(long_option[option_index].name, "help"))
        {
            argUsage(p_argdesc);
            return -EINVAL;
        }

        // check if integer value
        argTable_t *descTab = atlt[option_index].tab;
        int index = atlt[option_index].index;

        switch (descTab[index].val_type) {
            case ARGOPT_VAL_INT:
                descTab[index].val.i = atoi(optarg);
                ADBG("type int, writing %d @ 0x%p\n",
                     descTab[index].val.i, &descTab[index].val);
                break;

            case ARGOPT_VAL_STR:
                descTab[index].val.s = optarg;
                break;

            case ARGOPT_VAL_FLT:
                descTab[index].val.f = atof(optarg);
                ADBG("type float, writing %f @ 0x%p\n",
                     descTab[index].val.f, &descTab[index].val);
                break;

            default:
                printf("Invalid option type -> exit \n");
                return -EINVAL;
        }

        // option required no more
        descTab[index].arg_type = 0;

    }
    return -1;
}

arg_value_t __getArg(const char* argname, argTable_t *p_tab)
{
    int idx = 0;
    while (p_tab[idx].name != "")
    {
        if (!strcmp(argname, p_tab[idx].name.c_str()))
              return p_tab[idx].val;
        idx++;
    }
    printf("\n");
    printf("WARNING: %s: getArg -> Invalid argname %s\n", arg_classname, argname);

    return (arg_value_t){ 0 };
}

/* NUMIN.C - Module for reading numeric input. */

/* Copyright (c) 1995-2004 by Radford M. Neal
 *
 * Permission is granted for anyone to copy, use, modify, or distribute this
 * program and accompanying programs and documents for any purpose, provided
 * this copyright notice is retained and prominently displayed, along with
 * a note saying that the original programs are available from Radford Neal's
 * web page, and note is made of any changes made to the programs.  The
 * programs and documents are distributed without any warranty, express or
 * implied.  As the programs were written for research purposes only, they have
 * not been tested to the degree that would be advisable in any important
 * application.  All use of these programs is entirely at the user's own risk.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "numin.h"

#include "CTFile.h"
#include "CTTree.h"

/* SPECIFY SOURCE OF NUMERIC INPUT.  The caller passes the specification
   string and the number of items required.  This procedure does not attempt
   to access the file, and so may be called simply to set defaults.  This should
   be done to initialize the structure before calling this procedure with a
   user-supplied specification; typically the specification "data@1,0" with
   n_items of one is used to set the defaults. */

void numin_spec(numin_source *ns, /* Structure holding numeric input specification */
                char *spec,       /* Specification of file and item indexes */
                int n_items       /* Number of values needed */
)
{
  char *s, *f;
  int i, j, n;

  s = spec;

  /* Stash away number of items. */

  if (n_items < 0)
  {
    fprintf(stderr, "Asking for negative number of items from a line!\n");
    exit(1);
  }

  ns->N_items = n_items;

  if (ns->N_items > Max_items)
  {
    fprintf(stderr,
            "Asking for too many items from a line (max %d)\n", Max_items);
    exit(1);
  }

  /* Check for all defaults. */

  if (strcmp(s, ".") == 0)
  {
    for (i = 0; i < ns->N_items; i++) // store in ns->index[] indexes of ns->N_items items, starting from last used
    {
      ns->index[i] = ns->last_index = ns->last_index + 1;
    }

    goto setup;
  }

  /* Look for file name, reset defaults if one is present. */

  if (*s != 0 && *s != '@' && *s != ',') // filename is specified
  {
    f = ns->filename;
    while (*s != 0 && *s != '@' && *s != ',')
      *f++ = *s++; // copy it to 'ns' structure
    *f = 0;
    ns->complement = 0;
    ns->first = 1;
    ns->last = 0;
    ns->last_index = 0;
  }

  /* Parse line range. */

  if (*s == '@')
  {
    s += 1;
    ns->complement = 0;
    if (*s == '-')
    {
      s += 1;
      ns->complement = 1;
    }
    if (*s >= '0' && *s <= '9')
    {
      ns->first = atoi(s); // atoi stops at first character that is not a valid digit
      if (ns->first <= 0)
        goto error;
      while (*s >= '0' && *s <= '9')
        s += 1;
      ns->last = 0;
      if (*s == ':')
      {
        s += 1;
        if (*s >= '0' && *s <= '9')
        {
          ns->last = atoi(s);
          if (ns->last < ns->first)
            goto error;
          while (*s >= '0' && *s <= '9')
            s += 1;
        }
      }
    }
    if (*s != 0 && *s != ',')
      goto error;
  }

  /* Parse list of indexes. */

  for (i = 0; i < ns->N_items && *s == ','; i++)
  {
    s += 1;
    if (*s < '0' || *s > '9')
      goto error;
    ns->index[i] = ns->last_index = atoi(s);
    while (*s >= '0' && *s <= '9')
      s += 1;
  }

  for (; i < ns->N_items; i++)
  {
    ns->index[i] = ns->last_index = ns->last_index + 1;
  }

  if (*s != 0)
    goto error;

  /* Set up other form of index. */

setup:
  n = 0;
  for (i = 0; i < ns->N_items; i++) // sort of ns->iused and ns->ifor so that indexes in ns->iused are ascending
  {
    if (ns->index[i] != 0)
    {
      for (j = n; j > 0 && ns->iused[j - 1] > ns->index[i]; j--)
      {
        ns->iused[j] = ns->iused[j - 1];
        ns->ifor[j] = ns->ifor[j - 1];
      }
      ns->iused[j] = ns->index[i];
      ns->ifor[j] = i;
      n += 1;
    }
  }
  ns->iused[n] = 0;

  return;

error:
  fprintf(stderr, "Bad file/index/range specification: %s\n", spec);
  exit(1);
}

/* START READING A FILE OF NUMERIC INPUT.  The specification must have
   been set up with a call of numin_spec.  The number of lines that will be
   read from the file is returned. */

int numin_start(numin_source *ns /* Structure holding numeric input specification */
)
{
  /* Check if file given is a ROOT file. */
  char *ch = strstr(ns->filename, ".root");

  if (ch && ch[5] == ':' && ch[6] == '/')
  // ROOT file name must be of the form file.root:/dir/tree (directories are optional)
  {
    ns->ROOT_input = 1; // yes, it looks like ROOT file

    char just_filename[100]; // ROOT file name with in-file directories and tree stripped off
    short i = -1;

    while (ns->filename[++i] != ':')
      just_filename[i] = ns->filename[i];

    just_filename[i] = '\0';

    /* Open ROOT file. */
    ns->ROOT_file = CTFile_TFile(just_filename, "READ");

    if (CTFile_IsZombie(ns->ROOT_file))
    {
      fprintf(stderr, "Cannot open ROOT file %s\n", just_filename);
      exit(1);
    }

    /* Open tree. */
    ns->ROOT_tree = CTFile_Get(ns->ROOT_file, ch + 7);

    if (!ns->ROOT_tree)
    {
      fprintf(stderr, "Cannot retrieve tree %s from ROOT file\n", ch + 7);
      exit(1);
    }

    /* Set up the rest fields of 'ns' structure. */
    ns->length = CTTree_GetEntries(ns->ROOT_tree);

    if (ns->last > ns->length || ns->last == 0 && ns->first > ns->length + 1)
    {
      fprintf(stderr, "Range of entries specified is not present in file\n");
      exit(1);
    }

    if (ns->last == 0)
      ns->last = ns->length;

    ns->line = 1;

    /* Dealing with branches. */
    short n_branches = CTTree_GetNbranches(ns->ROOT_tree);

    // Allocate buffer for branch names
    char **bnames = (char **)calloc(n_branches, sizeof(char *));

    for (i = 0; i < n_branches; i++)
      bnames[i] = (char *)calloc(100, sizeof(char));

    CTTree_GetListOfBranchNames(ns->ROOT_tree, bnames); // read branch names

    // Allocate buffer to read from specified branches
    ns->ROOT_buffer = (double *)calloc(ns->N_items, sizeof(double));

    short j = 0;

    /*for (i = 0; i < n_branches; i++)
      if (i + 1 == ns->iused[j])  // if branch index was in argument list (note, the latter starts with 1, not 0)...
      {
        // ...set associated variable for it...
        CTTree_SetBranchAddress(ns->ROOT_tree, bnames[i], &(ns->ROOT_buffer[j]));
        j++;
      }
      else
        CTTree_SetBranchStatus(ns->ROOT_tree, bnames[i], 0);  // ...otherwise turn the branch off*/

    CTTree_SetBranchStatus(ns->ROOT_tree, "*", 0); // turn all the branches off

    for (i = 0; i < ns->N_items; i++)
    {
      CTTree_SetBranchStatus(ns->ROOT_tree, bnames[ns->index[i] - 1], 1);
      CTTree_SetBranchAddress(ns->ROOT_tree, bnames[ns->index[i] - 1], &(ns->ROOT_buffer[i]));
      /* I.e. buffer corresponds to branches in the order they were given in arguements. */
    }

    // Buffer for branch names is no longer needed
    for (i = 0; i < n_branches; i++)
      free(bnames[i]);

    free(bnames);

    return ns->last - ns->first + 1;
    // ROOT files do not support omitting range, so ns->complement is effectively zero
  }

  /* If this code is executed, input file is usual ASCII. */
  ns->ROOT_input = 0;

  int c;

  /* Open file and count lines. */

  if (*ns->filename == '%')
  {
    ns->file = popen(ns->filename + 1, "r");
  }
  else
  {
    ns->file = fopen(ns->filename, "r");
  }

  if (ns->file == NULL)
  {
    fprintf(stderr, "Can't open %s\n", ns->filename);
    exit(1);
  }

  ns->length = 0;
  c = getc(ns->file);

  while (c != EOF && (ns->complement || ns->last == 0 || ns->length < ns->last))
  {
    ns->length += 1;
    while (c != EOF && c != '\n')
      c = getc(ns->file);
    c = getc(ns->file);
  }

  /* Check range with file length. */

  if (ns->last > ns->length || ns->last == 0 && ns->first > ns->length + 1)
  {
    fprintf(stderr, "Range of lines specified is not present in file\n");
    exit(1);
  }

  if (ns->last == 0)
    ns->last = ns->length;

  if (*ns->filename == '%')
  {
    pclose(ns->file);
    ns->file = popen(ns->filename + 1, "r");
  }
  else
  {
    rewind(ns->file);
  }

  ns->line = 1;

  return ns->complement ? ns->length - (ns->last - ns->first + 1)
                        : ns->last - ns->first + 1;
}

/* READ THE NEXT LINE.  The number of items required (as passed to numin_start)
   are read from the next line of the file, and stored in the array of doubles
   passed.  The caller must stop reading before the end of file is reached
   (based on the line count returned from numin_start).  Missing values are
   represented by NaN. */

void numin_read(numin_source *ns, /* Structure holding numeric input specification */
                double *p         /* Place to store values read, or null to discard */
)
{
  /* ROOT support.*/
  if (ns->ROOT_input && p)
  {
    for (; ns->line < ns->first; ns->line++) // skip lines below given range
      CTTree_GetEntry(ns->ROOT_tree, ns->line - 1);

    CTTree_GetEntry(ns->ROOT_tree, ns->line - 1); // fill the buffer (lines start with 1)

    short i;

    for (i = 0; i < ns->N_items; i++) // copy to destination
      p[i] = ns->ROOT_buffer[i];

    ns->line++;

    return;
  }

  /* If code here is executed, input file is usual ASCII. */
  char item[101];
  int i, j, c, n, k;
  double value;

  while (ns->complement ? ns->line >= ns->first && ns->line <= ns->last
                        : ns->line < ns->first)
  {
    c = getc(ns->file);
    if (c == EOF)
      goto eof;
    if (c == '\n')
      ns->line += 1;
  }

  if (ns->complement ? ns->line > ns->length : ns->line > ns->last)
  {
    fprintf(stderr, "Reading too much in numin_read!\n");
    exit(1);
  }

  i = 1;
  n = 0;

  c = getc(ns->file);
  if (c == EOF)
    goto eof;

  for (;;)
  {
    while (c == ' ' || c == '\t' || c == ',' || c == ';')
      c = getc(ns->file);
    if (c == EOF || c == '\n')
      break;
    j = 0;
    while (c != EOF && c != '\n' && c != ' ' && c != '\t' && c != ',' && c != ';')
    {
      if (j < 100)
        item[j++] = c;
      c = getc(ns->file);
    }
    item[j] = 0;

    if (ns->iused[n] == i)
    {
      if (strcmp(item, "?") == 0)
      {
        value = 0.0 / 0.0;
      }
      else if (sscanf(item, "%lf", &value) != 1)
      {
        fprintf(stderr, "Bad numeric item on line %d of %s: %s\n",
                ns->line, ns->filename, item);
        exit(1);
      }
      while (ns->iused[n] == i)
      {
        if (p != 0)
          p[ns->ifor[n]] = value;
        n += 1;
      }
    }

    i += 1;
  }

  for (k = 0; k < ns->N_items; k++)
  {
    if (ns->index[k] == 0)
    {
      if (p != 0)
        p[k] = 0;
      n += 1;
    }
  }

  if (n != ns->N_items)
  {
    fprintf(stderr, "Line %d of %s is missing one or more required items\n",
            ns->line, ns->filename);
    exit(1);
  }

  ns->line += 1;

  return;

eof:
  fprintf(stderr, "Unexpectedly hit EOF!\n");
  exit(1);
}

/* CLOSE THE FILE.  Closes the file from which numeric input is being read. */

void numin_close(numin_source *ns /* Structure holding numeric input specification */
)
{
  if (ns->ROOT_input)
  {
    free(ns->ROOT_buffer);
    CTFile_Close(ns->ROOT_file);
  }
  else if (*ns->filename == '%')
  {
    pclose(ns->file);
  }
  else
  {
    fclose(ns->file);
  }
}

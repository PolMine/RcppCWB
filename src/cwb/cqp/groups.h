/*
 *  IMS Open Corpus Workbench (CWB)
 *  Copyright (C) 1993-2006 by IMS, University of Stuttgart
 *  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2, or (at your option) any later
 *  version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details (in the file "COPYING", or available via
 *  WWW at http://www.gnu.org/copyleft/gpl.html).
 */

#ifndef _cqp_groups_h_
#define _cqp_groups_h_

#include <stdio.h>

#include "../cl/cl.h"

#include "corpmanag.h"
#include "output.h"

#define SEPARATOR  "#---------------------------------------------------------------------\n" /**< used by ascii_print_group */

#define ANY_ID -2


typedef struct _id_cnt_mapping {
  int s, t, freq, s_freq;
} ID_Count_Mapping;

typedef struct _grouptable {
  CorpusList *my_corpus;

  Attribute *source_attribute;
  int source_is_struc;
  char *source_base;
  FieldType source_field;
  int source_offset;

  Attribute *target_attribute;
  int target_is_struc;
  char *target_base;
  FieldType target_field;
  int target_offset;

  Attribute *within_attribute;

  int cutoff_frequency;
  int is_grouped;

  int nr_cells;
  ID_Count_Mapping *count_cells;
} Group;

Group *compute_grouping(CorpusList *cl,
                        FieldType source_field,
                        int source_offset,
                        const char *source_attr_name,
                        FieldType target_field,
                        int target_offset,
                        const char *target_attr_name,
                        int cutoff_freq,
                        int is_grouped,
                        const char *within);

void free_group(Group **group);

void print_group(Group *group, struct Redir *rd);

const char *Group_id2str(Group *group, int i, int target);


#endif

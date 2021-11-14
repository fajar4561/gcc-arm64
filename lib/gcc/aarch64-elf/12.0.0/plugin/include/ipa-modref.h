/* Search for references that a functions loads or stores.
   Copyright (C) 2019-2021 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef IPA_MODREF_H
#define IPA_MODREF_H

typedef modref_tree <alias_set_type> modref_records;
typedef unsigned short eaf_flags_t;

/* Single function summary.  */

struct GTY(()) modref_summary
{
  /* Load and stores in function (transitively closed to all callees)  */
  modref_records *loads;
  modref_records *stores;
  auto_vec<eaf_flags_t> GTY((skip)) arg_flags;
  eaf_flags_t retslot_flags;
  eaf_flags_t static_chain_flags;
  unsigned writes_errno : 1;
  unsigned side_effects : 1;
  /* Flags coputed by finalize method.  */
  unsigned global_memory_read : 1;
  unsigned global_memory_written : 1;
  unsigned try_dse : 1;


  modref_summary ();
  ~modref_summary ();
  void dump (FILE *);
  bool useful_p (int ecf_flags, bool check_flags = true);
  void finalize (tree);
};

modref_summary *get_modref_function_summary (cgraph_node *func);
void ipa_modref_c_finalize ();
void ipa_merge_modref_summary_after_inlining (cgraph_edge *e);

/* All flags that are implied by the ECF_CONST functions.  */
static const int implicit_const_eaf_flags
   = EAF_NO_DIRECT_CLOBBER | EAF_NO_INDIRECT_CLOBBER
    | EAF_NO_DIRECT_ESCAPE | EAF_NO_INDIRECT_ESCAPE
    | EAF_NO_DIRECT_READ | EAF_NO_INDIRECT_READ
    | EAF_NOT_RETURNED_INDIRECTLY;

/* All flags that are implied by the ECF_PURE function.  */
static const int implicit_pure_eaf_flags
   = EAF_NO_DIRECT_CLOBBER | EAF_NO_INDIRECT_CLOBBER
    | EAF_NO_DIRECT_ESCAPE | EAF_NO_INDIRECT_ESCAPE;

/* All flags implied when we know we can ignore stores (i.e. when handling
   call to noreturn).  */
static const int ignore_stores_eaf_flags
   = EAF_NO_DIRECT_CLOBBER | EAF_NO_INDIRECT_CLOBBER
    | EAF_NO_DIRECT_ESCAPE | EAF_NO_INDIRECT_ESCAPE;

/* Return slot is write-only.  */
static const int implicit_retslot_eaf_flags
   = EAF_NO_DIRECT_READ | EAF_NO_INDIRECT_READ
     | EAF_NO_INDIRECT_ESCAPE | EAF_NO_INDIRECT_CLOBBER
     | EAF_NOT_RETURNED_INDIRECTLY;

/* If function does not bind to current def (i.e. it is inline in comdat
   section), the modref analysis may not match the behaviour of function
   which will be later symbol interposed to.  All side effects must match
   however it is possible that the other function body contains more loads
   which may trap.
   MODREF_FLAGS are flags determined by analysis of function body while
   FLAGS are flags known otherwise (i.e. by fnspec, pure/const attributes
   etc.)  */
static inline int
interposable_eaf_flags (int modref_flags, int flags)
{
  /* If parameter was previously unused, we know it is only read
     and its value is not used.  */
  if ((modref_flags & EAF_UNUSED) && !(flags & EAF_UNUSED))
    {
      modref_flags &= ~EAF_UNUSED;
      modref_flags |= EAF_NO_DIRECT_ESCAPE | EAF_NO_INDIRECT_ESCAPE
		      | EAF_NOT_RETURNED_DIRECTLY | EAF_NOT_RETURNED_INDIRECTLY
		      | EAF_NO_DIRECT_CLOBBER | EAF_NO_INDIRECT_CLOBBER;
    }
  /* We can not deterine that value is not read at all.  */
  if ((modref_flags & EAF_NO_DIRECT_READ) && !(flags & EAF_NO_DIRECT_READ))
    modref_flags &= ~EAF_NO_DIRECT_READ;
  if ((modref_flags & EAF_NO_INDIRECT_READ) && !(flags & EAF_NO_INDIRECT_READ))
    modref_flags &= ~EAF_NO_INDIRECT_READ;
  return modref_flags;
}

#endif

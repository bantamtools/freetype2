/***************************************************************************/
/*                                                                         */
/*  gxvmort1.c                                                             */
/*                                                                         */
/*    TrueTypeGX/AAT mort table validation                                 */
/*    body for type1 (Contextual Substitution) subtable.                   */
/*                                                                         */
/*  Copyright 2005 by suzuki toshiya, Masatake YAMATO, Red Hat K.K.,       */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* gxvalid is derived from both gxlayout module and otvalid module.        */
/* Development of gxlayout was support of Information-technology Promotion */
/* Agency(IPA), Japan.                                                     */
/***************************************************************************/

#include "gxvmort.h"

  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TRACE() and FT_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_COMPONENT
#define FT_COMPONENT  trace_gxvmort


  typedef struct  GXV_mort_subtable_type1_StateOptRec_
  {
    FT_UShort  substitutionTable;
    FT_UShort  substitutionTable_length;

  }  GXV_mort_subtable_type1_StateOptRec,
    *GXV_mort_subtable_type1_StateOptRecData;

#define  GXV_MORT_SUBTABLE_TYPE1_HEADER_SIZE ( GXV_STATETABLE_HEADER_SIZE + 2 )

  static void
  gxv_mort_subtable_type1_substitutionTable_load( FT_Bytes       table,
                                                  FT_Bytes       limit,
                                                  GXV_Validator  valid )
  {
    FT_Bytes  p = table;
    GXV_mort_subtable_type1_StateOptRecData  optdata = valid->statetable.optdata;


    GXV_LIMIT_CHECK( 2 );
    optdata->substitutionTable = FT_NEXT_USHORT( p );
  }


  static void
  gxv_mort_subtable_type1_subtable_setup( FT_UShort      table_size,
                                          FT_UShort      classTable,
                                          FT_UShort      stateArray,
                                          FT_UShort      entryTable,
                                          FT_UShort*     classTable_length_p,
                                          FT_UShort*     stateArray_length_p,
                                          FT_UShort*     entryTable_length_p,
                                          GXV_Validator  valid )
  {
    FT_UShort  o[4];
    FT_UShort  *l[4];
    FT_UShort  buff[5];
    GXV_mort_subtable_type1_StateOptRecData  optdata = valid->statetable.optdata;


    o[0] = classTable;
    o[1] = stateArray;
    o[2] = entryTable;
    o[3] = optdata->substitutionTable;
    l[0] = classTable_length_p;
    l[1] = stateArray_length_p;
    l[2] = entryTable_length_p;
    l[3] = &(optdata->substitutionTable_length);

    gxv_set_length_by_ushort_offset( o, l, buff, 4, table_size, valid );
  }


  static void
  gxv_mort_subtable_type1_offset_to_subst_validate( FT_Short       wordOffset,
                                                    FT_String*     tag,
                                                    FT_Byte        state,
                                                    GXV_Validator  valid )
  {
    FT_UShort  substTable;
    FT_UShort  substTable_limit;
    FT_UShort  min_gid;
    FT_UShort  max_gid;

    substTable       = ((GXV_mort_subtable_type1_StateOptRec *)
                        (valid->statetable.optdata))->substitutionTable;
    substTable_limit = substTable +
                       ((GXV_mort_subtable_type1_StateOptRec *)
                        (valid->statetable.optdata))->substitutionTable_length;

    min_gid = ( substTable       - ( wordOffset * 2 ) ) / 2;
    max_gid = ( substTable_limit - ( wordOffset * 2 ) ) / 2;
    max_gid = FT_MAX( max_gid, valid->face->num_glyphs );

    /* TODO: min_gid & max_gid comparison with ClassTable contents */
  }


  static void
  gxv_mort_subtable_type1_entry_validate( FT_Byte                         state,
                                          FT_UShort                       flags,
                                          GXV_StateTable_GlyphOffsetDesc  glyphOffset,
                                          FT_Bytes                        table,
                                          FT_Bytes                        limit,
                                          GXV_Validator                   valid )
  {
    FT_UShort setMark;
    FT_UShort dontAdvance;
    FT_UShort reserved;
    FT_Short  markOffset;
    FT_Short  currentOffset;


    setMark     =   flags / 0x8000;
    dontAdvance = ( flags & 0x4000 ) / 0x4000;
    reserved    =   flags & 0x3FFF;
    markOffset    = GXV_USHORT_TO_SHORT( glyphOffset.ul / 0x00010000 );
    currentOffset = GXV_USHORT_TO_SHORT( glyphOffset.ul & 0x0000FFFF );

    if ( 0 < reserved )
    {
      GXV_TRACE(( " non-zero bits found in reserved range\n" ));
      if ( valid->root->level >= FT_VALIDATE_PARANOID )
        FT_INVALID_DATA;
    }

    gxv_mort_subtable_type1_offset_to_subst_validate( markOffset,
                                                      "markOffset",
                                                      state,
                                                      valid );

    gxv_mort_subtable_type1_offset_to_subst_validate( currentOffset,
                                                      "currentOffset",
                                                      state,
                                                      valid );
  }

  static void
  gxv_mort_subtable_type1_substTable_validate( FT_Bytes       table,
                                               FT_Bytes       limit,
                                               GXV_Validator  valid )
  {
    FT_Bytes   p = table;
    FT_UShort  num_gids = ((GXV_mort_subtable_type1_StateOptRec *)
                           (valid->statetable.optdata))->substitutionTable_length
                          / 2;
    FT_UShort  i;


    GXV_NAME_ENTER(( "validate contents in substitionTable" ));
    for ( i = 0; i < num_gids ; i ++ )
    {
      FT_UShort  dst_gid;


      GXV_LIMIT_CHECK( 2 );
      dst_gid = FT_NEXT_USHORT( p );

      if ( dst_gid >= 0xFFFF )
        continue;

      if ( dst_gid > valid->face->num_glyphs )
      {
        GXV_TRACE(( "substTable include too-large gid[%d]=%d > max defined gid #%d\n",
                     i, dst_gid, valid->face->num_glyphs ));
        if ( valid->root->level >= FT_VALIDATE_PARANOID )
          FT_INVALID_GLYPH_ID;
      }
    }

    GXV_EXIT;
  }

  /*
   * subtable for Contextual glyph substition is modified StateTable.
   * In addition classTable, stateArray, entryTable, "substitutionTable"
   * is added.
   */
  static void
  gxv_mort_subtable_type1_validate( FT_Bytes       table,
                                    FT_Bytes       limit,
                                    GXV_Validator  valid )
  {
    FT_Bytes  p = table;
    GXV_mort_subtable_type1_StateOptRec  st_rec;


    GXV_NAME_ENTER( "mort chain subtable type1 (Contextual Glyph Subst)" );

    GXV_LIMIT_CHECK( GXV_MORT_SUBTABLE_TYPE1_HEADER_SIZE );

    valid->statetable.optdata               = &st_rec;
    valid->statetable.optdata_load_func     = gxv_mort_subtable_type1_substitutionTable_load;
    valid->statetable.subtable_setup_func   = gxv_mort_subtable_type1_subtable_setup;
    valid->statetable.entry_glyphoffset_fmt = GXV_GLYPHOFFSET_ULONG;
    valid->statetable.entry_validate_func   = gxv_mort_subtable_type1_entry_validate;
    gxv_StateTable_validate( p, limit, valid );

    gxv_mort_subtable_type1_substTable_validate( table
                                                   + st_rec.substitutionTable,
                                                 table
                                                   + st_rec.substitutionTable
                                                   + st_rec.substitutionTable_length,
                                                 valid );

    GXV_EXIT;
  }


/* END */
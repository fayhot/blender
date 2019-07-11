/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/** \file
 * \ingroup RNA
 */

#include <stdlib.h>
#include <stdio.h>

#include "DNA_profilewidget_types.h"
#include "DNA_texture_types.h"

#include "BLI_utildefines.h"

#include "RNA_define.h"
#include "rna_internal.h"

#include "WM_api.h"
#include "WM_types.h"

/* HANS-TODO: Update runtime */
#ifdef RNA_RUNTIME

#  include "RNA_access.h"

#  include "DNA_image_types.h"
#  include "DNA_material_types.h"
#  include "DNA_movieclip_types.h"
#  include "DNA_node_types.h"
#  include "DNA_object_types.h"
#  include "DNA_particle_types.h"
#  include "DNA_sequence_types.h"

#  include "MEM_guardedalloc.h"

#  include "BKE_colorband.h"
#  include "BKE_profile_widget.h"
#  include "BKE_image.h"
#  include "BKE_movieclip.h"
#  include "BKE_node.h"
#  include "BKE_sequencer.h"
#  include "BKE_linestyle.h"

#  include "DEG_depsgraph.h"

#  include "ED_node.h"

#  include "IMB_colormanagement.h"
#  include "IMB_imbuf.h"

static void rna_ProfileWidget_clip_set(PointerRNA *ptr, bool value)
{
  ProfileWidget *prwdgt = (ProfileWidget *)ptr->data;

  if (value) {
    prwdgt->flag |= PROF_DO_CLIP;
  }
  else {
    prwdgt->flag &= ~PROF_DO_CLIP;
  }

  profilewidget_changed(prwdgt, false);
}

static void rna_ProfileWidget_sample_straight_set(PointerRNA *ptr, bool value) {
  ProfileWidget *prwdgt = (ProfileWidget *)ptr->data;

  if (value) {
    prwdgt->flag |= PROF_SAMPLE_STRAIGHT_EDGES;
  }
  else {
    prwdgt->flag &= ~PROF_SAMPLE_STRAIGHT_EDGES;
  }

  profilewidget_changed(prwdgt, false);
}

static void rna_ProfileWidget_remove_point(ProfileWidget *prwdgt, ReportList *reports, PointerRNA *point_ptr)
{
  ProfilePoint *point = point_ptr->data;
  if (profilewidget_remove_point(prwdgt, point) == false) {
    BKE_report(reports, RPT_ERROR, "Unable to remove path point");
    return;
  }

  RNA_POINTER_INVALIDATE(point_ptr);
}

/* HANS-TODO: Needs to change to a 2D output */
//static float rna_ProfileWidget_evaluate(struct ProfileWidget *cuma, ReportList *reports, float value)
//{
//  if (!cuma->table) {
//    BKE_report(reports, RPT_ERROR,
//        "CurveMap table not initialized, call initialize() on CurveMapping owner of the CurveMap");
//    return 0.0f;
//  }
//  return curvemap_evaluateF(cuma, value);
//}

static void rna_ProfileWidget_initialize(struct ProfileWidget *prwdgt, int nsegments)
{
  profilewidget_initialize(prwdgt, nsegments);
}

static void rna_ProfileWidget_changed(struct ProfileWidget *prwdgt)
{
  profilewidget_changed(prwdgt, false);
}

#else

static void rna_def_profilepoint(BlenderRNA *brna)
{
  StructRNA *srna;
  PropertyRNA *prop;

  static const EnumPropertyItem prop_handle_type_items[] = {
      {0, "AUTO", 0, "Auto Handle", ""}, /* HANS-TODO: Remove */
      {PROF_HANDLE_AUTO_ANIM, "AUTO_CLAMPED", 0, "Auto Clamped Handle", ""},
      {PROF_HANDLE_VECTOR, "VECTOR", 0, "Vector Handle", ""},
      {0, NULL, 0, NULL, NULL},
  };

  srna = RNA_def_struct(brna, "ProfilePoint", NULL);
  RNA_def_struct_ui_text(srna, "ProfilePoint", "Point of a path used to define a profile");

  prop = RNA_def_property(srna, "location", PROP_FLOAT, PROP_XYZ);
  RNA_def_property_float_sdna(prop, NULL, "x");
  RNA_def_property_array(prop, 2);
  RNA_def_property_ui_text(prop, "Location", "X/Y coordinates of the path point");

  prop = RNA_def_property(srna, "handle_type", PROP_ENUM, PROP_NONE);
  RNA_def_property_enum_bitflag_sdna(prop, NULL, "flag");
  RNA_def_property_enum_items(prop, prop_handle_type_items);
  RNA_def_property_ui_text(prop, "Handle Type",
                           "Path interpolation at this point: Bezier or vector");

  prop = RNA_def_property(srna, "select", PROP_BOOLEAN, PROP_NONE);
  RNA_def_property_boolean_sdna(prop, NULL, "flag", PROF_SELECT);
  RNA_def_property_ui_text(prop, "Select", "Selection state of the path point");
}

/* HANS-TODO: Add more API callbacks */
static void rna_def_profilewidget_points_api(BlenderRNA *brna, PropertyRNA *cprop)
{
  StructRNA *srna;
  PropertyRNA *parm;
  FunctionRNA *func;

  RNA_def_property_srna(cprop, "ProfilePoints");
  srna = RNA_def_struct(brna, "ProfilePoints", NULL);
  RNA_def_struct_sdna(srna, "ProfileWidget");
  RNA_def_struct_ui_text(srna, "Profile Point", "Collection of Profile Points");

  func = RNA_def_function(srna, "new", "profilewidget_insert");
  RNA_def_function_ui_description(func, "Add point to the profile widget");
  parm = RNA_def_float(func, "x", 0.0f, -FLT_MAX, FLT_MAX, "X Position",
                       "X Position for new point", -FLT_MAX, FLT_MAX);
  RNA_def_parameter_flags(parm, 0, PARM_REQUIRED);
  parm = RNA_def_float(func, "y", 0.0f, -FLT_MAX, FLT_MAX, "Y Position",
                       "Y Position for new point", -FLT_MAX, FLT_MAX);
  RNA_def_parameter_flags(parm, 0, PARM_REQUIRED);
  parm = RNA_def_pointer(func, "point", "ProfilePoint", "", "New point");
  RNA_def_function_return(func, parm);

  func = RNA_def_function(srna, "remove", "rna_ProfileWidget_remove_point");
  RNA_def_function_flag(func, FUNC_USE_REPORTS);
  RNA_def_function_ui_description(func, "Delete point from profile widget");
  parm = RNA_def_pointer(func, "point", "ProfilePoint", "", "PointElement to remove");
  RNA_def_parameter_flags(parm, PROP_NEVER_NULL, PARM_REQUIRED | PARM_RNAPTR);
  RNA_def_parameter_clear_flags(parm, PROP_THICK_WRAP, 0);
}

/* HANS-STRETCH-GOAL: Give the presets icons! */
/* HANS-TODO: Add more props? */
const EnumPropertyItem rna_enum_profilewidget_preset_items[] = {
    {PROF_PRESET_LINE, "LINE", 0, "Line", "Default"},
    {PROF_PRESET_SUPPORTS, "SUPPORTS", 0, "Support Loops", "Loops on either side of the profile"},
    {PROF_PRESET_EXAMPLE1, "EXAMPLE1", 0, "Molding Example", "An example use case"},
    {0, NULL, 0, NULL, NULL},
};

static void rna_def_profilewidget(BlenderRNA *brna)
{
  StructRNA *srna;
  PropertyRNA *prop;
  PropertyRNA *parm;
  FunctionRNA *func;

  srna = RNA_def_struct(brna, "ProfileWidget", NULL);
  RNA_def_struct_ui_text(srna,"ProfileWidget", "Profile Path editor used to build a profile path");

  prop = RNA_def_property(srna, "preset", PROP_ENUM, PROP_NONE);
  RNA_def_property_enum_sdna(prop, NULL, "preset");
  RNA_def_property_enum_items(prop, rna_enum_profilewidget_preset_items);
//  RNA_def_property_enum_funcs(prop, NULL, "rna_Object_rotation_mode_set", NULL);
  RNA_def_property_ui_text(prop, "Preset", "");

  prop = RNA_def_property(srna, "use_clip", PROP_BOOLEAN, PROP_NONE);
  RNA_def_property_boolean_sdna(prop, NULL, "flag", PROF_DO_CLIP);
  RNA_def_property_ui_text(prop, "Clip", "Force the path view to fit a defined boundary");
  RNA_def_property_boolean_funcs(prop, NULL, "rna_ProfileWidget_clip_set");

  prop = RNA_def_property(srna, "sample_straight_edges", PROP_BOOLEAN, PROP_NONE);
  RNA_def_property_boolean_sdna(prop, NULL, "flag", PROF_SAMPLE_STRAIGHT_EDGES);
  RNA_def_property_ui_text(prop, "Sample Straight Edges", "Sample edges with vector handles");
  RNA_def_property_boolean_funcs(prop, NULL, "rna_ProfileWidget_sample_straight_set");

  func = RNA_def_function(srna, "update", "rna_ProfileWidget_changed");
  RNA_def_function_ui_description(func, "Update profile widget after making changes");

  func = RNA_def_function(srna, "initialize", "rna_ProfileWidget_initialize");
  parm = RNA_def_int(func, "nsegments", 1, 1, 1000, "", "The number of segment values to"
                     " initialize the arrays with", 1, 100);
  RNA_def_parameter_flags(parm, PROP_NEVER_NULL, PARM_REQUIRED);
  RNA_def_function_ui_description(func, "Initialize profile");

  prop = RNA_def_property(srna, "points", PROP_COLLECTION, PROP_NONE);
  RNA_def_property_collection_sdna(prop, NULL, "path", "totpoint");
  RNA_def_property_struct_type(prop, "ProfilePoint");
  RNA_def_property_ui_text(prop, "Points", "");
  rna_def_profilewidget_points_api(brna, prop);

  /* HANS-TODO: This is disabled until I figure out how to return the X/Y pair */
//  func = RNA_def_function(srna, "evaluate", "rna_ProfileWidget_Evaluate");
//  RNA_def_function_flag(func, FUNC_USE_REPORTS);
//  RNA_def_function_ui_description(func, "Evaluate the position of the indexed ProfilePoint");
//  parm = RNA_def_float(func, "position", 0.0f, -FLT_MAX, FLT_MAX, "Position",
//                       "Position to evaluate curve at", -FLT_MAX, FLT_MAX);
//  RNA_def_parameter_flags(parm, 0, PARM_REQUIRED);
//  parm = RNA_def_float(func, "value", 0.0f, -FLT_MAX, FLT_MAX, "Value",
//                       "Value of curve at given location", -FLT_MAX, FLT_MAX);
//  RNA_def_function_return(func, parm);
}

void RNA_def_profile(BlenderRNA *brna)
{
  rna_def_profilepoint(brna);
  rna_def_profilewidget(brna);
}

#endif

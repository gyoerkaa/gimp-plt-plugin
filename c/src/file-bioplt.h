// ##### BEGIN GPL LICENSE BLOCK #####
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
// ##### END GPL LICENSE BLOCK #####

#ifndef FILE_BIOPLT_H
#define FILE_BIOPLT_H

#include <libgimp/gimp.h>

#define LOAD_PROCEDURE "file-bioplt-load"
#define SAVE_PROCEDURE "file-bioplt-save"
#define ADDL_PROCEDURE "file-bioplt-addl"

#define PLT_HEADER_VERSION "PLT V1  "
#define PLT_NUM_LAYERS 10

// New layers can easily be added by extending this list, they
// will automatically be included
// Add them to the end, list-position = plt-layer-idx (skin = 0)
const gchar *PLT_LAYERS[PLT_NUM_LAYERS]  = {
    "skin",
    "hair",
    "metal1",
    "metal2",
    "cloth1",
    "cloth2",
    "leather1",
    "leather2",
    "tattoo1",
    "tattoo2"};

static void query(void);

static void run(const gchar      *name,
                gint              nparams,
                const GimpParam  *param,
                gint             *nreturn_vals,
                GimpParam       **return_vals);

static GimpPDBStatusType plt_load(gchar *filename, gint32 *image_id);

static GimpPDBStatusType plt_save(gchar *filename, gint32 image_id);

static GimpPDBStatusType plt_add_layers(gint32 image_id);

#endif

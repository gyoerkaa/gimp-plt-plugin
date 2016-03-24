#ifndef FILE_BIOPLT_H
#define FILE_BIOPLT_H

#include <libgimp/gimp.h>

#define LOAD_PROCEDURE "file-bioplt-load"
#define SAVE_PROCEDURE "file-bioplt-save"

#define PLT_NUM_LAYERS 10
#define PLT_HEADER_VERSION "PLT V1  "

// Don't change order, keep everything lowercase
const gchar *plt_layernames[PLT_NUM_LAYERS]  = {
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

#ifndef FILE_BIOPLT_H
#define FILE_BIOPLT_H

#define LOAD_PROCEDURE "file-bioplt-load"
#define SAVE_PROCEDURE "file-bioplt-save"


// Don't change order, keep everything lowercase
#define NUM_PLT_LAYERS 10
const gchar *plt_layernames[NUM_PLT_LAYERS]  = {
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


static GimpPDBStatusType plt_load(gchar *filename, gint32 *imageID);


static GimpPDBStatusType plt_save(gint32 imageID, gint32 drawableID);


#endif

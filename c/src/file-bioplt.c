#include "file-bioplt.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

static void query(void)
{
    // Load arguments
    static const GimpParamDef load_args[] =
    {
        {GIMP_PDB_INT32,  (gchar*)"run-mode",     (gchar*)"Interactive, non-interactive"},
        {GIMP_PDB_STRING, (gchar*)"filename",     (gchar*)"The name of the file to load"},
        {GIMP_PDB_STRING, (gchar*)"raw-filename", (gchar*)"The name entered"}
    };

    // Load return values
    static const GimpParamDef load_return_values[] =
    {
        {GIMP_PDB_IMAGE, (gchar*)"image", (gchar*)"Output image"}
    };

    // Save arguments
    static const GimpParamDef save_args[] =
    {
        {GIMP_PDB_INT32,    (gchar*)"run-mode",     (gchar*)"Interactive, non-interactive" },
        {GIMP_PDB_IMAGE,    (gchar*)"image",        (gchar*)"Input image" },
        {GIMP_PDB_DRAWABLE, (gchar*)"drawable",     (gchar*)"Drawable to save" },
        {GIMP_PDB_STRING,   (gchar*)"filename",     (gchar*)"The name of the file to save the image in" },
        {GIMP_PDB_STRING,   (gchar*)"raw-filename", (gchar*)"The name entered" }
    };

    // Install load procedure
    gimp_install_procedure(LOAD_PROCEDURE,
                           "Load a Packed Layer Texture (.plt)",
                           "Load a Packed Layer Texture (.plt)",
                           "Attila Györkös",
                           "GPL v3",
                           "2016",
                           "Packed Layer Texture",
                           NULL,
                           GIMP_PLUGIN,
                           G_N_ELEMENTS(load_args),
                           G_N_ELEMENTS(load_return_values),
                           load_args,
                           load_return_values);
    // Register load handlers
    gimp_register_file_handler_mime(LOAD_PROCEDURE, "image/plt");
    gimp_register_load_handler(LOAD_PROCEDURE, "plt", "");

    // Install save procedure
    gimp_install_procedure(SAVE_PROCEDURE,
                           "Save a Packed Layer Texture (.plt)",
                           "Save a Packed Layer Texture (.plt)",
                           "Attila Györkös",
                           "GPL v3",
                           "2016",
                           "Packed Layer Texture",
                           "RGB*",
                           GIMP_PLUGIN,
                           G_N_ELEMENTS(save_args),
                           0,
                           save_args,
                           NULL);
    // Register save handlers
    gimp_register_file_handler_mime(SAVE_PROCEDURE, "image/plt");
    gimp_register_save_handler(SAVE_PROCEDURE, "plt", "");
}


static void run(const gchar      *name,
                gint              nparams,
                const GimpParam  *param,
                gint             *nreturn_vals,
                GimpParam       **return_vals)
{
    static GimpParam return_values[2];
    GimpPDBStatusType status = GIMP_PDB_SUCCESS;
    GimpRunMode run_mode;

    /* Mandatory output values */
    *nreturn_vals = 1;
    *return_vals  = return_values;

    /* Default return value = success*/
    return_values[0].type = GIMP_PDB_STATUS;
    return_values[0].data.d_status = status;

    gint32 imageID;
    gint32 drawableID;

    /* Get run_mode - don't display a dialog if in NONINTERACTIVE mode */
    run_mode = (GimpRunMode)param[0].data.d_int32;
    if (!strcmp(name, LOAD_PROCEDURE))
    {
        // No import options/interactivity right now, so every run mode is
        // handled the same way
        switch (run_mode)
        {
            case GIMP_RUN_INTERACTIVE:
            case GIMP_RUN_WITH_LAST_VALS:
            case GIMP_RUN_NONINTERACTIVE:
            default:
                status = plt_load(param[1].data.d_string, &imageID);
                break;
        }

        if(status == GIMP_PDB_SUCCESS)
        {
        }
    }
    else if (!strcmp(name, SAVE_PROCEDURE))
    {
        imageID    = param[1].data.d_int32;
        drawableID = param[2].data.d_int32;

        // No import options/interactivity right now, so every run mode is
        // handled the same way
        switch (run_mode)
        {
            case GIMP_RUN_INTERACTIVE:
            case GIMP_RUN_WITH_LAST_VALS:
            case GIMP_RUN_NONINTERACTIVE:
            default:
                status = plt_save(imageID, drawableID);
                break;
        }

    }
    else
    {
        return_values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
    }
}


static GimpPDBStatusType plt_load(gchar *filename, gint32 *imageID)
{
    FILE *plt_file = 0;
    int i;

    uint8_t  plt_version[8];
    uint32_t plt_width  = 0;
    uint32_t plt_height = 0;

    uint32_t numPx = 0;
    uint8_t px_value = 0;
    uint8_t px_layer = 0;

    gint32 newImgID   = -1;
    gint32 newLayerID = -1;
    gint32 layerIDs[NUM_PLT_LAYERS];
    guint8 pixel[2] = {0, 255}; // GRAYA image = 2 Channels: Value + Alpha

    plt_file = fopen(filename, "rb");
    if(plt_file == 0)
    {
        g_message("Error opening file.\n");
        return(GIMP_PDB_EXECUTION_ERROR);
    }

    // Header
    fread(&plt_version, 1, 8, plt_file);
    fseek(plt_file, 8, SEEK_CUR); // These 8 bytes don't matter
    fread(&plt_width, 4, 1, plt_file);
    fread(&plt_height, 4, 1, plt_file);

    // Create a new image
    newImgID = gimp_image_new(plt_width, plt_height, GIMP_GRAY);
    if(newImgID == -1)
    {
        g_message("Can't allocate new image.\n");
        fclose(plt_file);
        return(GIMP_PDB_EXECUTION_ERROR);
    }

    // Create 10 plt layers

    for (i = 0; i < NUM_PLT_LAYERS; i++)
    {
        newLayerID = gimp_layer_new(newImgID,
                                    plt_layernames[i],
                                    plt_width, plt_height,
                                    GIMP_GRAYA_IMAGE,
                                    100.0,
                                    GIMP_NORMAL_MODE);
        gimp_image_insert_layer(newImgID, newLayerID, 0, 0);
        layerIDs[i] = newLayerID;
    }

    // Image data
    // Expecting width*height (value, layer) tuples
    numPx = plt_width * plt_height;
    i = 0;
    while (i < numPx)
    {
        fread(&px_value, 1, 1, plt_file);
        fread(&px_layer, 1, 1, plt_file);

        pixel[0] = px_value;
        gimp_drawable_set_pixel(layerIDs[px_layer],
                                i % plt_width,
                                plt_height - (int)(floor(i / plt_width)) - 1,
                                2,
                                pixel);

        i++;
    }
    //gimp_image_delete(newImgID);

    fclose(plt_file);
    *imageID = newImgID;
    return(GIMP_PDB_SUCCESS);
}


static GimpPDBStatusType plt_save(gint32 imageID, gint32 drawableID)
{
    return(GIMP_PDB_SUCCESS);
}


GimpPlugInInfo PLUG_IN_INFO = {NULL, NULL, query, run};


MAIN()

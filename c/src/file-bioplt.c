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

#include "file-bioplt.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

static void query(void)
{
    // Load procedure arguments
    static const GimpParamDef load_args[] =
    {
        {GIMP_PDB_INT32,  (gchar*)"run-mode",     (gchar*)"Interactive, non-interactive"},
        {GIMP_PDB_STRING, (gchar*)"filename",     (gchar*)"The name of the file to load"},
        {GIMP_PDB_STRING, (gchar*)"raw-filename", (gchar*)"The name entered"}
    };

    // Load procedure return values
    static const GimpParamDef load_return_values[] =
    {
        {GIMP_PDB_IMAGE, (gchar*)"image", (gchar*)"Output image"}
    };

    // Install load procedure
    gimp_install_procedure(LOAD_PROCEDURE,
                           "Load a Packed Layer Texture (.plt)",
                           "Load a Packed Layer Texture (.plt)",
                           "Attila Gyoerkoes",
                           "GPL v3",
                           "2016",
                           "<Load>/Packed Layer Texture",
                           NULL,
                           GIMP_PLUGIN,
                           G_N_ELEMENTS(load_args),
                           G_N_ELEMENTS(load_return_values),
                           load_args,
                           load_return_values);
    // Register Load handlers
    gimp_register_file_handler_mime(LOAD_PROCEDURE, "image/plt");
    gimp_register_load_handler(LOAD_PROCEDURE, "plt", "");

    // Save procedure arguments
    static const GimpParamDef save_args[] =
    {
        {GIMP_PDB_INT32,    (gchar*)"run-mode",     (gchar*)"Interactive, non-interactive" },
        {GIMP_PDB_IMAGE,    (gchar*)"image",        (gchar*)"Input image" },
        {GIMP_PDB_DRAWABLE, (gchar*)"drawable",     (gchar*)"Drawable to save" },
        {GIMP_PDB_STRING,   (gchar*)"filename",     (gchar*)"The name of the file to save the image in" },
        {GIMP_PDB_STRING,   (gchar*)"raw-filename", (gchar*)"The name entered" }
    };

    // Install save procedure
    gimp_install_procedure(SAVE_PROCEDURE,
                           "Save a Packed Layer Texture (.plt)",
                           "Save a Packed Layer Texture (.plt)",
                           "Attila Gyoerkoes",
                           "GPL v3",
                           "2016",
                           "<Save>/Packed Layer Texture",
                           "RGB*",
                           GIMP_PLUGIN,
                           G_N_ELEMENTS(save_args),
                           0,
                           save_args,
                           NULL);
    // Register Save handlers
    gimp_register_file_handler_mime(SAVE_PROCEDURE, "image/plt");
    gimp_register_save_handler(SAVE_PROCEDURE, "plt", "");

     // Add Layers procedure arguments
    static const GimpParamDef addl_args[] =
    {
        {GIMP_PDB_INT32, (gchar*)"run-mode", (gchar*)"Interactive, non-interactive" },
        {GIMP_PDB_IMAGE, (gchar*)"image",    (gchar*)"Input image" }
    };

    // Install Add Layers procedure
    gimp_install_procedure(ADDL_PROCEDURE,
                           "Add plt layers",
                           "Add plt layers",
                           "Attila Gyoerkoes",
                           "GPL v3",
                           "2016",
                           "Add missing plt layers",
                           "*",
                           GIMP_PLUGIN,
                           G_N_ELEMENTS(addl_args),
                           0,
                           addl_args,
                           NULL);
    // Register Add Layers handlers
    gimp_plugin_menu_register(ADDL_PROCEDURE, "<Image>/Tools");
}


static void run(const gchar      *name,
                gint              nparams,
                const GimpParam  *param,
                gint             *nreturn_vals,
                GimpParam       **return_vals)
{
    static GimpParam return_values[2];
    GimpPDBStatusType status = GIMP_PDB_EXECUTION_ERROR;
    GimpRunMode run_mode;

    /* Mandatory output values */
    *nreturn_vals = 2;
    *return_vals  = return_values;

    /* Default return value */
    return_values[0].type = GIMP_PDB_STATUS;
    return_values[0].data.d_status = status;

    gint32 image_id;
    gint32 drawable_id;

    /* Get run_mode - don't display a dialog if in NONINTERACTIVE mode */
    run_mode = (GimpRunMode) param[0].data.d_int32;
    if (!g_strcmp0(name, LOAD_PROCEDURE))
    {
        image_id    = -1;
        drawable_id = -1;

        switch (run_mode)
        {
            case GIMP_RUN_INTERACTIVE:
            case GIMP_RUN_WITH_LAST_VALS:
                status = plt_load(param[1].data.d_string, &image_id);
                gimp_displays_flush();
                break;
            case GIMP_RUN_NONINTERACTIVE:
            default:
                status = plt_load(param[1].data.d_string, &image_id);
                break;
        }

        return_values[0].data.d_status = status;
        if (status == GIMP_PDB_SUCCESS)
        {
            return_values[1].type = GIMP_PDB_IMAGE;
            return_values[1].data.d_image = image_id;
        }
    }
    else if (!g_strcmp0(name, SAVE_PROCEDURE))
    {
        image_id    = param[1].data.d_int32;
        drawable_id = param[2].data.d_int32;

        // No import options/interactivity right now, so every run mode is
        // handled the same way
        switch (run_mode)
        {
            case GIMP_RUN_INTERACTIVE:
            case GIMP_RUN_WITH_LAST_VALS:
            case GIMP_RUN_NONINTERACTIVE:
            default:
                status = plt_save(param[3].data.d_string, image_id);
                break;
        }

        return_values[0].data.d_status = status;
    }
    else if (!g_strcmp0(name, ADDL_PROCEDURE))
    {
        image_id = param[1].data.d_int32;

        switch (run_mode)
        {
            case GIMP_RUN_INTERACTIVE:
            case GIMP_RUN_WITH_LAST_VALS:
                status = plt_add_layers(image_id);
                gimp_displays_flush();
                break;
            case GIMP_RUN_NONINTERACTIVE:
            default:
                status = plt_add_layers(image_id);
                break;
        }

        return_values[0].data.d_status = status;
    }
    else
    {
        return_values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
    }
}


static GimpPDBStatusType plt_load(gchar *filename, gint32 *image_id)
{
    FILE *stream = 0;
    unsigned int i;

    // Using uint for guaranteed sizes across all systems
    // guint may or may not work (?)
    uint8_t  plt_version[8];
    uint32_t plt_width  = 0;
    uint32_t plt_height = 0;
    uint8_t *buffer;

    uint8_t px_value = 0;
    uint8_t px_layer = 0;
    uint32_t num_px = 0;

    gint32 newImgID   = -1;
    gint32 newLayerID = -1;
    gint32 plt_layer_ids[PLT_NUM_LAYERS];
    guint8 pixel[2] = {0, 255}; // GRAYA image = 2 Channels: Value + Alpha

    stream = fopen(filename, "rb");
    if(stream == 0)
    {
        g_message("Error opening file.\n");
        return (GIMP_PDB_EXECUTION_ERROR);
    }

    gimp_progress_init_printf("Opening %s", filename);
    gimp_progress_update(0.0);

    // Read header: Version, should be 8x1 bytes = "PLT V1  "
    if (fread(plt_version, 1, 8, stream) < 8)
    {
        g_message("Invalid plt file: Unable to read version information.\n");
        fclose(stream);
        return (GIMP_PDB_EXECUTION_ERROR);
    }
    if (g_ascii_strncasecmp(plt_version, PLT_HEADER_VERSION, 8) != 0)
    {

        g_message("Invalid plt file: Version mismatch.\n");
        fclose(stream);
        return (GIMP_PDB_EXECUTION_ERROR);
    }
    // Read header: Next 8 bytes don't matter
    fseek(stream, 8, SEEK_CUR);
    // Read header: Width
    if (fread(&plt_width, 4, 1, stream) < 1)
    {
        g_message("Invalid plt file: Unable to read width.\n");
        fclose(stream);
        return (GIMP_PDB_EXECUTION_ERROR);
    }
    // Read header: Height
    if (fread(&plt_height, 4, 1, stream) < 1)
    {
        g_message("Invalid plt file: Unable to read height.\n");
        fclose(stream);
        return (GIMP_PDB_EXECUTION_ERROR);
    }

    // Create a new image
    newImgID = gimp_image_new(plt_width, plt_height, GIMP_GRAY);
    if(newImgID == -1)
    {
        g_message("Unable to allocate new image.\n");
        fclose(stream);
        return (GIMP_PDB_EXECUTION_ERROR);
    }
    gimp_image_set_filename(newImgID, filename);

    // Create the 10 plt layers, add them to the new image and save their ID's
    for (i = 0; i < PLT_NUM_LAYERS; i++)
    {
        newLayerID = gimp_layer_new(newImgID,
                                    plt_layernames[i],
                                    plt_width, plt_height,
                                    GIMP_GRAYA_IMAGE,
                                    100.0,
                                    GIMP_NORMAL_MODE);
        gimp_image_insert_layer(newImgID, newLayerID, 0, 0);
        plt_layer_ids[i] = newLayerID;
    }

    // Read image data
    // Expecting width*height (value, layer) tuples = 2*width*height bytes
    num_px = plt_width * plt_height;
    buffer = (uint8_t*) g_malloc(sizeof(uint8_t)*2*num_px);
    if (fread(buffer, 1, 2*num_px, stream) < (2*num_px))
    {
        g_message("Image size mismatch.\n");
        fclose(stream);
        g_free(buffer);
        gimp_image_delete(newImgID);
        return (GIMP_PDB_EXECUTION_ERROR);
    }
    for (i = 0; i < num_px; i++)
    {
        pixel[0] = buffer[2*i];
        px_layer = buffer[2*i+1];
        gimp_drawable_set_pixel(plt_layer_ids[px_layer],
                                i % plt_width,
                                plt_height - (int)(floor(i / plt_width)) - 1,
                                2,
                                pixel);
        gimp_progress_update((float) i/ (float) num_px);
    }
    gimp_progress_update(1.0);
    gimp_image_set_active_layer(newImgID, plt_layer_ids[0]);

    fclose(stream);

    g_free(buffer);

    *image_id = newImgID;
    return (GIMP_PDB_SUCCESS);
}


static GimpPDBStatusType plt_save(gchar *filename, gint32 image_id)
{
    FILE *stream = 0;
    unsigned int i, j;

    uint8_t  plt_version[8] = PLT_HEADER_VERSION;
    uint8_t  plt_info[8]    = {10, 0, 0, 0, 0, 0, 0, 0};
    uint32_t plt_width  = 0;
    uint32_t plt_height = 0;

    uint8_t *buffer;
    uint8_t *pixel;
    uint32_t num_px = 0;

    GimpImageBaseType img_basetype;
    gint img_num_layers; // num layers in image
    gint *img_layer_ids; // all layers in image
    gint32 layer_id;
    gint *plt_layer_ids;
    gint max_layer_id;
    gint32 detected_plt_layers = 0;
    gint x, y;

    gint num_channels; // Channels in image

    // Only get image data if it's valid
    if (!gimp_image_is_valid(image_id))
    {
        g_message("Invalid image.\n");
        return (GIMP_PDB_EXECUTION_ERROR);
    }
    plt_width  = gimp_image_width(image_id);
    plt_height = gimp_image_height(image_id);
    img_basetype = gimp_image_base_type(image_id);

    gimp_progress_init_printf("Exporting %s", filename);
    gimp_progress_update(0.0);


    img_layer_ids = gimp_image_get_layers(image_id, &img_num_layers);
    if (img_num_layers < PLT_NUM_LAYERS)
    {
        g_message("Requires an image with at least 10 layers.\n");
        g_free(img_layer_ids);
        return (GIMP_PDB_EXECUTION_ERROR);
    }

    max_layer_id  = -1;
    for (i = 0; i < img_num_layers; i++)
    {
        if (img_layer_ids[i] > max_layer_id)
        {
            max_layer_id = img_layer_ids[i];
        }
    }

    // Try to find the 10 plt layers by looking for matching layer names
    plt_layer_ids = g_malloc(sizeof(gint)*max_layer_id);
    for (i = 0; i < max_layer_id; i++)
    {
        plt_layer_ids[i] = -1;
    }
    for (i = 0; i < PLT_NUM_LAYERS; i++)
    {
        for (j = 0; j < img_num_layers; j++)
        {
            if (!g_ascii_strcasecmp(plt_layernames[i], gimp_item_get_name(img_layer_ids[j])))
            {
                plt_layer_ids[img_layer_ids[j]] = i;
                detected_plt_layers++;
                break;
            }
        }
    }
    // If we can't find the layers by name, use the 10 topmost layers instead
    // (Interpreted as tattoo2, tattoo1, ... from the top)
    if (detected_plt_layers < PLT_NUM_LAYERS)
    {
        for (i = 0; i < PLT_NUM_LAYERS; i++)
        {
            plt_layer_ids[img_layer_ids[i]] = PLT_NUM_LAYERS-i-1;
        }
    }

    // Write image data to buffer
    num_px = plt_width * plt_height;
    buffer = (uint8_t*) g_malloc(sizeof(uint8_t)*2*num_px);
    switch(img_basetype)
    {
        case GIMP_GRAY:
        {
            // Gray Image
            // >= 1 channels: value (+ alpha). We ignore alpha though, so it
            // doesn't matter wether its present or not.
            for (i = 0; i < num_px; i++)
            {
                x = (gint)(i % plt_width);
                y = plt_height - (gint)(floor(i / plt_width)) - 1;
                layer_id = gimp_image_pick_correlate_layer(image_id, x, y);
                if ((layer_id >= 0) && (plt_layer_ids[layer_id] >= 0))
                {
                    pixel = gimp_drawable_get_pixel(layer_id,
                                                    x,
                                                    y,
                                                    &num_channels);
                    buffer[2*i]   = pixel[0];
                    buffer[2*i+1] = plt_layer_ids[layer_id];
                }
                else
                {
                    buffer[2*i]   = 255;
                    buffer[2*i+1] = 0;
                }
                gimp_progress_update((float) i/ (float) num_px);
            }
            break;
        }
        case GIMP_RGB:
        {
            // RGB Image
            // >= 3 channels: r +g + b (+ alpha). We ignore alpha though, so it
            // doesn't matter wether its present or not.
            // We'll calculate gray value with (r+g+b)/3.
            for (i = 0; i < num_px; i++)
            {
                x = (gint)(i % plt_width);
                y = plt_height - (gint)(floor(i / plt_width)) - 1;
                layer_id = gimp_image_pick_correlate_layer(image_id, x, y);
                if ((layer_id >= 0) && (plt_layer_ids[layer_id] >= 0))
                {
                    pixel = gimp_drawable_get_pixel(layer_id,
                                                    x,
                                                    y,
                                                    &num_channels);
                    buffer[2*i]   = pixel[0];
                    buffer[2*i+1] = plt_layer_ids[layer_id];
                }
                else
                {
                    buffer[2*i]   = 255;
                    buffer[2*i+1] = 0;
                }
                gimp_progress_update((float) i/ (float) num_px);
            }
            break;
        }
        case GIMP_INDEXED: // You're out of luck buddy
        default:
        {
            g_message("Image type has to be Grayscale or RGB.\n");
            g_free(buffer);
            g_free(img_layer_ids);
            g_free(plt_layer_ids);
            return (GIMP_PDB_EXECUTION_ERROR);
        }
    }
    gimp_progress_update(1.0);

    stream = fopen(filename, "wb");
    if (stream == 0)
    {
        g_message("Error opening %s\n", filename);
        g_free(buffer);
        g_free(img_layer_ids);
        g_free(plt_layer_ids);
        return (GIMP_PDB_EXECUTION_ERROR);
    }

    // Write header
    fwrite(plt_version, 1, 8, stream);
    fwrite(plt_info, 1, 8, stream);
    fwrite(&plt_width, 4, 1, stream);
    fwrite(&plt_height, 4, 1, stream);

    // Write image data
    fwrite(buffer, 1, 2*num_px, stream);

    fclose(stream);

    g_free(buffer);
    g_free(img_layer_ids);
    g_free(plt_layer_ids);

    return (GIMP_PDB_SUCCESS);
}


static GimpPDBStatusType plt_add_layers(gint32 image_id)
{
    unsigned int i, j;
    GimpImageBaseType img_basetype;
    gint img_num_layers;
    gint *img_layer_ids;
    gint img_width = 0;
    gint img_height = 0;
    gboolean plt_layer_detected = FALSE;
    GimpImageType layer_type= GIMP_GRAYA_IMAGE;
    gint32 layer_id;

    // Set type of the layer depending on the type of the image
    img_basetype = gimp_image_base_type(image_id);
    switch(img_basetype)
    {
        case GIMP_GRAY:
        {
            layer_type = GIMP_GRAYA_IMAGE;
            break;
        }
        case GIMP_RGB:
        {
            layer_type = GIMP_RGBA_IMAGE;
            break;
        }
        case GIMP_INDEXED: // You're still out of luck
        default:
        {
            g_message("Image type has to be Grayscale or RGB.\n");
            return (GIMP_PDB_EXECUTION_ERROR);
        }
    }

    // We don't want to create already existing plt layers
    // Get all layers from the current image and look for missing layers
    //TODO
    img_width  = gimp_image_width(image_id);
    img_height = gimp_image_height(image_id);
    img_layer_ids = gimp_image_get_layers(image_id, &img_num_layers);
    for (i = 0; i < PLT_NUM_LAYERS; i++)
    {
        plt_layer_detected = FALSE;
        for (j = 0; j < img_num_layers; j++)
        {
            if (!g_ascii_strcasecmp(plt_layernames[i], gimp_item_get_name(img_layer_ids[j])))
            {
                plt_layer_detected = TRUE;
                break;
            }
        }
        if (!plt_layer_detected)
        {
            layer_id = gimp_layer_new(image_id,
                                      plt_layernames[i],
                                      img_width, img_height,
                                      layer_type,
                                      100.0,
                                      GIMP_NORMAL_MODE);
            gimp_image_insert_layer(image_id, layer_id, 0, 0);
        }
    }

    g_free(img_layer_ids);
    return (GIMP_PDB_SUCCESS);
}


GimpPlugInInfo PLUG_IN_INFO = {NULL, NULL, query, run};


MAIN()

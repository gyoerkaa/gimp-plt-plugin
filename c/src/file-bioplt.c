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
#include <stdlib.h>
#include <math.h>


static void flip_plt(uint8_t *pixels,
                     const uint32_t width,
                     const uint32_t height)
{
    const uint32_t num_rows = height / 2;
    const uint32_t row_width = width * 2;
    uint8_t* temp = (uint8_t*) g_malloc(sizeof(uint8_t)*row_width);

    int source_offset, target_offset;
    for (int r = 0; r < num_rows; r++)
    {
        source_offset = r * row_width;
        target_offset = (height - r - 1) * row_width;

        memcpy(temp,                   pixels + source_offset, row_width);
        memcpy(pixels + source_offset, pixels + target_offset, row_width);
        memcpy(pixels + target_offset, temp,                   row_width);
    }
    g_free(temp);
    temp = NULL;
}


static int get_layer_bounds(gint32 image_id, gint32 layer_id, 
                             gint *bx, gint *by, gint *bw, gint *bh)
{
    gint img_width = gimp_image_width(image_id);
    gint img_height = gimp_image_height(image_id);
    gint lay_width  = gimp_drawable_width(layer_id);
    gint lay_height = gimp_drawable_height(layer_id); 
    
    gint lay_x, lay_y;
    gimp_drawable_offsets(layer_id, &lay_x, &lay_y);
    
    if (lay_x < 0)
    {
        *bx = -1 * lay_x;
        if (lay_x + lay_width > img_width)  // layer exceeds image size
            *bw = lay_width;
        else if (lay_x + lay_width > 0)  // layer partially within image
            *bw = lay_x + lay_width;
        else  // layer out of bounds
            return FALSE;
    }
    else
    {
        *bx = 0;
        if (lay_x >= img_width)  // layer out of bounds
            return FALSE;
        else if (lay_x + lay_width > img_width)  // layer partially within image
            *bw = img_width - lay_x;
        else  // layer completely within image
            *bw = lay_width;
    }
    
    if (lay_y < 0)
    {
        *by = -1 * lay_y;
        if (lay_y + lay_height > img_width)  // layer exceeds image size
            *bh = lay_height;
        else if (lay_y + lay_height > 0)  // layer partially within image
            *bh = lay_y + lay_height;
        else  // layer out of bounds
            return FALSE;
    }
    else
    {
        *by = 0;
        if (lay_y >= img_height)  // layer out of bounds
            return FALSE;
        else if (lay_y + lay_height > img_height)  // layer partially within image
            *bh = img_height - lay_y;
        else  // layer completely within image
            *bh = lay_height;
    }    
    
    return TRUE;
}


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
                           "Packed Layer Texture",
                           NULL,
                           GIMP_PLUGIN,
                           G_N_ELEMENTS(load_args),
                           G_N_ELEMENTS(load_return_values),
                           load_args,
                           load_return_values);
    gimp_plugin_menu_register(LOAD_PROCEDURE, "<Load>/Packed Layer Texture");
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
                           "Packed Layer Texture",
                           "RGB*",
                           GIMP_PLUGIN,
                           G_N_ELEMENTS(save_args),
                           0,
                           save_args,
                           NULL);
    gimp_plugin_menu_register(SAVE_PROCEDURE, "<Save>/Packed Layer Texture");
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
                           "Plt: Setup Layers",
                           "",
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
    unsigned int i, j;

    // Using uint instead of guint for guaranteed sizes
    // (not 100% sure if guint works)
    uint8_t  plt_version[8];
    uint32_t plt_width  = 0;
    uint32_t plt_height = 0;
    uint32_t plt_num_px;
    uint8_t *plt_data;
    
    uint8_t *layer_data;
    gint32 layer_id = -1;
    gint32 plt_layer_ids[PLT_NUM_LAYERS];
    
    gint32 img_id = -1;
    GimpPixelRgn region;
    GimpDrawable *drawable;
    guint8 pixel[2] = {0, 255}; // GRAYA image = 2 Channels: Value + Alpha

    stream = fopen(filename, "rb");
    if(stream == 0)
    {
        g_message("Error opening file.\n");
        return (GIMP_PDB_EXECUTION_ERROR);
    }

    gimp_progress_init_printf("Creating layers...");
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
    img_id = gimp_image_new(plt_width, plt_height, GIMP_GRAY);
    if(img_id == -1)
    {
        g_message("Unable to allocate new image.\n");
        fclose(stream);
        return (GIMP_PDB_EXECUTION_ERROR);
    }
    gimp_image_set_filename(img_id, filename);

    // Read image data
    // Expecting width*height (value, layer) tuples = 2*width*height bytes
    plt_num_px = plt_width * plt_height;
    plt_data = (uint8_t*) g_malloc(sizeof(uint8_t)*2*plt_num_px);
    if (fread(plt_data, 1, 2*plt_num_px, stream) < (2*plt_num_px))
    {
        g_message("Image size mismatch.\n");
        fclose(stream);
        g_free(plt_data);
        gimp_image_delete(img_id);
        return (GIMP_PDB_EXECUTION_ERROR);
    }
    fclose(stream);

    // Write data into layers
    layer_data = (uint8_t*) g_malloc(sizeof(uint8_t)*2*plt_num_px);
    gimp_progress_update(0.0);
    for (i = 0; i < PLT_NUM_LAYERS; i++)
    {
        layer_id = gimp_layer_new(img_id,
                                  PLT_LAYERS[i],
                                  plt_width, plt_height,
                                  GIMP_GRAYA_IMAGE,
                                  100.0,
                                  GIMP_NORMAL_MODE);
        drawable = gimp_drawable_get(layer_id);
        gimp_pixel_rgn_init (&region, drawable,
                             0, 0, plt_width, plt_height,
                             TRUE, FALSE);
        // Grab the pixels belonging to this layer
        for (j = 0; j < 2*plt_num_px; j+=2)
        {
            if (plt_data[j+1] == i)
            {
                layer_data[j] = plt_data[j];
                layer_data[j+1] = 255;
            }
            else
            {
                layer_data[j] = 0;
                layer_data[j+1] = 0;
            }
        }
        gimp_pixel_rgn_set_rect(&region,
                                layer_data,
                                0, 0,
                                plt_width, plt_height);
        gimp_drawable_flush(drawable);
        gimp_drawable_detach(drawable);
        gimp_image_insert_layer(img_id, layer_id, 0, 0);
        gimp_progress_update((float) i/ (float) PLT_NUM_LAYERS);
    }
    gimp_progress_update(1.0);
    gimp_image_set_active_layer(img_id, layer_id);
    // Cleanup
    g_free(layer_data);
    g_free(plt_data);
    // Adjust coordinate systems
    gimp_image_flip(img_id, GIMP_ORIENTATION_VERTICAL);
    *image_id = img_id;
    return (GIMP_PDB_SUCCESS);
}


static GimpPDBStatusType plt_save(gchar *filename, gint32 image_id)
{
    FILE *stream = 0;
    unsigned int i, j, k, l;

    uint8_t  plt_version[8] = PLT_HEADER_VERSION;
    uint8_t  plt_info[8] = {10, 0, 0, 0, 0, 0, 0, 0};
    uint32_t plt_width  = 0;
    uint32_t plt_height = 0;
    uint8_t *plt_data;

    gint layer_id;
    gint layer_x, layer_y;
    uint8_t *layer_data;
    gchar *layer_name;
    
    uint32_t plt_num_px;
    gint32 detected_layers;
    gint bpp;

    gint img_num_layers; // num layers in image
    gint *img_layer_ids; // all layers in image
    gint *plt_layer_ids; // valid plt layers
    gint plt_id;
    
    
    GimpImageBaseType img_basetype;
    GimpDrawable *drawable;
    GimpPixelRgn region;
    gint region_x, region_y, region_w, region_h;  // part of the region to get
    
    // Only get image data if it's valid
    if (!gimp_image_is_valid(image_id))
    {
        g_message("Invalid image.\n");
        return (GIMP_PDB_EXECUTION_ERROR);
    }
    plt_width = gimp_image_width(image_id);
    plt_height = gimp_image_height(image_id);
    img_basetype = gimp_image_base_type(image_id);

    // Make sure image is not indexed
    img_basetype = gimp_image_base_type(image_id);
    if (img_basetype == GIMP_INDEXED)
    {
        g_message("Image type has to be Grayscale or RGB.\n");
        return (GIMP_PDB_EXECUTION_ERROR);
    }

    //  Determine which gimp layer to use for which plt layer
    img_layer_ids = gimp_image_get_layers(image_id, &img_num_layers);
    plt_layer_ids = g_malloc(sizeof(gint)*PLT_NUM_LAYERS*2);
    for (i = 0; i < PLT_NUM_LAYERS; i++)
    {
        plt_layer_ids[i] = i;
        plt_layer_ids[i+PLT_NUM_LAYERS] = -1;  // -1 == Invalid layer        
    }
    detected_layers = 0;
    // 1. Look for matching names
    /*
    for (l = PLT_NUM_LAYERS; l-- > 0; )
    {
        plt_layer_ids[l] = l;
        plt_layer_ids[l+PLT_NUM_LAYERS] = -1;  // -1 == Invalid layer
        // find a matching gimp layer
        for (i = 0; i < img_num_layers; i++)
        {
            layer_name = gimp_item_get_name(img_layer_ids[i]); 
            if (!g_ascii_strcasecmp(PLT_LAYERS[l], layer_name))
            {
                plt_layer_ids[l+PLT_NUM_LAYERS] = img_layer_ids[i];  
                detected_layers++;
            }
        }
    }
    */
    // Start with the top layer to reflect what is displayed in gimp
    // (NOTE: layer names are unique, so no problems with duplicates)
    for (l = 0; l < img_num_layers; l++)
    {
        layer_name = gimp_item_get_name(img_layer_ids[l]);
        for (i = 0; i < PLT_NUM_LAYERS; i++)
        {
            if (!g_ascii_strcasecmp(PLT_LAYERS[i], layer_name))
            {
                // a valid plt layer
                plt_layer_ids[detected_layers] = i;
                plt_layer_ids[detected_layers+PLT_NUM_LAYERS] = img_layer_ids[l];  
                detected_layers++;
            }
        }
    }    
    // 2. Fallback, use the n topmost layers, if no layers have been detected
    // before
    if (detected_layers <= 0)
    {
        for (l = 0; ((l < PLT_NUM_LAYERS) && (l < img_num_layers)); l++)
        {
            plt_layer_ids[detected_layers] = l;
            plt_layer_ids[detected_layers+PLT_NUM_LAYERS] = img_layer_ids[l];
            detected_layers++;
        }
    }
    g_free(img_layer_ids);
            
    // Init image data
    plt_num_px = plt_width * plt_height;
    plt_data = (uint8_t*) g_malloc(sizeof(uint8_t)*2*plt_num_px);
    for (i = 0; i < 2*plt_num_px; i+=2)
    {
        plt_data[i] = 255;
        plt_data[i+1] = 0;
    }

    // Generate image data
    gimp_progress_init_printf("Processing layers...");
    gimp_progress_update(0.0);
    switch(img_basetype)
    {
        case GIMP_GRAY:
        {
            // Same buffer for images with or without alpha to avoid having to
            // allocate in loop
            layer_data = (uint8_t*) g_malloc(sizeof(uint8_t)*2*plt_num_px);
            
            for (l = 0; l < PLT_NUM_LAYERS; l++)
            {
                plt_id   = plt_layer_ids[l];  
                layer_id = plt_layer_ids[l+PLT_NUM_LAYERS];                                
                
                if ((layer_id >= 0) && get_layer_bounds(image_id, layer_id, &region_x, &region_y, &region_w, &region_h))
                {
                    drawable = gimp_drawable_get(layer_id);
                    bpp = gimp_drawable_bpp(layer_id);
                
                    gimp_pixel_rgn_init(&region, drawable,
                                        region_x, region_y,
                                        region_w, region_h,
                                        FALSE, FALSE);
                    gimp_pixel_rgn_get_rect(&region,
                                            (uint8_t*) layer_data,
                                            region_x, region_y,
                                            region_w, region_h);
                    
                    k = 0;
                    gimp_drawable_offsets(layer_id, &layer_x, &layer_y);
                    // layer bounds are already adjusted to adress layers being out of bounds
                    // we still need to adjust plt pixel index 
                    layer_x = (layer_x > 0) ? layer_x : 0;
                    layer_y = (layer_y > 0) ? layer_y : 0;
                    if (gimp_drawable_has_alpha(layer_id))
                    {
                        for (i = 0; i < region_h; i++)
                        {
                            for (j = 0; j < region_w; j++)
                            {                                
                                if (layer_data[k+1] > PLT_ALPHA_THRESHOLD)
                                {
                                    plt_data[2*((i+layer_y)*plt_width+j+layer_x)] = layer_data[k];
                                    plt_data[2*((i+layer_y)*plt_width+j+layer_x)+1] = plt_id;
                                }
                                k+=bpp;
                            }                            
                        }
                    }
                    else
                    {
                        // No alpha value means bottom layers are not visible
                        // i.e. always overwrite everything
                        for (i = 0; i < region_h; i++)
                        {
                            for (j = 0; j < region_w; j++)
                            {
                                plt_data[2*((i+layer_y)*plt_width+j+layer_x)] = layer_data[k];
                                plt_data[2*((i+layer_y)*plt_width+j+layer_x)+1] = plt_id;
                                k += bpp;
                            }                            
                        }
                    }
                }
                gimp_progress_update((float) i/(float) detected_layers);
            }
            break;
        }
        case GIMP_RGB:
        {
            // Same buffer for images with or without alpha to avoid having to
            // allocate in loop
            layer_data = (uint8_t*) g_malloc(sizeof(uint8_t)*plt_num_px*4);
                                
            for (l = 0; l < PLT_NUM_LAYERS; l++)
            {
                plt_id   = plt_layer_ids[l];  
                layer_id = plt_layer_ids[l+PLT_NUM_LAYERS];                                
                
                if ((layer_id >= 0) && get_layer_bounds(image_id, layer_id, &region_x, &region_y, &region_w, &region_h))
                {
                    drawable = gimp_drawable_get(layer_id);
                    bpp = gimp_drawable_bpp(layer_id);
                
                    gimp_pixel_rgn_init(&region, drawable,
                                        region_x, region_y,
                                        region_w, region_h,
                                        FALSE, FALSE);
                    gimp_pixel_rgn_get_rect(&region,
                                            (uint8_t*) layer_data,
                                            0, 0,
                                            region_w, region_h);
                    k = 0;
                    gimp_drawable_offsets(layer_id, &layer_x, &layer_y);
                    // layer bounds are already adjusted to adress layers being out of bounds
                    // we still need to adjust plt pixel index 
                    layer_x = (layer_x > 0) ? layer_x : 0;
                    layer_y = (layer_y > 0) ? layer_y : 0;
                    if (gimp_drawable_has_alpha(layer_id))
                    {
                        for (i = 0; i < region_h; i++)
                        {
                            for (j = 0; j < region_w; j++)
                            {                                
                                if (layer_data[k+3] > PLT_ALPHA_THRESHOLD)
                                {
                                    plt_data[2*((i+layer_y)*plt_width+j+layer_x)] = (layer_data[k] + layer_data[k+1] + layer_data[k+2])/3;
                                    plt_data[2*((i+layer_y)*plt_width+j+layer_x)+1] = plt_id;
                                }
                                k+=bpp;
                            }                            
                        }
                    }
                    else
                    {
                        // No alpha value means bottom layers are not visible
                        // i.e. always overwrite everything
                        for (i = 0; i < region_h; i++)
                        {
                            for (j = 0; j < region_w; j++)
                            {
                                plt_data[2*((i+layer_y)*plt_width+j+layer_x)]= (layer_data[k] + layer_data[k+1] + layer_data[k+2])/3;
                                plt_data[2*((i+layer_y)*plt_width+j+layer_x)+1] = plt_id;
                                k += bpp;
                            }                            
                        }
                    }
                }
                gimp_progress_update((float) i/(float) detected_layers);
            }
            break;
        }
    }
    gimp_progress_update(1.0);
    g_free(layer_data);
    g_free(plt_layer_ids);

    // Adjust coordinates
    flip_plt(plt_data, plt_width, plt_height);

    // Write to file
    stream = fopen(filename, "wb");
    if (stream == 0)
    {
        g_message("Error opening %s\n", filename);
        g_free(plt_data);
        return (GIMP_PDB_EXECUTION_ERROR);
    }
    // Write header
    fwrite(plt_version, 1, 8, stream);
    fwrite(plt_info, 1, 8, stream);
    fwrite(&plt_width, 4, 1, stream);
    fwrite(&plt_height, 4, 1, stream);
    // Write image data
    fwrite(plt_data, 1, 2*plt_num_px, stream);
    fclose(stream);

    g_free(plt_data);

    return (GIMP_PDB_SUCCESS);
}


static GimpPDBStatusType plt_add_layers(gint32 image_id)
{
    if (!gimp_image_is_valid(image_id))
    {
        g_message("Invalid Image.\n");
        return (GIMP_PDB_EXECUTION_ERROR);
    }   
    unsigned int i, j;
    GimpImageBaseType img_basetype;
    gint img_num_layers;
    gint *img_layer_ids;
    gint img_width = 0;
    gint img_height = 0;
    GimpImageType layer_type= GIMP_GRAYA_IMAGE;
    gint32 layer_id;
    gint32 layer_pos;

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
        case GIMP_INDEXED: // Doesn't work
        default:
        {
            g_message("Image type has to be Grayscale or RGB.\n");
            return (GIMP_PDB_EXECUTION_ERROR);
        }
    }

    // We don't want to create already existing plt layers
    // Get all layers from the current image and look for missing layers
    img_width  = gimp_image_width(image_id);
    img_height = gimp_image_height(image_id);
    img_layer_ids = gimp_image_get_layers(image_id, &img_num_layers);
    gimp_image_undo_group_start(image_id);
    for (i = 0; i < PLT_NUM_LAYERS; i++)
    {
        layer_pos = -1;  // plt layer not present, need to insert
        for (j = 0; j < img_num_layers; j++)
        {
            if (!g_ascii_strcasecmp(PLT_LAYERS[i], gimp_item_get_name(img_layer_ids[j])))
            {
                layer_pos = j;
                break;
            }
        }
        if (layer_pos < 0)
        {
            // Determine position of new layer, previous layers should have already
            // been inserted, so we search for the previous one in the list
            if (i>0)
            {
                for (j = 0; j < img_num_layers; j++)
                {
                    if (!g_ascii_strcasecmp(PLT_LAYERS[i-1], gimp_item_get_name(img_layer_ids[j])))
                    {
                        layer_pos = j;
                        break;
                    }
                }     
            }
            else
            {
                layer_pos = img_num_layers;
            }
            layer_id = gimp_layer_new(image_id,
                                      PLT_LAYERS[i],
                                      img_width, img_height,
                                      layer_type,
                                      100.0,
                                      GIMP_NORMAL_MODE);
            gimp_image_insert_layer(image_id, layer_id, 0, layer_pos);
        }
    }
    gimp_image_undo_group_end(image_id);
    g_free(img_layer_ids);
    return (GIMP_PDB_SUCCESS);
}


GimpPlugInInfo PLUG_IN_INFO = {NULL, NULL, query, run};


MAIN()

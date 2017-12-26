#!/usr/bin/env python
# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 3
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####


# Plt File documentation
#
# First 8 bytes: "PLT V1  "
# 50 4C 54 20 56 31 20 20
# Next 8 bytes: Random? But these 8 bytes always work
# 0A 00 00 00 00 00 00 00
# Next 4 bytes: width
# 00 00 00 00
# Next 4 bytes: height
# 00 00 00 00
# The rest is data:
# AA 00 BB 01 ..., with AA 00 = (value, layer), BB 01 = (value, layer)


import os
import struct

from array import array
from gimpfu import *


# New layers can easily be added by extending this list, the script
# will automatically include them
# Add them to the end, list-position = plt-layer-idx (skin = 0)
PLT_LAYERS = ["skin", "hair", "metal1", "metal2", "cloth1", "cloth2",
              "leather1", "leather2", "tattoo1", "tattoo2"]


def plt_load(filename, raw_filename):
    f = open(filename, 'rb')
    # First 24 bytes contain header
    header = struct.unpack('<16s2I', f.read(24))
    if header[0][0:7] == 'PLT V1  ':
        gimp.pdb.gimp_message('Not a valid plt file' + header[0][0:8])
        f.close()
        return 1
    (width, height) = header[1], header[2]
    # Now read (color, layer) tuples - 1 byte for each value
    num_px = width * height
    plt_px = [struct.unpack('<2B', f.read(2)) for i in xrange(num_px)]
    f.close()
    # Adjust coordinates
    plt_px = [px for i in range(height)
              for px in
              plt_px[(height*width)-((i+1)*width):(height*width)-(i*width)]]

    # Create a new image
    img = gimp.Image(width, height, GRAY)
    img.filename = os.path.split(filename)[1]
    img.disable_undo()

    # Create Layers
    num_layers = len(PLT_LAYERS)
    gimp.progress_init("Creating layers ...")
    gimp.progress_update(0.0)
    for plt_id, lname in enumerate(PLT_LAYERS):
        # Create layer
        lay = gimp.Layer(img, lname, width, height,
                         GRAYA_IMAGE, 100, NORMAL_MODE)
        img.insert_layer(layer=lay, position=0)
        # Get region and write data
        region = lay.get_pixel_rgn(0, 0, width, height, True, True)
        lay_px = [[px[0], 255] if px[1] == plt_id else [0, 0] for px in plt_px]
        lay_px_data = array('B', [i for sl in lay_px for i in sl]).tostring()
        region[0:width, 0:height] = lay_px_data
        # Update image and progress
        lay.flush()
        lay.merge_shadow(TRUE)
        lay.update(0, 0, width, height)
        gimp.progress_update(float(plt_id+1)/float(num_layers))

    gimp.progress_update(1.0)
    img.enable_undo()
    return img


def plt_save(img, drawable, filename, raw_filename):
    f = open(filename, 'wb')
    # Get img data
    width = img.width
    height = img.height
    num_px = width * height
    # Write header
    plt = struct.pack('<8s2I2I', 'PLT V1  ', 10, 0, width, height)
    f.write(plt)

    # Search for layers containing plt data and save which plt layer
    # corresponds to which gimp layer
    # 1. Look for matching names
    plt_lay_ids = []
    for lay_id, layer in enumerate(img.layers):
        if layer.visible and layer.name.lower() in PLT_LAYERS:
            plt_lay_ids.append((PLT_LAYERS.index(layer.name.lower()), lay_id))
    # 2. Fallback: No layers haven been found
    #              Use the 10 top most layers instead
    if not plt_lay_ids:
        plt_lay_ids = [(lay_id, lay_id)
                       for lay_id, layer in enumerate(img.layers)]
    num_layers = len(plt_lay_ids)
    # Generate image data from layers
    gimp.progress_init("Processing layers...")
    gimp.progress_update(0.0)
    plt_px = [[255, 0] for i in xrange(num_px)]
    if img.base_type == GRAY:
        for plt_id, lay_id in reversed(plt_lay_ids):
            lay = img.layers[lay_id]
            region = lay.get_pixel_rgn(0, 0, width, height, False, False)
            bpp = region.bpp
            lay_px = array('B', region[0:width, 0:height])
            if lay.has_alpha:
                for i in xrange(0, num_px*bpp, bpp):
                    cval = lay_px[i]
                    aval = lay_px[i+1]
                    if aval > 0:
                        plt_px[i/bpp] = [cval, plt_id]
            else:
                for i in xrange(0, num_px*bpp, bpp):
                    cval = lay_px[i]
                    plt_px[i/bpp] = [cval, plt_id]
            gimp.progress_update(float(plt_id+1)/float(num_layers))
    elif img.base_type == RGB:
        for plt_id, lay_id in reversed(plt_lay_ids):
            lay = img.layers[lay_id]
            region = lay.get_pixel_rgn(0, 0, width, height, False, False)
            bpp = region.bpp
            lay_px = array('B', region[0:width, 0:height])
            if lay.has_alpha:
                for i in xrange(0, num_px*bpp, bpp):
                    cval = int(sum(lay_px[i:i+bpp-1])/(bpp-1))
                    aval = lay_px[i+bpp-1]
                    if aval > 0:
                        plt_px[i/bpp] = [cval, plt_id]
            else:
                for i in xrange(0, num_px*bpp, bpp):
                    cval = int(sum(lay_px[i:i+bpp])/bpp)
                    plt_px[i/bpp] = [cval, plt_id]
            gimp.progress_update(float(plt_id+1)/float(num_layers))
    else:  # Indexed, do nothing
        f.close()
        return
    gimp.progress_update(1.0)
    # Adjust coordinates
    plt_px = [px for i in range(height) for px in
              plt_px[(height*width)-((i+1)*width):(height*width)-(i*width)]]
    plt_data = struct.pack('<' + str(num_px*2) + 'B',
                           *[i for sl in plt_px for i in sl])
    f.write(plt_data)
    f.close()


def plt_create_layers(img):
    layer_type = GRAYA_IMAGE
    if img.base_type == RGB:
        layer_type = RGBA_IMAGE
    elif img.base_type == INDEXED:
        layer_type = INDEXEDA_IMAGE

    # Get all layers from the current image
    img_layernames = []
    for layer in img.layers:
        img_layernames.append(layer.name.lower())

    for layername in PLT_LAYERS:
        # We don't want to create already existing plt layers
        if layername not in img_layernames:
            lay = gimp.Layer(img, layername, img.width, img.height,
                             layer_type, 100, NORMAL_MODE)
            lay.fill(TRANSPARENT_FILL)
            img.insert_layer(layer=lay, position=0)


def register_load_handlers():
    gimp.register_load_handler('file-bioplt-load', 'plt', '')
    pdb['gimp-register-file-handler-mime']('file-bioplt-load', 'image/plt')
    # Too slow for python
    # pdb['gimp-register-thumbnail-loader']('file-bioplt-load',
    #                                       'file-bioplt-load-thumb')


def register_save_handlers():
    gimp.register_save_handler('file-bioplt-save', 'plt', '')
    pdb['gimp-register-file-handler-mime']('file-bioplt-save', 'image/plt')


register(
    'file-bioplt-load',  # name
    'load a Packed Layer Texture (.plt)',  # description
    'load a Packed Layer Texture (.plt)',
    'Attila Gyoerkoes',  # author
    'GPL v3',  # copyright
    '2015',  # year
    'Packed Layer Texture',
    None,  # image type
    [   # input args (type, name, description, default [, extra])
        (PF_STRING, 'filename', 'The name of the file to load', None),
        (PF_STRING, 'raw_filename', 'The name entered', None),
    ],
    [(PF_IMAGE, 'image', 'Output image')],  # results (type, name, description)
    plt_load,  # callback
    on_query=register_load_handlers,
    menu="<Load>",
)


register(
    'file-bioplt-save',  # name
    'save a Packed Layer Texture (.plt)',  # description
    'save a Packed Layer Texture (.plt)',
    'Attila Gyoerkoes',  # author
    'GPL v3',  # copyright
    '2015',  # year
    'Packed Layer Texture',
    'RGB*, GRAY*',
    [   # input args (type, name, description, default [, extra])
        (PF_IMAGE, "image", "Input image", None),
        (PF_DRAWABLE, "drawable", "Input drawable", None),
        (PF_STRING, "filename", "The name of the file", None),
        (PF_STRING, "raw-filename", "The name of the file", None),
    ],
    [],  # results (type, name, description)
    plt_save,  # callback
    on_query=register_save_handlers,
    menu='<Save>'
)


register(
    'file-bioplt-createlayers',  # name
    'Create Packed Layer Texture (.plt)',  # description
    'Create the layers for a Packed Layer Texture (.plt)',
    'Attila Gyoerkoes',  # author
    'GPL v3',  # copyright
    '2015',  # year
    'Plt: Create Layers',
    '*',
    [   # input args (type, name, description, default [, extra])
        (PF_IMAGE, "image", "Input image", None)
    ],
    [],  # results (type, name, description)
    plt_create_layers,  # callback
    # on_query = register_save_handlers,
    menu='<Image>/Tools'
)


main()

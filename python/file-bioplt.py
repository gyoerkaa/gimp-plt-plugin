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


# Don't change order, keep everything lowercase
PLT_LAYERS = ["skin", "hair", "metal1", "metal2", "cloth1", "cloth2", \
              "leather1", "leather2", "tattoo1", "tattoo2"]


def plt_load2(filename, raw_filename):
    f = open(filename, 'rb')
    # First 24 bytes contain header
    header = struct.unpack('<16s2I', f.read(24))
    if header[0][0:7] == 'PLT V1  ':
        gimp.pdb.gimp_message('Not a valid plt file' + header[0][0:8])
        f.close()
        return 1
    (width, height) = header[1], header[2]
    # The rest contains (color, layer) tuples - 1 byte for each value
    num_px = width * height
    px_data = [struct.unpack('<2B', f.read(2)) for i in xrange(num_px)]
    f.close()
    # Adjust coordinate system
    px_data = [p for i in range(height) for p in px_data[(height*width)-((i+1)*width):(height*width)-(i*width)]]
    # Create a new image
    img = gimp.Image(width, height, GRAY)
    img.filename = os.path.split(filename)[1]
    img.disable_undo()
    # Create Layers
    num_layers = len(PLT_LAYERS)
    gimp.progress_init("Creating layers ...")
    gimp.progress_update(0)
    for lidx, lname in enumerate(PLT_LAYERS):
        # Create layer
        lay = gimp.Layer(img, lname, width, height, GRAYA_IMAGE, 100, NORMAL_MODE)
        img.insert_layer(layer = lay, position = 0)
        # Get region and write data
        rgn = lay.get_pixel_rgn(0, 0, width, height, True, True)
        lay_px = [[px[0], 255] if px[1] == lidx else [0,0] for px in px_data]
        lay_px_data = array('B', [i for sl in lay_px for i in sl]).tostring()
        rgn[0:width, 0:height] = lay_px_data
        # Update image and progress
        lay.flush()
        lay.merge_shadow(TRUE)
        lay.update(0, 0, width, height)
        gimp.progress_update(float(lidx/num_layers))
    gimp.progress_update(1)
    img.enable_undo()
    return img


def plt_save2(img, drawable, filename, raw_filename):
    f = open(filename, 'wb')
    # Get img data
    width  = img.width
    height = img.height
    num_px = width * height
    # Write header
    pltdata = struct.pack('<8s', 'PLT V1  ')
    pltfile.write(pltdata)
    pltdata = struct.pack('<II', 10, 0)
    pltfile.write(pltdata)
    pltdata = struct.pack('<II', width, height)
    pltfile.write(pltdata)
    
    plt_data   = []
    # Search for layers containing plt data:
    # - First: By names
    # - Second: By order (if no names are found)
    plt_layer_ids = []
    for lid, lname in enumerate(PLT_LAYERS):
        if lname.lower() in img.layers:
            plt_layer_ids.append(img.layers)
            
     
    gimp.progress_init("Reading data from Gimp layers ...")
    gimp.progress_update(0)
    if img.base_type == GRAY:
        for lid in plt_layer_ids:
            gimp.progress_update(float(lid/len(layer_ids)))
    elif img.base_type == RGB:
        for lid in plt_layer_ids:
            gimp.progress_update(float(lid/len(layer_ids)))
    else: # Indexed, do nothing
        f.close()
        return
    f.close()

def plt_save(img, drawable, filename, raw_filename):
    pltfile = open(filename, 'wb')

    width  = img.width
    height = img.height

    pltdata = struct.pack('<8s', 'PLT V1  ')
    pltfile.write(pltdata)
    pltdata = struct.pack('<II', 10, 0)
    pltfile.write(pltdata)
    pltdata = struct.pack('<II', width, height)
    pltfile.write(pltdata)

    # Grab the top 10 layers and interpret them as
    # ['Skin', 'Hair', 'Metal1', 'Metal2', 'Cloth1', 'Cloth2', 'Leather1',
    #  'Leather2', 'Tattoo1', 'Tattoo2']
    data = []
    gimp.progress_init("Reading data from Gimp layers ...")
    gimp.progress_update(0)
    numpx = width * height

    # for speed
    l_int = int
    l_float = float
    l_floor = math.floor
    l_picklayer = img.pick_correlate_layer

    if img.base_type == GRAY:
        for i in range(numpx):
            x = l_int(i % width)
            y = height - l_int(l_floor(i / width)) - 1
            layer = l_picklayer(x, y)
            if layer >= 0:
                pxVal = layer.get_pixel(x,y)[0]
                try:
                    pxLayer = PLT_LAYERS.index(layer.name.lower())
                except ValueError:
                    pxLayer = 0
            else:
                pxVal = 255
                pxLayer = 0
            data.append(pxVal)
            data.append(pxLayer)
            gimp.progress_update(l_float(i)/l_float(numpx))

    elif img.base_type == RGB:
        for i in range(numpx):
            x = l_int(i % width)
            y = height - l_int(l_floor(i / width)) - 1
            layer = l_picklayer(x, y)
            if layer >= 0:
                pxVal = (layer.get_pixel(x,y)[0] + layer.get_pixel(x,y)[1] + layer.get_pixel(x,y)[2]) / 3
                try:
                    pxLayer = PLT_LAYERS.index(layer.name.lower())
                except ValueError:
                    pxLayer = 0
            else:
                pxVal = 255
                pxLayer = 0
            data.append(pxVal)
            data.append(pxLayer)
            gimp.progress_update(l_float(i)/l_float(numpx))

    else: # Indexed, do nothing
        pltfile.close()
        return

    pltdata = struct.pack('<' + str(numpx*2) + 'B', *data)
    pltfile.write(pltdata)
    pltfile.close()


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
            lay = gimp.Layer(img, layername, img.width, img.height, layer_type, 100, NORMAL_MODE)
            lay.fill(TRANSPARENT_FILL)
            img.insert_layer(layer = lay, position = 0)


def register_load_handlers():
    gimp.register_load_handler('file-bioplt-load', 'plt', '')
    pdb['gimp-register-file-handler-mime']('file-bioplt-load', 'image/plt')
    # Too slow for python
    #pdb['gimp-register-thumbnail-loader']('file-bioplt-load', 'file-bioplt-load-thumb')


def register_save_handlers():
    gimp.register_save_handler('file-bioplt-save', 'plt', '')
    pdb['gimp-register-file-handler-mime']('file-bioplt-save', 'image/plt')


register(
    'file-bioplt-load', #name
    'load a Packed Layer Texture (.plt)', #description
    'load a Packed Layer Texture (.plt)',
    'Attila Gyoerkoes', #author
    'GPL v3', #copyright
    '2015', #year
    'Packed Layer Texture',
    None, #image type
    [   #input args (type, name, description, default [, extra])
        (PF_STRING, 'filename', 'The name of the file to load', None),
        (PF_STRING, 'raw_filename', 'The name entered', None),
    ],
    [(PF_IMAGE, 'image', 'Output image')], #results (type, name, description)
    plt_load2, #callback
    on_query = register_load_handlers,
    menu = "<Load>",
)


register(
    'file-bioplt-save', #name
    'save a Packed Layer Texture (.plt)', #description
    'save a Packed Layer Texture (.plt)',
    'Attila Gyoerkoes', #author
    'GPL v3', #copyright
    '2015', #year
    'Packed Layer Texture',
    '*',
    [   #input args (type, name, description, default [, extra])
        (PF_IMAGE, "image", "Input image", None),
        (PF_DRAWABLE, "drawable", "Input drawable", None),
        (PF_STRING, "filename", "The name of the file", None),
        (PF_STRING, "raw-filename", "The name of the file", None),
    ],
    [], #results (type, name, description)
    plt_save, #callback
    on_query = register_save_handlers,
    menu = '<Save>'
)


register(
    'file-bioplt-createlayers', #name
    'Create Packed Layer Texture (.plt)', #description
    'Create the layers for a Packed Layer Texture (.plt)',
    'Attila Gyoerkoes', #author
    'GPL v3', #copyright
    '2015', #year
    'Plt: Create Layers',
    '*',
    [   #input args (type, name, description, default [, extra])
        (PF_IMAGE, "image", "Input image", None)
    ],
    [], #results (type, name, description)
    plt_create_layers, #callback
    #on_query = register_save_handlers,
    menu = '<Image>/Tools'
)


main()

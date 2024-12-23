
   +--------------------------+
   | V3dconverter quick start |
   +--------------------------+

* FOREWORD:
    This tools is under development. Only *.obj to *.3d conversion is fully operational at now. Convert *.3d terrain to *.obj in order to edit an height field seems to gives good results too. Convert other *.3d objects to *.obj or convert *.ac to *.3d will only work if you're lucky, therefore for now please report bugs only for *.obj to *.3d conversion. I do 99% of my testing by exporting files from Blender in *.obj format, then converting them to *.3d format with v3dconverter.

* DESCRIPTION:
    V3dconverter is a file converter for Sar2 developers. Its purpose is to convert *.3d (Vertex 3d) model or *.hf (terrain) files from/to *.obj (WaveFront) or *.ac (AC3D) files.

* HOW TO COMPILE:
    cd /path/to/v3dconverter
    gcc -lm -Wall -o v3dconverter ./src/v3dconverter.c

* HOW TO USE:
    ./v3dconverter -h

* IMPORTANT NOTES:
    - Don't stole 3D models. Create a model can take a lot of hours/days/months of hard work. Ensure that original model is free before add it in Sar2. If you're not sure of that, please try to contact model author and kindly ask him for permission to reuse it in Sar2.
    - Even if v3dconverter can convert *.ac to *.3d and even if FlightGear models are generally free, it is not a very good idea to become a "FlightGear models serial converter". FlightGear models -especially flying ones- can be very detailed, generally too much detailed for Sar2. A better way is to open and simplify them as much as possible (I use mainly Blender and / or sometimes Meshlab to do that) BEFORE convert them to *.3d format.
    - V3dconverter will only do for you the longest and less interesting job, i.e. convert 3D primitives. To get clean Sar2 models, you will certainly have to manually add/remove/modify some stuffs in your *.3d file, and you always will have to convert texture files.

* CONVERSION TIPS:
    - Sar2 texture (*.tex) is a rebadged *.tga (Truevision Targa), 3*8 bits rgb or 4*8 bits rgba color, top left origin, without RLE (Run Length Encoding) compression image.
    - Remember that in Sar2, a pure black color (0x000000) will be fully transparent. V3dconverter will check and automatically overwrite "pure black" by "almost black" (0x010101) in color statements. If you see some holes in your *.3d textured model, maybe your *.tex file contains pure black color. To replace pure black color in a texture, you can of course do it with an image editor or, in command line, you can do it with Imagemagick (https://imagemagick.org/) as this: "convert myoriginaltexture.tga -fill 'rgb(1, 1, 1)' -opaque black mynewtexture.tga". Then, rename your *.tga file to *.tex, and check your model in Sar2 again. Of course, due to Imagemagick magic, "myoriginaltexture.tga" can be "myoriginaltexture.jpg" or "myoriginaltexture.png".
    - Because of a pure black (0x000000) colored pixel will be fully transparent, if you don't need semi-transparency, preferably use 3*8 bits rgb image instead of 4*8 bits rgba: file size of your image will be 25% lowered!
    - When you load a new model in Sar2, start Sar2 from command line. If Sar2 can't properly load your model, it will explain why to you.
    - V3dconverter will only work with *.obj and *.ac files. Blender and Meshlab can export to *.obj: do not forget to export "Materials groups" (Blender) or "Texcoord / Texture file" (Meshlab). You can also try assimp (https://www.assimp.org/), which one works as backend of a lot of web 3D converters. Assimp quick start example: "assimp export inputFileName.3ds outputFileName.obj".
    - When you convert a model to Vertex 3d, v3dconverter will print objects dimensions: always check if they are coherent for your model. If not, use "-sc" option to scale. For example, if object X dimension is 230 meters but should be 2.3 meters, just add "-sc 2.3/230" or "-sc 1/100" or "-sc 0.01" to v3dconverter command line.
    - Generally, we want object "Zmin" (minimal vertical) value to be zero. As v3dconverter prints object rectangular bounds, you can easy check it and if necessary modify Zmin value using "-tr" (translate) option. For example, if object Zmin value is 12.3 meters but should be 0, just add "-tr 0 0 -12.3" to v3dconverter command line.
    - Beware, some models have their origin far away from main object center. If you don't see you model in Sar2, first zoom out and check if you see it.
    - If your *.3d model is not visible, check if alpha (transparency) parameter is rightly set to 1.0 : look for "color" in *.3d file, alpha chanel is the fourth value.
    - If your *.3d model seems to have all faces inverted (for example, if when you look at your model from front of it, you see its back texture), you can try "-if" option to invert faces visibility (flip winding).
    - If you have removed all pure black colors and it already seems that some parts of you model are missing, maybe that missing faces have to be flipped. This can be checked with Blender by showing face orientation. Select bad meshes then use Mesh -> Normals -> Flip to flip them.
    - Especially if you wants to create new aircrafts, don't hesitate to save your *.obj or *.ac file into small parts, then convert these parts to *.3d, then manually reassemble your multiple *.3d files in the final one. If not, it can be very difficult to define aircraft moving parts (begin_model ... / end_model ...) as rotors, gears, doors, flaps, an so on...
    - If you want to understand "how aircraft moving parts work", it can be usefull to convert an existing *.3d aircraft file to *.obj using the v3dconverter '-do-not-transform' option: moving parts will be drawn "as sar2 see them when it reads the *.3d file".
    - To check your model rendering in Sar2 without add it in a scenery, copy or link your *.3d file in data/aircrafts/ directory, then start Sar2 (aircrafts list is loaded at startup), go to Free Flight, click on Aircraft button, select your model in list, then click on Details button. If your model is not a flying one, don't forget to remove your file / link from aircrafts directory once checked.
    - Sometimes, v3dconverter prints a lot of (too much ?) warnings. These warnings can be redirected in a text file as this: v3dconverter -i "my input file.obj" -o my_output_file.3d 2> warnings.txt .
    - If in Blender, your imported *.obj model has no texture and appears pink, it's because Blender can't find the path of textures. This can be fixed by creating a link to the textures folder (for example: "cd my_working_directory && ln -s /usr/share/sar2/textures/"), or by copying the whole textures folder in your working directory, or by editing your model *.mtl file (look for "map_Kd ..........." in *.mtl file).
    - A Sar2 height field (*.hf) is a rebadged *.tga (Truevision Targa), 8 bits grey color, top left origin, without RLE (Run Length Encoding) compression image.
    - Keep in mind that *.3d terrain to *.obj conversion -> manual terrain editing -> *.obj to *.hf conversion, is only available for altitude ("Z axis") modification. X and/or Y edition will cause *.obj to *.hf conversion to stop.
    - Keep in mind that v3dconverter uses *.3d file data to convert terrain to *.obj, thus it will place your *.obj terrain "as in Sar2" (unless you specified the '-do-not-transform' option). It is very usefull if your terrain is made from multiple tiles, because once each terrain tile converted to *.obj, you can import all of them in Blender and they will automatically be put at the right place, but, if you import only one tile and if translation in *.3d file is not null, terrain can be far away from view center (0,0,0). If you want that it be centered in Blender view, DO NOT move it from Blender, add a -tr (translate) option to your conversion command line, or use the '-do-not-transform' option. If you move it from Blender, *.obj to *.hf conversion will fail.
    - If you want to edit a terrain file in Blender, it's a good idea to add a -sc 1/100 (i.e. scale 1/100) or -sc 1/1000 option to your v3dconverter command line. If not and if -like me- you are not a Blender professional, it will be difficult to see your terrain because it will be too big for camera clipping and focal length. Don't worry about scale value: it will be stored during *.3d to *.obj conversion process, then automatically canceled during *.obj to *.hf conversion.
    - Warning: in *.obj terrain files, material has a strange name: this name don't be modified because it will be used as a file name to retrieve important data during *.obj to *.hf conversion process. If you're curious, have a look in $HOME/.local/share/v3dconverter.
    - You can easily compare two *.hf files with ImageMagick like this: "magick compare image_1.hf.tga image_2.hf.tga -compose src difference.png".

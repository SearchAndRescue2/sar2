 
   +--------------------------+
   | V3dconverter quick start |
   +--------------------------+

* FOREWORD:
    This tools is under development.
    Only *.obj to *.3d conversion is fully operational at now.
    Convert *.3d to *.obj or *.ac to *.3d is experimental and will only work if you're lucky, therefore please report bugs only for *.obj to *.3d conversion.
    
* DESCRIPTION:
    V3dconverter is a command line 3d models converter for Sar2 developers. Its purpose is to convert *.3d (Vertex 3d) file format from/to *.obj (WaveFront) or *.ac (AC3D) formats. It will not work for *.3d terrain files.

* HOW TO COMPILE:
    cd /path/to/v3dconverter
    gcc -lm -Wall -o v3dconverter ./src/v3dconverter.c

* HOW TO USE:
    ./v3dconverter -h

* IMPORTANT NOTES:
    - Don't stole 3D models. Create a model can take a lot of hours/days/months of hard work. Ensure that original model is free before add it in Sar2. If you're not sure of that, please try to contact model author and kindly ask him for permission to reuse it in Sar2.
    - Even if v3dconverter can convert *.ac to *.3d and even if FlightGear models are generally free, it is not a very good idea to become a "FlightGear models serial converter". FlightGear models -especially flying ones- can be very detailed, generally too much detailed for Sar2. A better way is to open and simplificate them as much as possible (I do that with Blender and / or Meshlab) BEFORE convert them to *.3d format.
    - V3dconverter will only do for you the longest and less interesting job, i.e. convert 3D primitives. To get clean Sar2 models, you will certainly have to manually add/remove/modify some stuffs in your *.3d file, and you always will have to convert texture files to *.tga format.

* CONVERSION TIPS:
    - Sar2 texture (*.tex) is a rebadged *.tga (Truevision Targa), 4*8 bits rgba color, top left origin, without RLE (Run Length Encoding) compression.
    - Remember that in Sar2, a pure black color (0x000000) will be transparent. V3dconverter will check and automatically overwrite "pure black" by "almost black" (0x010101) in color statements. If you see some holes in your *.3d textured model, maybe your *.tex file contains pure black color. To replace pure black color in a texture, you can of course do it with an image editor or, in command line, you can do it with Imagemagick (https://imagemagick.org/) as this: "convert myoriginaltexture.tga -fill 'rgb(1, 1, 1)' -opaque black mynewtexture.tga". Then, rename your *.tga file to *.tex, and check your model in Sar2 again. Of course, due to Imagemagick magic, "myoriginaltexture.tga" can be "myoriginaltexture.jpg" or "myoriginaltexture.png".
    - When you load a new model in Sar2, start it from command line. If Sar2 can't properly load your model, it will explain why to you.
    - V3dconverter will only work with *.obj and *.ac files. Blender and Meshlab can export to *.obj, just don't forget to export "Materials groups" in Blender or "Texcoord / Texture file" in Meshlab. You can also try assimp (https://www.assimp.org/), which one works as backend of a lot of Web 3D converters. Assimp quick start example: "assimp export inputFileName.3ds outputFileName.obj".
    - When you convert a model to Vertex 3d, v3dconverter will print object dimensions: always check if they are coherent for your model. If not, use "-sc" (scale) option. For example, if object X dimension is 230 meters but should be 2.3 meters, just add "-sc 2.3/230" or "-sc 1/100" or "-sc 0.01" to v3dconverter command line.
    - Generally, we want object "Zmin" (minimal vertical) value to be zero. As v3dconverter prints object rectangular bounds, you can easy check it and if necessary modify Zmin value using "-tr" (translate) option. For example, if object Zmin value is 12.3 meters but should be 0, just add "-tr 0 0 -12.3" to v3dconverter command line.
    - Beware, some models found on the Web have their origin far away from main object center. If you don't see you model in Sar2, first zoom out and check if you see it. Of course, the better way is to move this object in your 3d editor before convert it to *.3d .
    - If your *.3d model is not visible, check if alpha (transparency) parameter is set to 1.0 : look for "color" in *.3d file, alpha chanel is the fourth value. I experienced that in some *.obj files found on the Web because of their "Tr" parameter was wrongly set. V3dconverter will warn you about that.
    - If your *.3d model seems to have all faces inverted (for example, if when you look at your model from front of it, you see its back texture), you can try "-if" option to invert faces visibility (flip winding).
    - If you have removed all pure black colors and it already seems that some parts of you model are missing, maybe that missing faces have to be flipped. This can be checked with Blender by showing face orientation. Select bad meshes then use Mesh -> Normals -> Flip to reorient them.
    - Especially if you wants to create new aircrafts, don't hesitate to save your *.obj or *.ac file into small parts, then convert these parts to *.3d, then manually reassemble your multiple *.3d files in the final one. If not, it can be very difficult to define aircraft moving parts (begin_model ... / end_model ...) as rotors, gears, doors, flaps, an so on...
    - To quick check your model rendering in Sar2 without add it in a scenery, you can copy or link your *.3d file in data/aircrafts/ directory, then start Sar2 (aircrafts list is loaded at startup), go to Free Flight, click on Aircraft button, select your model in list, then click on Details button. If your model is not a flying one, don't forget to remove your file / link from aircrafts directory once checked.
    - Sometimes, v3dconverter will print a lot of (too much ?) warnings. These warnings can be redirected in a text file as this: v3dconverter -i "my input file.obj" -o my_output_file.3d 2> warnings.txt .
    - If you try *.3d to *.obj conversion, Blender seems to read converted models better than Meshlab. I didn't try to understand why (as said, *.3d to *.obj conversion is experimental), and I didn't try with other 3d softwares.

/**********************************************************************
*   This file is part of Search and Rescue II (SaR2).                 *
*                                                                     *
*   SaR2 is free software: you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License v.2 as       *
*   published by the Free Software Foundation.                        *
*                                                                     *
*   SaR2 is distributed in the hope that it will be useful, but       *
*   WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See          *
*   the GNU General Public License for more details.                  *
*                                                                     *
*   You should have received a copy of the GNU General Public License *
*   along with SaR2.  If not, see <http://www.gnu.org/licenses/>.     *
***********************************************************************/

//
// Each line must start by '__' (two underline characters). If you need '__' in your text, use '&#95;' (HTML '_' character) for at least one of both (i.e. '&#95;_').
// Each line must be terminated by a '\' (backslash) character. If you need '\' (backslash) in your text, escape it with an other '\' (i.e. '\\').
//
// Key words:
// NAME : parameter name, without any space.
// SYNOPSIS : first word is parameter name and will be automatically bolded, following words are arguments. For optional arguments, space before and after brackets ('[' ']') are mandatory. Example: 'parameterName arg1 arg2 [ opt_arg ] [ ... ]'. 
// DESCRIPTION : what the parameter do.
// ARGUMENTS : parameter arguments. One argument per line. First word must be argument name, all following words are argument description. Arguments will be drawn in a HTML table. Use '(none)' as argument name if parameter has no argument.
// CONTEXT values: use 'mis' for mission (*.mis file), '3d' for model (*.3d file) and 'scn' for scenery (*.scn file). At least one context is mandatory. Space is mandatory between two context names.
// EXAMPLE (or EXAMPLES) : nothing special here.
// ----- : five minus characters closes a parameter section. '\' character before and after section close are not mandatory, but increase ligibility of this file.
//
// Usefull tips:
// Do NOT use '<' and '>' characters. Use '&gt;' and '&lt' instead. HTML "space" character '&nbsp;' can sometimes be usefull too.
// '##' prefix will create a link. For example, '##my_destination' will create a link to 'my_destination'. If 'my_destination' starts with 'http', an external link will be created. If not, a winthin page link (hash) will be created.
// Do NOT use '\n' (newline) character. HTML will automagically format your text.
// It is possible, but not recommended in order to preserve document's structure and legibility, to add html tags. Usefull tags can be:  '<hr>', '<b>', '</b>' and '<br>'.
//
// Alphabetic sort of parameters names is NOT automatic, please do it manually!
//
// Q: Why use a variable to store text and not simply load a local text file?
// A: Because browsers don't like to load local text files for securiy reason ("Cross-Origin Request" error).
//
// Q: Why not use Markdown?
// A: Because I think that scrolling menu it is not possible with Markdown.
//

var cmdDescription="\
__NAME\
__add_fire\
__SEE\
__##create_fire parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__add_helipad\
__SEE\
__##create_helipad parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__add_human\
__SEE\
__##create_human parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__add_object\
__SEE\
__##create_object parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__add_premodeled\
__SEE\
__##create_premodeled parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__add_runway\
__SEE\
__##create_runway parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__add_smoke\
__SEE\
__##create_smoke parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__aileron_elevator_left_new\
__SEE\
__##---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;--- .\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__aileron_elevator_right_new\
__SEE\
__##---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;--- .\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__aileron_left_new\
__SEE\
__##---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;--- .\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__aileron_right_new\
__SEE\
__##---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;--- .\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__air_brake_new\
__SYNOPSIS\
__air_brake_new x y z heading pitch bank dep_dheading dep_dpitch dep_dbank deployed? anim_rate visible_when_retracted?\
__DESCRIPTION\
__New air brake.\
__ARGUMENTS\
__x air brake X position.\
__y air brake Y position.\
__z air brake Z position.\
__heading air brake heading direction.\
__pitch air brake pitch direction.\
__bank air brake bank direction.\
__dep_dheading deployed air brake heading direction.\
__dep_dpitch deployed air brake pitch direction.\
__dep_dbank deployed air brake bank direction.\
__deployed? air brake initially deployed ?\
__anim_rate animation rate\
__visible_when_retracted? is air brake visible when retracted ?\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__air_brakes\
__SYNOPSIS\
__air_brakes area\
__DESCRIPTION\
__Air brakes.\
__ARGUMENTS\
__area air brakes exposed area when deployed, in square meters.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__airplane_acceleration_responsiveness\
__SYNOPSIS\
__airplane_acceleration_responsiveness i j k\
__DESCRIPTION\
__Airplane acceleration responsiveness.\
__ARGUMENTS\
__i must be positive.\
__j must be positive.\
__k must be positive.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__airplane_accelresp\
__SEE\
__##airplane_acceleration_responsiveness parameter.\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__attitude_change_rate\
__SYNOPSIS\
__attitude_change_rate heading pitch bank\
__DESCRIPTION\
__Attitude change rates.\
__ARGUMENTS\
__heading heading change rate, in degrees per second.\
__pitch pitch change rate, in degrees per second.\
__bank bank change rate, in degrees per second.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__attitude_leveling\
__SYNOPSIS\
__attitude_leveling heading pitch bank\
__DESCRIPTION\
__Attitude leveling.\
__ARGUMENTS\
__heading heading attitude leveling, in degrees per second.\
__pitch pitch attitude leveling, in degrees per second.\
__bank bank attitude leveling, in degrees per second.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__author\
__SYNOPSIS\
__author author_name\
__DESCRIPTION\
__Name of the 3d model file author.\
__ARGUMENTS\
__author_name name of 3d model author. 80 characters maximum.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__begin_header\
__SYNOPSIS\
__begin_header\
__DESCRIPTION\
__Begins a header block, which defines the V3D file header. This should be the first V3D data block in the file.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_header\
__...\
__##end_header\
\
__-----\
\
__NAME\
__begin_lines\
__SYNOPSIS\
__begin_lines\
__DESCRIPTION\
__Marks the start bound for a series of vertex statement(s) that describe line segment(s).\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_lines\
__...\
__##end_lines\
\
__-----\
\
__NAME\
__begin_line_loop\
__SYNOPSIS\
__begin_line_loop\
__DESCRIPTION\
__Marks the start bound for a series of vertex statement(s) that describe a line loop.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_line_loop\
__...\
__##end_line_loop\
\
__-----\
\
__NAME\
__begin_line_strip\
__SYNOPSIS\
__begin_line_strip\
__DESCRIPTION\
__Marks the start bound for a series of vertex statement(s) that describe a line strip.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_line_strip\
__...\
__##end_line_strip\
\
__-----\
\
__NAME\
__begin_model\
__SYNOPSIS\
__begin_model model_type\
__DESCRIPTION\
__Begins a model block, which defines a V3D model. You can specify one or more in a V3D file. Each should have a different name, the first one is (by tradition) named standard.\
__ARGUMENTS\
__model_type standard\
__ standard_dawn\
__ standard_dusk\
__ standard_far\
__ standard_night\
__ rotor\
__ aileron_left\
__ aileron_right\
__ rudder_top\
__ rudder_bottom\
__ elevator\
__ cannard\
__ aileron_elevator_left\
__ aileron_elevator_right\
__ flap\
__ air_brake\
__ landing_gear\
__ door\
__ fueltank\
__ cockpit\
__ shadow\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_model standard_far\
__...\
__##end_model standard_far\
\
__-----\
\
__NAME\
__begin_points\
__SYNOPSIS\
__begin_points\
__DESCRIPTION\
__Marks the start bound for a series of vertex statement(s) that describe point(s).\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_points\
__...\
__##end_points\
\
__-----\
\
__NAME\
__begin_polygon\
__SYNOPSIS\
__begin_polygon\
__DESCRIPTION\
__Marks the start bound for a series of vertex statement(s) that describe a polygon.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_polygon\
__...\
__##end_polygon\
\
__-----\
\
__NAME\
__begin_quads\
__SYNOPSIS\
__begin_quads\
__DESCRIPTION\
__Marks the start bound for a series of vertex statement(s) that describe quad(s).\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_quads\
__...\
__##end_quads\
\
__-----\
\
__NAME\
__begin_quad_strip\
__SYNOPSIS\
__begin_quad_strip\
__DESCRIPTION\
__Marks the start bound for a series of vertex statement(s) that describe a quad strip.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_quad_strip\
__...\
__##end_quad_strip\
\
__-----\
\
__NAME\
__begin_triangles\
__SYNOPSIS\
__begin_triangles\
__DESCRIPTION\
__Begins a triangles section.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_triangles\
__...\
__##end_triangles\
\
__-----\
\
__NAME\
__begin_triangle_fan\
__SYNOPSIS\
__begin_triangle_fan\
__DESCRIPTION\
__Marks the start bound for a series of vertex statement(s) that describe a triangle fan.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_triangle_fan\
__...\
__##end_triangle_fan\
\
__-----\
\
__NAME\
__begin_triangle_strip\
__SYNOPSIS\
__begin_triangle_strip\
__DESCRIPTION\
__Marks the start bound for a series of vertex statement(s) that describe a triangle strip.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_triangle_strip\
__...\
__##end_triangle_strip\
\
__-----\
\
__NAME\
__belly_height\
__SYNOPSIS\
__belly_height height\
__DESCRIPTION\
__Belly height.\
__ARGUMENTS\
__height belly height, in meters.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__cannard_new\
__SEE\
__##---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;--- .\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__cockpit_offset\
__SYNOPSIS\
__cockpit_offset x y z\
__DESCRIPTION\
__Cockpit offset.\
__ARGUMENTS\
__x \
__y \
__z \
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__color&nbsp;(in&nbsp;*.3d&nbsp;header)\
__SYNOPSIS\
__color name r g b a [ ambient diffuse specular shininess emission ]\
__DESCRIPTION\
__Specifies a preset color specified by 'name' who's values are those given, each in the range [0.0 to 1.0] (including <shininess> which OpenGL has a different bounds for). //FIXME from V3D format doc: \"Implementation of this not complete as of this writing\".\
__ARGUMENTS\
__name color name\
__r red\
__g green\
__b blue\
__a alpha (transparency)\
__ambient \
__diffuse \
__specular \
__shininess \
__emission \
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__color&nbsp;(in&nbsp;*.3d&nbsp;model&nbsp;primitive)\
__SYNOPSIS\
__color r g b a [ ambient diffuse specular shininess emission ]\
__DESCRIPTION\
__Specifies the new color and material values, each in the range [0.0 to 1.0] (including <shininess> which OpenGL has a different bounds for). \
__ARGUMENTS\
__r red\
__g green\
__b blue\
__a alpha (transparency)\
__ambient \
__diffuse \
__specular \
__shininess \
__emission \
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__contact_cylendrical\
__SYNOPSIS\
__contact_cylendrical radius height_min height_max\
__DESCRIPTION\
__Defines cylendricals contact bounds for interaction with other objects. Must be used in conjunction with the ##crash_flags parameter.\
__ARGUMENTS\
__radius radius of the cylinder, in meters.\
__height_min min height of the cylinder, in meters. Must be lower than height_max.\
__height_max max height of the cylinder, in meters. Must be greater than height_min.\
__CONTEXT\
__3d scn\
__EXAMPLE\
__# Crash flags\
__crash_flags 0 1 0 0\
__# Tree (palm01) contact bounds\
__contact_cylendrical 2.0 0.0 6.0\
\
__-----\
\
__NAME\
__contact_rectangular\
__SYNOPSIS\
__contact_rectangular x_min x_max y_min y_max z_min z_max\
__DESCRIPTION\
__Defines rectangulars contact bounds for interaction with other objects. Must be used in conjunction with the ##crash_flags parameter.\
__ARGUMENTS\
__x_min min value in X direction, in meters. Must be lower than x_max.\
__x_max max value in X direction, in meters. Must be greater than x_min.\
__y_min min value in Y direction, in meters. Must be lower than y_max.\
__y_max max value in Y direction, in meters. Must be greater than y_min.\
__z_min max value in Z direction, in meters. Must be lower than z_max.\
__z_max max value in Z direction, in meters. Must be greater than z_min.\
__CONTEXT\
__3d scn\
__EXAMPLE\
__# Crash flags\
__crash_flags 0 1 0 3\
__# Building contact bounds\
__contact_rectangular -6.0 6.0 -6.0 6.0 0.0 28.0\
\
__-----\
\
__NAME\
__contact_spherical\
__SYNOPSIS\
__contact_spherical radius\
__DESCRIPTION\
__Defines sphericals contact bounds for interaction with other objects. Must be used in conjunction with the ##crash_flags parameter.\
__ARGUMENTS\
__radius radius of the sphere, in meters.\
__CONTEXT\
__3d scn\
__EXAMPLE\
__# Crash flags\
__crash_flags 0 1 0 0\
__# Contact bounds\
__contact_spherical 20.3\
\
__-----\
\
__NAME\
__control_panel\
__SYNOPSIS\
__control_panel x y z heading pitch bank width height path\
__DESCRIPTION\
__Control panel position.\
__ARGUMENTS\
__x control panel X position, in centimeters.\
__y control panel Y position, in centimeters.\
__z control panel Z position, in centimeters.\
__heading control panel heading, in degrees.\
__pitch control panel pitch, in degrees.\
__bank control panel bank, in degrees.\
__width width, in centimeters.\
__height height, in centimeters.\
__path specifies the control panel directory (not the instruments file)\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__crash_flags\
__SYNOPSIS\
__crash_flags crash_other cause_crash support_surface crash_type\
__DESCRIPTION\
__Defines how the object can be contacted by others. Must be used in conjonction with a ##contact_cylendrical , ##contact_rectangular or ##contact_spherical parameter.\
__ARGUMENTS\
__crash_other can be: '1' if this object can crash into or contact other objects, '0' if it can't. Can be '1' ONLY for 'human' and 'aircraft' objects, and in this case, contact bounds MUST be cylendrical (see ##contact_cylendrical parameter). Use it for mobile objects.\
__cause_crash can be: '1' if other objects can crash into this object, '0' if they can't. MUST be '0' if 'crash_other' flag is '1'. Use it for fixed objects.\
__support_surface must be: '1' if this object is landable/walkable, '0' if not. If '1', then the object can only have its heading rotated. MUST be '0' if 'crash_other' flag is '1'.\
__crash_type can be: 0 (obstruction), 1 (ground), 2 (mountain), 3 (building), 4 (aircraft). Defines collision message type.\
__CONTEXT\
__3d //FIXME and scene file (*.scn) ?\
__EXAMPLE\
__# Crash flags\
__crash_flags 0 1 0 0\
__# Contact bounds\
__contact_spherical 20.3\
\
__-----\
\
__NAME\
__create_fire\
__SYNOPSIS\
__create_fire radius height\
__DESCRIPTION\
__Creates a fire. At least one object ( see ##create_object ) must be created in mission / scenery file. If not, fire will not be generated.\
__ARGUMENTS\
__radius radius of the fire, in meters.\
__height height of the fire above its base, in feet.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Fire\
__create_fire 1.0 5.0\
__# Fire position\
__translate 30000.0 41020.0 0.0\
\
__-----\
\
__NAME\
__create_helipad\
__SYNOPSIS\
__create_helipad style length width recession label edge_lighting has_fuel has_repair has_drop_off restarting_point [ ref_obj_name offset_x offset_y offset_z offset_h offset_p offset_b ]\
__DESCRIPTION\
__Creates an helipad.\
__ARGUMENTS\
__style Can be: 'default' or 'standard' or 'ground_paved' (same result), 'ground_bare' (unpaved), 'building' (roof top of last drawn building), 'vehicle' (on a vehicle or a vessel).\
__length length, in meters.\
__width width, in meters.\
__recession recession\'s height, in feet.\
__label label of the helipad, which will be \"painted\" on the ground.\
__edge_lighting has edge lighting? Can be: y (yes) or n (no).\
__has_fuel has fuel? Can be: y (yes) or n (no). If 'y', aircraft can be refueled here.\
__has_repair has repair? Can be: y (yes) or n (no). If 'y', aircraft can be repaired here.\
__has_drop_off has drop off? Can be: y (yes) or n (no). If 'y', human can be dropped off here.\
__restarting_point is a restarting point? Can be y (yes) or n (no). //FIXME Not used at now. Need more explanations !\
__ref_obj_name reference object name. Can be omitted. //FIXME Can be omitted only if helipad is on a fixed object?\
__&nbsp;&nbsp;&nbsp;&nbsp;&#9507;&#9473;&nbsp;offset_x X offset from reference object, in meters. Must be omitted if there is no reference object.\
__&nbsp;&nbsp;&nbsp;&nbsp;&#9507;&#9473;&nbsp;offset_y Y offset from reference object, in meters. Must be omitted if there is no reference object.\
__&nbsp;&nbsp;&nbsp;&nbsp;&#9507;&#9473;&nbsp;offset_z Z offset from reference object, in feet. Must be omitted if there is no reference object.\
__&nbsp;&nbsp;&nbsp;&nbsp;&#9507;&#9473;&nbsp;offset_h heading offset from reference object, in degrees (clockwise rotation around the Z axis). Must be omitted if there is no reference object.\
__&nbsp;&nbsp;&nbsp;&nbsp;&#9507;&#9473;&nbsp;offset_p pitch offset from reference object, in degrees (rotation around the Y axis). Must be omitted if there is no reference object.\
__&nbsp;&nbsp;&nbsp;&nbsp;&#9495;&#9473;&nbsp;offset_b bank offset from reference object, in degrees (rotation around the X axis). Must be omitted if there is no reference object.\
__CONTEXT\
__mis scn\
__EXAMPLES\
__# Los Angeles International (LAX)\
__create_helipad default 40.0 40.0 0.0 LAX y y y y y\
__no_depth_test\
__object_name helipad_lax\
__object_map_description Helipad - Los Angeles International\
__range 4000\
__translation -5430 35750 0\
__<br>\
__# UCLA hospital\
__#                 type     range length width height  walls_texture  walls_night_texture  roof_texture\
__create_premodeled building 5000  110    60    90      building02_tex building01_night_tex wall01_tex\
__translation -4900 44800 0\
__# Helipad at UCLA hospital's roof.\
__create_helipad building 30.0 30.0 3.0 UCLA y y y y y\
__object_name helipad_ucla\
__object_map_description Helipad - UCLA Hospital\
__range 4000\
__# building\'s height is 90 feet, and helipad\'s recession height is 3 feet, so helipad\'s Z must be 93 feet.\
__translation -4920 44800 93\
__<br>\
__# Helipad on vessel\
__create_helipad vehicle 29.0 18.0 3.2 USCG y y y y y uscg378_01 0.0 -58.5 36.0 0.0 0.0 0.0\
__object_name helipad_default\
__object_map_description USCG Cutter Helipad\
__range 2200\
\
__-----\
\
__NAME\
__create_human\
__SYNOPSIS\
__create_human type_name flag [ flag ] [ ... ]\
__DESCRIPTION\
__Creates a human. Note: human types are definite in 'human.ini' file.\
__ARGUMENTS\
__<b>type_name</b> <hr>\
__default default human. May be used whenever nothing in particular is requested.\
__diver used for drawing at end of hoist rope.\
__victim_streatcher_assisted victim is intended to be on a stretcher and one assisting human will be drawn.\
__<b>flags</b> <hr>\
__need_rescue this human needs to be rescued (player has to take him aboard).\
__sit_up human is drawn sitting 'like on a chair', feet on the floor.\
__sit_down human is drawn sitting 'like on the floor', legs straight out in front of him.\
__sitting human is drawn sitting 'like on a chair', buttocks (not feet) on the floor.\
__lying human is drawn lying horizontally (on the floor or on a stretcher, see on_streatcher flag below).\
__alert human alerts (moves his arms to draw attention).\
__aware human is aware. //FIXME need more explanations !\
__in_water human swims (moves his arms to float).\
__on_streatcher if this flag is specified, a stretcher is drawn below the human.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Human\
__create_human default need_rescue alert aware in_water\
__# Human position.\
__# If human 'z' position is greater than the ground / water / object which is under him, human will 'fall' until he comes into contact.\
__translate 30000.0 41020.0 0.0\
\
__-----\
\
__NAME\
__create_object\
__SYNOPSIS\
__create_object object_type\
__DESCRIPTION\
__Creates an object. Note: you can look for 'Object Types:' in obj.h file.\
__ARGUMENTS\
__<b>object_type</b> <hr>\
__0 (garbage).\
__1 object is static. A house, a tree, or even a car or a boat which don't move are static objects.\
__2 object is an automobile.\
__3 object is a watercraft.\
__4 object is an aircraft (helicopter or airplane).\
__5 (not used, old 'airplane' object type).\
__6 object is ground, or a landable object.\
__7 object is a runway. Don't use \"create_object 7\": use ##create_runway instead.\
__8 object is an helipad. Don't use \"create_object 8\": use ##create_helipad instead.\
__9 object is a human. Don't use \"create_object 9\": use ##create_human instead.\
__10 object is smoke. Don't use \"create_object 10\": use ##create_smoke instead.\
__11 object is a fire. Don't use \"create_object 11\": use ##create_fire instead.\
__12 object is an explosion. //FIXME : seems to be used in sar2 code only (it's not possible to use it in missions and sceneries).\
__13 object is chemical spray: water, fire-retardant, etc. Note: not yet implemented.\
__14 object is a fuel tank, dropped from an aircraft.\
__20 object is premodeled. Don't use \"create_object 20\": use ##create_premodeled instead.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Jetski\
__create_object 3\
__model_file vessels/jetski.3d\
__translation 36 -193 0\
__rotate 200 0 0\
\
__-----\
\
__NAME\
__create_premodeled\
__SYNOPSIS\
__create_premodeled type range height hazard_lights\
__create_premodeled type range length width height walls_texture roof_texture\
__create_premodeled type range length width height walls_texture walls_texture_night roof_texture\
__DESCRIPTION\
__Creates a pre-modeled object.\
__ARGUMENTS\
__ <hr>\
__ if <b>type</b> is '<b>power_transmission_tower</b>', '<b>radio_tower</b>' or '<b>tower</b>', then arguments are:\
__range visible range, in meters.\
__height height, in feet.\
__hazard_lights number of hazard lights.\
__ <hr>\
__ if <b>type</b> is '<b>control_tower</b>', then arguments are:\
__range visible range, in meters.\
__length length, in meters.\
__width width, in meters.\
__height height, in feet.\
__walls_texture name of the walls texture.\
__roof_texture name of the roof texture.\
__ <hr>\
__ if <b>type</b> is '<b>building</b>', then arguments are:\
__range visible range, in meters.\
__length length, in meters.\
__width width, in meters.\
__height height, in feet.\
__walls_texture name of the walls texture to use when it is day.\
__walls_texture_night name of the walls texture to use when it is night.\
__roof_texture name of the roof texture.\
__CONTEXT\
__mis scn\
__EXAMPLES\
__# Radio tower\
__create_premodeled radio_tower 25000  350    4\
__translation -550.0 650.0 0.0\
__rotate 0 0 0\
__<br>\
__# LAX control tower\
__create_premodeled control_tower 6000 20 20 150 building04_tex wall01_tex\
__# Control tower position\
__translation -3619 35679 0\
__<br>\
__# Building near LAX\
__create_premodeled building 4000 240 140 80 building01_tex building01_night_tex wall01_tex\
__# Building position\
__translation -2650 36900 0\
\
__-----\
\
__NAME\
__create_runway\
__SYNOPSIS\
__create_runway range length width surface dashes edge_light_spacing [ north_label south_label north_displaced_threshold south_displaced_threshold [ flag ] [ ... ] ]\
__DESCRIPTION\
__Creates a runway. Have a look at ##https://en.wikipedia.org/wiki/Runway for more explanations on flags.\
__ARGUMENTS\
__range runway visual range, in meters. Beyond this range, runway wil not be visible (not drawn).\
__length runway length, in meters.\
__width runway width, in meters.\
__surface runway surface type: 0 = paved, 1 = gravel, 2 = concrete, 3 = grooved.\
__dashes runway middle dashes number (0 for none).\
__edge_light_spacing distance between two edge lights (0.0 for none), in meters. //FIXME : seems to have no effect...\
__north_label runway 'north' label.\
__south_label runway 'south' label.\
__north_displaced_threshold  'northern' displaced threshold length (0.0 for none).\
__south_displaced_threshold 'southern' displaced threshold length (0.0 for none).\
__flags: <hr>\
__thresholds add thresholds lines.\
__borders add a border line on each side.\
__td_markers add touch down markers.\
__tdmarkers same as 'td_markers'.\
__midway_markers midway markers.\
__midwaymarkers same as 'midway_markers'.\
__north_gs add a 'northern' glide slope .\
__northgs same as 'north_gs'.\
__south_gs add a 'southern' glide slope.\
__southgs same as 'south_gs'.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Create runway\
__# Runway 36\
__new_runway 20000.0 2720.0 45.0 0 11 100.0 18 36 200.0 0.0 thresholds borders td_markers midway_markers north_gs south_gs\
__runway_approach_lighting_north end tracer align\
__runway_approach_lighting_south end tracer align\
__no_depth_test\
__translation 0.0 0.0 0.0\
__rotate 0 0 0\
\
__-----\
\
__NAME\
__create_smoke\
__SYNOPSIS\
__create_smoke x y z r_st r_max r_rate hide@max respawn_int units color_code\
__DESCRIPTION\
__Creates smoke clouds. At least one object ( see ##create_object ) must be created in mission / scenery file. If not, smoke will not be generated.\
__ARGUMENTS\
__x X offset, relative to 'translate' position, in meters.\
__y Y offset, relative to 'translate' position, in meters.\
__z Z offset, relative to 'translate' position, in meters.\
__r_st start radius, in meters. Cloud radius at beginning.\
__r_max maximum radius, in meters. Cloud radius at end.\
__r_rate radius rate, in meters per second. At which speed the clouds grows. If r_rate is négative, radius rate will be automatically generated.\
__hide@max altitude, in meters, at which one smoke clouds will be hidden when they reach it.\
__respawn_int respawn interval, in milliseconds. Time interval between two clouds generation.\
__units number of clouds.\
__color_code smoke clouds color code. Can be: 0 (light grey/white), 1 (medium grey), 2 (dark/black), and 3 (orange).\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Smoke\
__create_smoke 0.0 0.0 0.0 0.5 2.0 -1.0 0 2000 10 1\
__# Smoke's position\
__translate 30000.0 41020.0 0.0\
\
__-----\
\
__NAME\
__creator\
__SYNOPSIS\
__creator prog_name\
__DESCRIPTION\
__Specifies the program that generated this V3D model file.\
__ARGUMENTS\
__prog_name name of the program which created the file. 80 characters maximum.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__crew\
__SYNOPSIS\
__crew crew passengers passengers_max\
__DESCRIPTION\
__Number of persons aboard.\
__ARGUMENTS\
__crew number of crew members. Must be positive or null.\
__passengers number of passengers. Must be positive or null.\
__passengers_max maximum number of passengers. Must be positive or null.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__desc\
__SEE\
__##description parameter.\
__CONTEXT\
__mis scn 3d\
\
__-----\
\
__NAME\
__description\
__SYNOPSIS\
__description text\
__DESCRIPTION\
__A description text for a mission or an aircraft.\
__ARGUMENTS\
__text description text. Newlines ('\\n'), and escapes ('&lt;bold&gt;', '&lt;underline&gt;', '&lt;default&gt;') are accepted.\
__CONTEXT\
__mis 3d\
__EXAMPLES\
__# In training08.mis file:\
__#\
__# Mission Header\
__#\
__##name Training 8: Mountain Climbers\
__description &lt;bold&gt;Rescue&lt;default&gt; &lt;bold&gt;mock victims&lt;default&gt; posing\\\
__as lost mountain climbers.\\\
__\\n\\\
__&lt;bold&gt;Return&lt;default&gt; to the &lt;underline&gt;helipad&lt;default&gt; at\\\
__&lt;underline&gt;SMO&lt;default&gt; after you have picked up\\\
__the mock victims.\
__<br>\
__# In as350.3d file:\
__##name Aerospatile AS-350\
__desc &lt;bold&gt;Aerospatile AS-350&lt;default&gt;\
__Manufacturer: &lt;bold&gt;Aerospatiale&lt;default&gt;\
__...\
__Number of Pilots: &lt;bold&gt;2&lt;default&gt;\\\
__Number Flight Crew: &lt;bold&gt;2&lt;default&gt;\
\
__-----\
\
__NAME\
__dry_mass\
__SYNOPSIS\
__dry_mass mass\
__DESCRIPTION\
__Dry mass.\
__ARGUMENTS\
__mass mass, in kg.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__elevator_new\
__SEE\
__##---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;--- .\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__end_header\
__SYNOPSIS\
__end_header\
__DESCRIPTION\
__Ends a header block.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_header\
__...\
__end_header\
\
__-----\
\
__NAME\
__end_line_loop\
__SYNOPSIS\
__end_line_loop\
__DESCRIPTION\
__Marks the end bound for a series of vertex statement(s) that describe a line loop.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_line_loop\
__...\
__end_line_loop\
\
__-----\
\
__NAME\
__end_line_strip\
__SYNOPSIS\
__end_line_strip\
__DESCRIPTION\
__Marks the end bound for a series of vertex statement(s) that describe a line strip.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_line_strip\
__...\
__end_line_strip\
\
__-----\
\
__NAME\
__end_lines\
__SYNOPSIS\
__end_lines\
__DESCRIPTION\
__Marks the end bound for a series of vertex statement(s) that describe line segment(s).\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_lines\
__...\
__end_lines\
\
__-----\
\
__NAME\
__end_model\
__SYNOPSIS\
__end_model model_type\
__DESCRIPTION\
__Ends a model block.\
__ARGUMENTS\
__model_type standard\
__ standard_dawn\
__ standard_dusk\
__ standard_far\
__ standard_night\
__ rotor\
__ aileron_left\
__ aileron_right\
__ rudder_top\
__ rudder_bottom\
__ elevator\
__ cannard\
__ aileron_elevator_left\
__ aileron_elevator_right\
__ flap\
__ air_brake\
__ landing_gear\
__ door\
__ fueltank\
__ cockpit\
__ shadow\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_model standard\
__...\
__end_model standard\
\
__-----\
\
__NAME\
__end_points\
__SYNOPSIS\
__end_points\
__DESCRIPTION\
__Marks the end bound for a series of vertex statement(s) that describe point(s).\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_points\
__...\
__end_points\
\
__-----\
\
__NAME\
__end_polygon\
__SYNOPSIS\
__end_polygon\
__DESCRIPTION\
__Marks the end bound for a series of vertex statement(s) that describe a polygon.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_polygon\
__...\
__end_polygon\
\
__-----\
\
__NAME\
__end_quad_strip\
__SYNOPSIS\
__end_quad_strip\
__DESCRIPTION\
__Marks the end bound for a series of vertex statement(s) that describe a quad strip.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_quad_strip\
__...\
__end_quad_strip\
\
__-----\
\
__NAME\
__end_quads\
__SYNOPSIS\
__end_quads\
__DESCRIPTION\
__Marks the end bound for a series of vertex statement(s) that describe quad(s).\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_quads\
__...\
__end_quads\
\
__-----\
\
__NAME\
__end_triangle_fan\
__SYNOPSIS\
__end_triangle_fan\
__DESCRIPTION\
__Marks the end bound for a series of vertex statement(s) that describe a triangle fan.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_triangle_fan\
__...\
__end_triangle_fan\
\
__-----\
\
__NAME\
__end_triangle_strip\
__SYNOPSIS\
__end_triangle_strip\
__DESCRIPTION\
__Marks the end bound for a series of vertex statement(s) that describe a triangle strip.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_triangle_strip\
__...\
__end_triangle_strip\
\
__-----\
\
__NAME\
__end_triangles\
__SYNOPSIS\
__end_triangles\
__DESCRIPTION\
__Ends a triangles section.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_triangles\
__...\
__end_triangles\
\
__-----\
\
__NAME\
__engine\
__SYNOPSIS\
__engine can_pitch? initial_pitch power collective_range\
__DESCRIPTION\
__Engine.\
__ARGUMENTS\
__can_pitch? can be: y (yes), or n (no). Set it to 'y' for aicrafts (helicopters or airplanes) only.\
__initial_pitch initial pitch angle. Mandatory, even if can_pitch? is 'n' (in this case, set initial_pitch to 0).\
__power engine power (in kg * m / s^2).\
__collective_range collective range coefficient (from 0.0 to 1.0)\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__engine_state\
__SYNOPSIS\
__engine_state state\
__DESCRIPTION\
__//FIXME . Not used at now. Found in sarfioopen.c > SARParmLoadFromFileIterate(...) and sarfiosave.c > SARParmSaveToFileAnyIterate(...).\
__ARGUMENTS\
__state can be: 0 (engine off), 1 (engine init), 2 (engine on).\
__CONTEXT\
__scn\
__EXAMPLE\
__# //FIXME\
__//FIXME\
\
__-----\
\
__NAME\
__flap_new\
__SEE\
__##---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;--- .\
__CONTEXT\
__3d\
\
__-----\
\
__NAMES\
__---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;---\
__aileron_left_new\
__aileron_right_new\
__rudder_top_new\
__rudder_bottom_new\
__elevator_new\
__cannard_new\
__aileron_elevator_left_new\
__aileron_elevator_right_new\
__flap_new\
__SYNOPSIS\
__aileron_left_new x y z heading pitch bank t_min t_max\
__aileron_right_new x y z heading pitch bank t_min t_max\
__rudder_top_new x y z heading pitch bank t_min t_max\
__rudder_bottom_new x y z heading pitch bank t_min t_max\
__elevator_new x y z heading pitch bank t_min t_max\
__cannard_new x y z heading pitch bank t_min t_max\
__aileron_elevator_left_new x y z heading pitch bank t_min t_max\
__aileron_elevator_right_new x y z heading pitch bank t_min t_max\
__flap_new x y z heading pitch bank t_min t_max\
__DESCRIPTION\
__Creates a new flight control surface (see above names list).\
__ARGUMENTS\
__x flight control surface X position.\
__y flight control surface Y position.\
__z flight control surface Z position.\
__heading flight control surface heading direction.\
__pitch flight control surface pitch direction.\
__bank flight control surface bank direction.\
__t_min flight control surface minimum 'toe' (stop) angle, in degrees.\
__t_max flight control surface maximum 'toe' (stop) angle, in degrees.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__fuel (*.3d)\
__SYNOPSIS\
__fuel consumption_rate initial max\
__DESCRIPTION\
__Defines the fuel capacity and efficiency for the aircraft.\
__ARGUMENTS\
__consumption_rate consumption rate, in kg/sec. Must be positive or null.\
__initial initial fuel quantity, in kg. Must be positive or null, and equal or lower than the 'max' value.\
__max max fuel quantity, in kg. Must be positive or null.\
__CONTEXT\
__3d\
__EXAMPLE\
__# Fuel capacity and efficiency. For the consumption_rate we just use the equation\
__# (fuel_max_kg / max_endurance_time_in_seconds / avg_throttle_coeff),\
__# where avg_throttle_coeff is the average throttle value (ie 0.8 for most cases).\
__fuel 0.069 581.0 581.0\
\
__-----\
\
__NAME\
__fuel (*.scn)\
__SYNOPSIS\
__fuel current max\
__DESCRIPTION\
__//FIXME: Defines the current fuel level and max fuel capacity of the helipad / runway refueling station ? Found in sarfioopen.c and sceneio.c .\
__ARGUMENTS\
__current current fuel quantity, in kg. Must be positive or null, and lower or equal to max_fuel.\
__max max fuel quantity, in kg. //FIXME: Can be positive or null, or negative. Negative to indicate illimited quantity ?\
__CONTEXT\
__scn\
__EXAMPLE\
__//FIXME Never used at now.\
\
__-----\
\__NAME\
__fuel_tank_new\
__SYNOPSIS\
__fuel_tank_new x y z radius btc_h dry_mass fuel fuel_max droppable?\
__DESCRIPTION\
__New external fuel tank.\
__ARGUMENTS\
__x fuel tank X position.\
__y fuel tank Y position.\
__z fuel tank Z position.\
__radius fuel tank radius. Must be positive or null.\
__btc_h belly to center height distance.\
__dry_mass fuel tank dry mass. Must be positive or null.\
__fuel quantity of fuel in fuel tank. Must be positive or null.\
__fuel_max maximum quantity of fuel in fuel tank. Must be greater or equal to 'fuel'.\
__droppable? is this fuel tank droppable (0 if No, 1 if Yes) ?\
__CONTEXT\
__3d\
__EXAMPLE\
__# Look for 'fueltank_new' in data/aircrafts/hh60.3d\
\
__-----\
\
__NAME\
__fueltank_new\
__SEE\
__##fuel_tank_new parameter.\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__gear_height\
__SYNOPSIS\
__gear_height height\
__DESCRIPTION\
__Gear height.\
__ARGUMENTS\
__height must be positive or null.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__ground_elevation\
__SYNOPSIS\
__ground_elevation altitude\
__DESCRIPTION\
__Ground elevation\
__ARGUMENTS\
__altitude elevation, in feet. Must be positive or null.\
__CONTEXT\
__3d scn\
__EXAMPLE\
__ground_elevation 1602\
\
__-----\
\
__NAME\
__ground_pitch_offset\
__SYNOPSIS\
__ground_pitch_offset ground_pitch_offset\
__DESCRIPTION\
__Ground pitch offset.\
__ARGUMENTS\
__ground_pitch_offset ground pitch offset, in degrees.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__ground_turning\
__SYNOPSIS\
__ground_turning turn_rad turn_vel_opt turn_vel_max\
__DESCRIPTION\
__Ground turning.\
__ARGUMENTS\
__turn_rad turn radius in meters, distance from farthest non-turnable wheel to turnable wheel. Can be negative. A value of 0 specifies no turning\
__turn_vel_opt optimal ground turning velocity along aircraft's y axis, in miles per hour.\
__turn_vel_max maximum ground turning velocity along aircraft's y axis, in miles per hour.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__heightfield_base_directory\
__SYNOPSIS\
__heightfield_base_directory path\
__DESCRIPTION\
__Specifies heightfield base directory which must be an absolute path (or a path not containing the drive prefix if used on Windows). The program handling this item may choose to ignore this value and impose its own heightfield base directory.\
__ARGUMENTS\
__path path of heightfield base directory.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__heightfield_load\
__SYNOPSIS\
__heightfield_load path hfx hfy hfz x y z heading pitch bank\
__DESCRIPTION\
__Heightfield definition and position. A *.hf file is an 8 bits gray, top left origin, without Run Length Encoding *.tga file.\
__ARGUMENTS\
__path heightfield (*.hf) file path, relative to \"sar2/data/\" directory.\
__hfx heightfield X, in meter.\
__hfy heightfield Y, in meter.\
__hfz heightfield Z (altitude), in meters. Altitude corresponds to altitude which is given to a full white (0b11111111) pixel. A full black (0b00000000) pixel correponds to an altitude of zero meters.\
__x heightfield X translation.\
__y heightfield Y translation.\
__z heightfield Z translation.\
__heading heightfield heading rotation.\
__pitch heightfield pitch rotation.\
__bank heightfield bank rotation.\
__CONTEXT\
__3d\
__EXAMPLE\
__##begin_model standard\
__color 1.0 1.0 1.0 1.0\
__##texture_select desert_mesa01\
__heightfield_load textures/desert/mesa01.hf  11000 6000 1400  0 0 0  0 0 0\
__##end_model standard\
\
__-----\
\
__NAME\
__helicopter_accelresp\
__SEE\
__##helicopter_acceleration_responsiveness parameter.\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__helicopter_acceleration_responsiveness\
__SYNOPSIS\
__helicopter_acceleration_responsiveness i j k\
__DESCRIPTION\
__Helicopter acceleration responsiveness\
__ARGUMENTS\
__i must be positive.\
__j must be positive.\
__k must be positive.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__hitpoints\
__SYNOPSIS\
__hitpoints hitpoints hitpoints_max\
__DESCRIPTION\
__//FIXME Object hit points.\
__ARGUMENTS\
__hitpoints //FIXME . Must be equal or lower than 'hitpoints_max' value.\
__hitpoints_max //FIXME .\
__CONTEXT\
__scn\
__EXAMPLE\
__//FIXME  Never used at now.\
\
__-----\
__NAME\
__hoist\
__SYNOPSIS\
__hoist x y z rope_max rope_rate capacity radius z_min z_max basket? diver? hook?\
__DESCRIPTION\
__Hoist position and values.\
__ARGUMENTS\
__x hoist X position, in meter.\
__y hoist Y position, in meter.\
__z hoist Z position, in meter.\
__rope_max maximum rope length, in meter. Must be positive or null.\
__rope_rate rope rate (hoisting speed), in meter per second. Must be positive or null.\
__capacity hoist capacity, in kg. Must be positive or null.\
__radius hoist cylendrical contact radius. Must be positive or null.\
__z_min hoist minimum height, in meter. Must be lower than z_max.\
__z_max hoist maximum height, in meter.\
__basket? can this hoist have a basket? (1 for yes, 0 for no).\
__diver? can this hoist have a diver? (1 for yes, 0 for no).\
__hook? can this hoist have a hook? (1 for yes, 0 for no).\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__human_ref\
__SEE\
__##human_reference parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__human_reference\
__SYNOPSIS\
__human_reference object_name direction\
__DESCRIPTION\
__Defines the human displacement.\
__ARGUMENTS\
__object_name name of the object from which one the human runs to or runs away from. Can be: 'player' or other named object.\
__direction: <hr>\
__run_towards human runs towards the named object. If object name is 'player', when aircraft is on ground and it's engines off, human will automatically run to aircraft, then board.\
__run_to same as 'run_towards'.\
__run_away //FIXME: human runs away from the named object. Not yet implemented.</br>See simop.c &nbsp > &nbsp else if(human->flags & SAR_HUMAN_FLAG_RUN_AWAY) &nbsp > &nbsp /* Need to work on this */\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# human displacement: human runs towards the helicopter when it has touched down.\
__human_reference player run_towards\
\
__-----\
\
__NAME\
__landing_gear_new\
__SYNOPSIS\
__landing_gear_new x y z heading pitch bank anim_rate up? fixed? skis? floats?\
__DESCRIPTION\
__New landing gear.\
__ARGUMENTS\
__x landing gear X position.\
__y landing gear Y position.\
__z landing gear Z position.\
__heading landing gear heading direction.\
__pitch landing gear pitch direction.\
__bank landing gear bank direction.\
__anim_rate landing gear animation rate.\
__up? landing gear initially up ?\
__fixed? is landing gear fixed? If not, landing gear is retractable. 1 for yes, 0 for no.\
__skis? has landing gear skis ? 1 for yes, 0 for no.\
__floats? has landing gear floats ? 1 for yes, 0 for no.\
__CONTEXT\
__3d\
__EXAMPLE\
__# see *.3d files in data/aircrafts/ directory\
\
__-----\
\
__NAME\
__light_new\
__SYNOPSIS\
__light_new x y z r g b a radius init_on? type int_on int_off int_on_delay\
__DESCRIPTION\
__New light.\
__ARGUMENTS\
__x X position, in meters.\
__y Y position, in meters.\
__z Z position, in meters.\
__r red value for light color.\
__g green value for light color.\
__b blue value for light color.\
__a alpha (transparancy) value for light color.\
__radius light radius, in pixels. Must be positive. Use 0 for default radius.\
__init_on? if positive, light will be on. If 0, light will be off.\
__type type of light. Can be: 1 for strobe or 2 for spot light. Use 0 for none.\
__&nbsp;&nbsp;int_on in milliseconds. Time on for strobe light. Total time for light to go from 50% to 100% and from 100% to 50% of its radius. Must be a positive or null integer.\
__&nbsp;&nbsp;int_off in milliseconds. Time off for strobe light. Total time for light to go from 50% to 0% and from 0% to 50% of its radius. Must be a positive or null integer.\
__&nbsp;&nbsp;int_on_delay in milliseconds. For strobe light. \"Additional interval to wait before turning light on when timer is reset\". Must be a positive or null integer. //FIXME: seems to have no effect.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__mission_add_intercept\
__SYNOPSIS\
__mission_add_intercept ref x y z radius urgency name\
__mission_add_intercept ref radius urgency name\
__DESCRIPTION\
__Add a new intercept point (waypoint) which will help player in his navigation. A ^ sign wich indicates direction to follow will be added on Head Up Display in cockpit view, and a red line will be drawn between player's position and intercept point in map view.\
__ARGUMENTS\
__ref reference code. Can be: 0 = Standard, (1 = Reserved), 2 = Begin at location, 3 = Arrive at location.\
__&nbsp;&nbsp;&#9507;&#9473;&nbsp;x X position, in meters. Specify only if reference code is '0'.\
__&nbsp;&nbsp;&#9507;&#9473;&nbsp;y Y position, in meters. Specify only if reference code is '0'.\
__&nbsp;&nbsp;&#9495;&#9473;&nbsp;z Z position, in meters. Specify only if reference code is '0'.\
__radius cylendrical size of intercept, in meters. Must be positive. //FIXME: what for ?\
__urgency urgency level, between 0.0 and 1.0 inclusive. //FIXME: what for ? Percentage of ##mission_objective_time_left ??? Seems to not be used in source code...\
__name intercept point name. //FIXME what for ? Never used at now.\
__CONTEXT\
__mis\
__EXAMPLES\
__# In mission01.mis:\
__mission_objective_new arrive\
__...\
__mission_add_intercept 3 40.0 0.5\
__<br>\
__//FIXME: mission_add_intercept 0 -4530 -24900 0 40.0 1.0 My new waypoint\
\
__-----\
\
__NAME\
__mission_begin_at\
__SYNOPSIS\
__mission_begin_at place_name\
__DESCRIPTION\
__Defines helipad / runway on which one the mission starts.\
__ARGUMENTS\
__place_name helipad / runway name, as defined in *.scn file with an ' ##object_name place_name ' parameter. 'place_name' can also be an object defined in mission file -if this object has been defined earlier in same file-, but a ##mission_begin_at_pos parameter must be set before.\
__CONTEXT\
__mis\
__EXAMPLE\
__# Mission begins at los Angeles International \
__mission_begin_at helipad_lax\
\
__-----\
\
__NAME\
__mission_begin_at_pos\
__SYNOPSIS\
__mission_begin_at_pos x y z heading pitch bank\
__DESCRIPTION\
__Defines position at which one the mission starts. Use it when starting position was not named in scenery file. Must be specified before ##mission_begin_at .\
__ARGUMENTS\
__x X position, in meters.\
__y Y position, in meters.\
__z Z position, in meters.\
__heading heading, in degrees (rotation around Z axis).\
__pitch pitch, in degrees (rotation around Y axis).\
__bank bank, in degrees (rotation around X axis).\
__CONTEXT\
__mis\
__EXAMPLES\
__# Mission begins at given position \
__mission_begin_at_pos -85591.0 15180.0 36.0 90 0 0\
__<br>\
__# In 'training06.mis':\
__# Begin at position and object name (position must be specified first\
__# before object name, since we are starting at an arbitary position\
__# instead of at a preset location on the scene file).\
__# x y z heading pitch bank\
__mission_begin_at_pos -85591.0 15180.0 36.0 90 0 0\
__mission_begin_at helipad_uscg378_01\
\
__-----\
\
__NAME\
__mission_objective_arrive_at\
__SYNOPSIS\
__mission_objective_arrive_at object_name\
__DESCRIPTION\
__Defines object name to land on.\
__ARGUMENTS\
__object_name object name, as defined in the *.scn file.\
__CONTEXT\
__mis\
__EXAMPLE\
__# See ##mission_objective_new for mission overview\
__# Arrive at object\
__mission_objective_arrive_at helipad_lax\
\
__-----\
\
__NAME\
__mission_objective_humans_tally\
__SYNOPSIS\
__mission_objective_humans_tally n_total n_rescue\
__DESCRIPTION\
__Total number of humans and initial number of humans that need to be rescued (both values are usually the same).\
__ARGUMENTS\
__n_total total number of humans that need to be rescued.\
__n_rescue initial number of humans that need to be rescued. Must be less or equal to n_total.\
__CONTEXT\
__mis\
__EXAMPLE\
__# See ##mission_objective_new for mission overview\
__# Total number and initial number of humans that need to be rescued : \
__mission_objective_humans_tally 1 1\
\
__-----\
\
__NAME\
__mission_objective_message_fail\
__SYNOPSIS\
__mission_objective_message_fail message\
__DESCRIPTION\
__Defines message display if mission fails.\
__ARGUMENTS\
__message the message you want to be displayed. Use a \'\\\' (backslash) to continue writing on the next line.\
__CONTEXT\
__mis\
__EXAMPLE\
__# See ##mission_objective_new for mission overview\
__# Mission fail message\
__mission_objective_message_fail You failed to \\\
__land safely at the Los Angeles International helipad!\
\
__-----\
\
__NAME\
__mission_objective_message_success\
__SYNOPSIS\
__mission_objective_message_success message\
__DESCRIPTION\
__Defines message to display if mission succeeds.\
__ARGUMENTS\
__message the message you want to be displayed. Use a \'\\\' (backslash) to continue writing on the next line.\
__CONTEXT\
__mis\
__EXAMPLE\
__# See ##mission_objective_new for mission overview\
__# Mission success message\
__mission_objective_message_success \\\
__Great, you made it back with  \\\
__the victim safely!\
\
__-----\
\
__NAME\
__mission_objective_new\
__SYNOPSIS\
__mission_objective_new objective\
__DESCRIPTION\
__Defines mission objective.\
__ARGUMENTS\
__objective: <hr>\
__arrive objective is to reach object specified by ##mission_objective_arrive_at within a (optional) time limit set by ##mission_objective_time_left .\
__pick_up objective is to pick up at least humans who need rescue ( see ##create_human ) within a (optional) time limit set by ##mission_objective_time_left .\
__pick_up_arrive objective is to pick up at least humans who need rescue ( see ##create_human ) within a (optional) time limit set by ##mission_objective_time_left , then reach object specified by ##mission_objective_arrive_at .\
__CONTEXT\
__mis\
__EXAMPLES\
__# Mission first objective: pick up the victim ...\
__mission_objective_new pick_up\
__mission_objective_humans_tally 1 1\
__mission_objective_time_left 0.0\
__mission_objective_message_success You got the victim, time to head home!\
__mission_objective_message_fail You failed to reach the victim safely!\
__<br>\
__# ... then second objective: go back to home\
__mission_objective_new arrive\
__mission_objective_arrive_at helipad_lax\
__mission_objective_time_left 0.0\
__mission_objective_message_success Good job, you arrived at LAX safely!\
__mission_objective_message_fail You failed to arrive at LAX safely!\
__<br>\
__# Mission objective: pick up and go back to home. Note that we have two mission goals, which are defined with *_humans_tally and *_arrive_at, and only one *_time_left time and only one *_message_success/fail.\
__mission_objective_new pick_up_arrive\
__mission_objective_humans_tally 1 1\
__mission_objective_arrive_at helipad_lax\
__mission_objective_time_left 0.0\
__mission_objective_message_success Good job, you got and brought back the victim safely!\
__mission_objective_message_fail You failed to get and bring back the victim safely!\
__-----\
\
__NAME\
__mission_objective_time_left\
__SYNOPSIS\
__mission_objective_time_left time\
__DESCRIPTION\
__Defines time left to complete objective. The ##mission_objective_message_fail message will be displayed if objective is not reached within times.\
__ARGUMENTS\
__time time left to complete objective, in minutes (decimals accepted). Can be null or non-positive to indicate unlimited time.\
__CONTEXT\
__mis\
__EXAMPLE\
__# See ##mission_objective_new for mission overview\
__# Mission objective time left (unlimited time )\
__mission_objective_time_left 0.0\
\
__-----\
\
__NAME\
__mission_scene_file\
__SYNOPSIS\
__mission_scene_file file_name\
__DESCRIPTION\
__Defines scenery path and file name.\
__ARGUMENTS\
__file_name path to and name of scenery file. Path is relative to game /data folder.\
__CONTEXT\
__mis\
__EXAMPLE\
__# Scenery file\
__mission_scene_file scenery/los_angeles.scn\
\
__-----\
\
__NAME\
__model_file\
__SYNOPSIS\
__model_file file_name\
__DESCRIPTION\
__Defines model path and file name.\
__ARGUMENTS\
__file_name path to and name of model file.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Jetski\
__create_object 3\
__model_file vessels/jetski.3d\
__translation 36 -193 0\
__rotate 200 0 0\
\
__-----\
\
__NAME\
__model_flags\
__SYNOPSIS\
__model_flags flag\
__DESCRIPTION\
__//FIXME.\
__ARGUMENTS\
__flag a flag. Can only be: hide.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__name\
__SYNOPSIS\
__name string\
__DESCRIPTION\
__Defines a name for the mission / scenery / model.\
__ARGUMENTS\
__string mission / scenery / model name. Space characters are allowed.\
__CONTEXT\
__mis 3d scn\
__EXAMPLE\
__# Name of mission\
__name Training: Short flight from SMO to LAX\
\
__-----\
\
__NAME\
__new_fire\
__SEE\
__##create_fire parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__new_helipad\
__SEE\
__##create_helipad parameter.\
__CONTEXT\
__scn //FIXME and mission file (*.mis) ?\
\
__-----\
\
__NAME\
__new_human\
__SEE\
__##create_human parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__new_object\
__SEE\
__##create_object parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__new_premodeled\
__SEE\
__##create_premodeled parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__new_runway\
\
__SEE\
__##create_runway parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__new_smoke\
__SEE\
__##create_smoke parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__normal\
__SYNOPSIS\
__normal i j k\
__DESCRIPTION\
__Specifies the normal (which may or may not be of unit length). This specification must come before the vertex in question and within the data block of ##begin_points , ##begin_lines , ##begin_line_strip , ##begin_line_loop , ##begin_triangles , ##begin_triangle_strip , ##begin_triangle_fan , ##begin_quads , ##begin_quad_strip , or ##begin_polygon .\
__ARGUMENTS\
__i \
__j \
__k \
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__no_depth_test\
__SYNOPSIS\
__no_depth_test\
__DESCRIPTION\
__//FIXME\
__ARGUMENTS\
__(none)\
__CONTEXT\
__mis scn 3d\
__EXAMPLE\
__# //FIXME\
__no_depth_test\
\
__-----\
\
__NAME\
__obj_map_desc\
__SEE\
__##object_map_description parameter.\
__CONTEXT\
__scn //FIXME and mission file (*.mis) ?\
\
__-----\
\
__NAME\
__obj_map_description\
__SEE\
__##object_map_description parameter.\
__CONTEXT\
__scn //FIXME and mission file (*.mis) ?\
\
__-----\
\
__NAME\
__obj_name\
__SEE\
__##object_name parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__object_map_description\
__SYNOPSIS\
__object_map_description description\
__DESCRIPTION\
__Defines how object is described on briefing's map when mouse cursor points on it.\
__ARGUMENTS\
__description object description. Space characters are allowed.\
__CONTEXT\
__scn //FIXME and mission file (*.mis) ?\
__EXAMPLE\
__# Los Angeles International (LAX)\
__create_helipad default 40.0 40.0 0.0 LAX y y y y y\
__no_depth_test\
__object_name helipad_lax\
__object_map_description Helipad - Los Angeles International\
__range 4000\
__translation -5430 35750 0\
\
__-----\
\
__NAME\
__object_name\
__SYNOPSIS\
__object_name name\
__DESCRIPTION\
__Defines object name. This name can be used later as argument for some parameters (for example for a ##mission_begin_at parameter).\
__ARGUMENTS\
__name name_of_the_object (without spaces!).\
__CONTEXT\
__mis scn\
__EXAMPLE\
__... if in scenery file there is:\
__# Los Angeles International (LAX)\
__create_helipad default 40.0 40.0 0.0 LAX y y y y y\
__no_depth_test\
__object_name helipad_lax\
__object_map_description Helipad - Los Angeles International\
__range 4000\
__translation -5430 35750 0\
__... then, in mission file you can write:\
__# Arrive at object (name of object from scene file)\
__mission_objective_arrive_at helipad_lax\
\
__-----\
\
__NAME\
__offset_polygons\
__SYNOPSIS\
__offset_polygons\
__DESCRIPTION\
__Offset Polygons FIXME ##https://en.wikipedia.org/wiki/Z-fighting\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__passengers\
__SYNOPSIS\
__passengers passengers passengers_max\
__DESCRIPTION\
__//FIXME . This parameter is not used at now.\
__ARGUMENTS\
__passengers must be equal or lower than 'passengers_max' value.\
__passengers_max maximum number of passengers in the aircraft.\
__CONTEXT\
__scn\
__EXAMPLE\
__# //FIXME\
__//FIXME\
\
__-----\
\
__NAME\
__player_model_file\
__SYNOPSIS\
__player_model_file name_of_model_file\
__DESCRIPTION\
__Create player's aircraft (helicopter).\
__ARGUMENTS\
__name_of_model_file path to and name of player's aircraft file.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Create player object\
__player_model_file aircrafts/hh65.3d\
\
__-----\
\
__NAME\
__polygon_offset\
__SYNOPSIS\
__polygon_offset [ flag ] [ flag ]\
__DESCRIPTION\
__//FIXME ##https://en.wikipedia.org/wiki/Z-fighting\
__ARGUMENTS\
__flags: <hr>\
__reverse reverse offset.\
__rev same as 'reverse'.\
__write_depth write depth after polygon offsetting.\
__writedepth same as 'write_depth'.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Los Angeles base plots, lowest base layers\
__create_object 6\
__# polygon_offset reverse write_depth\
__model_file objects/los_angeles/plot01.3d\
__translation -67500 67500 0\
\
__-----\
\
__NAME\
__propellar_blur_color\
__SEE\
__##rotor_blur_color parameter.\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__propellar_new\
__SEE\
__##rotor_new parameter.\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__range\
__SYNOPSIS\
__range value\
__DESCRIPTION\
__Defines the visual range. All objects further of the camera are not visible (not drawn).\
__ARGUMENTS\
__value visual range, in meters. Visual range is spherical. Must be positive or null.\
__CONTEXT\
__scn 3d\
__EXAMPLE\
__# Visual range\
__range 20000\
\
__-----\
\
__NAME\
__range_far\
__SYNOPSIS\
__range_far value\
__DESCRIPTION\
__Defines the visual range for displaying the far visual model when the camera is beyond this range.\
__ARGUMENTS\
__value visual range, in meters. Visual range is spherical. Must be positive or null.\
__CONTEXT\
__scn 3d\
__EXAMPLE\
__# Far model display range\
__range_far 2500\
\
__-----\
\
__NAME\
__reg_loc\
__SEE\
__##register_location parameter.\
__CONTEXT\
__scn\
\
__-----\
\
__NAME\
__reg_location\
__SEE\
__##register_location parameter.\
__CONTEXT\
__scn\
\
__-----\
\
__NAME\
__register_location\
__SYNOPSIS\
__register_location x y z heading pitch bank name\
__DESCRIPTION\
__Defines the registred locations used in free flight.\
__ARGUMENTS\
__x X offset, in meters.\
__y Y offset, in meters.\
__z Z offset (altitude), in feet.\
__heading heading, in degrees. Clockwise rotation around the Z (vertical) axis.\
__pitch pitch, in degrees. Rotation around the Y axis.\
__bank bank, in degrees. Rotation around the X axis.\
__name name of the location.\
__CONTEXT\
__scn\
__EXAMPLE\
__# Registered locations\
__#\
__# Only used in Free Flight starting location selection (missions use\
__# object names to match beginning location).\
__#\
__register_location -4931 -25183 0 48 0 0 Avalon/Catalina\
__register_location -5430 35750 0 270 0 0 Los Angeles International\
\
__-----\
\
__NAME\
__rescue_door_new\
__SYNOPSIS\
__rescue_door_new x_closed y_closed z_closed x_opened y_opened z_opened h_opened p_opened b_opened x_thres y_thres z_thres anim_rate opened?\
__DESCRIPTION\
__New rescue door.\
__ARGUMENTS\
__x_closed door closed X position.\
__y_closed door closed Y position.\
__z_closed door closed Z position.\
__x_opened door opened X position.\
__y_opened door opened Y position.\
__z_opened door opened Z position.\
__x_thres threshold X value.\
__y_thres threshold Y value.\
__z_thres threshold Z value.\
__anim_rate door animation rate.\
__opened? is this door opened by default at model generation? Can be y (yes) or n (no).\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__rotate\
__SYNOPSIS\
__rotate heading pitch bank\
__DESCRIPTION\
__Rotates earlier positionned object.\
__ARGUMENTS\
__heading heading, in degrees. Clockwise rotation around Z (vertical) axis.\
__pitch pitch, in degrees. Rotation around Y axis.\
__bank bank, in degrees. Rotation around X axis.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Rotates firetruck 'to the left'\
__rotate -90.0 0.0 0.0\
\
__-----\
\
__NAME\
__rotate&nbsp;(*.3d)\
__SYNOPSIS\
__rotate dh dp db\
__DESCRIPTION\
__Specifies to push a matrix and rotate all primitives by the given values that exist after this primitive but before the next ##unrotate primitive (if any). Rotation matrixes are always returned to level 0 at each ##end_model .\
__FIXME: seems to have no effect whatever the model. In an aircraft model, this primitive seems to be superseded by the ##---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;--- heading, pitch and bank values.\
__ARGUMENTS\
__dh heading offset, in degrees. Clockwise rotation around Z (vertical) axis.\
__dp pitch offset, in degrees. Rotation around Y axis.\
__db bank offset, in degrees. Rotation around X axis.\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_model standard\
__... (some primitives)\
__rotate 3.2 4.1 0.0\
__... (some primitives)\
__##unrotate\
__... (some primitives)\
__end_model standard\
\
__-----\
\
__NAME\
__rotor_blade_blur_texture\
__SYNOPSIS\
__rotor_blade_blur_texture name\
__DESCRIPTION\
__Rotor blade blur texture.\
__ARGUMENTS\
__name texture name\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__rotor_blur_color\
__SYNOPSIS\
__rotor_blur_color r g b a\
__DESCRIPTION\
__Rotor blur color. Only if blur_criteria ( see ##rotor_new ) is not set to 'never'.\
__ARGUMENTS\
__r red value of light color.\
__g green value of light color.\
__b blue value of light color.\
__a alpha (transparancy) value of light color.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__rotor_new\
__SYNOPSIS\
__rotor_new nblades x y z heading pitch bank radius has_prop_wash? follow_controls? blur_criteria can_pitch? no_pitch_landed? no_rotate? blades_offset\
__DESCRIPTION\
__New rotor.\
__ARGUMENTS\
__nblades number of blades\
__x X position of rotor.\
__y Y position of rotor.\
__z Z position of rotor.\
__heading heading of rotor.\
__pitch pitch of rotor.\
__bank bank of rotor.\
__radius radius of rotor blades. Must be positive or null.\
__has_prop_wash? has a rotor wash? 0 if no, 1 if yes.\
__follow_controls? follow controls for pitch and bank? 0 if no, 1 if yes.\
__blur_criteria can be: 0 (never blur), 1 (blur when spinning fast), 2 (blur always).\
__can_pitch? can pitch? 0 if no, 1 if yes.\
__no_pitch_landed? no pitching of rotors when landed? 0 if no, 1 if yes.\
__no_rotate? no rotate?\
__blades_offset blades offset\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__rudder_bottom_new\
__SEE\
__##---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;--- list.\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__rudder_top_new\
__SEE\
__##---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;--- list.\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__runway_approach_lighting_north\
__SYNOPSIS\
__runway_approach_lighting_north flag [ flag ] [ ... ]\
__DESCRIPTION\
__Defines the 'northern' Approach Lighting System of the earlier created runway. See ##create_runway ( ##new_runway ) parameter.\
__ARGUMENTS\
__flags: <hr>\
__end the 'northern' Approach Lighting System of the runway has End Identification Lights.\
__tracer the 'northern' ALS of the runway has 'tracer shells'.\
__align the 'northern' ALS of the runway has Alignment Indicator Lights.\
__ils_glide the 'northern' ALS of the runway has an 'ILS glide mark lighting'.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Runway 36\
__new_runway 20000.0 2720.0 45.0 0 11 100.0 18 36 200.0 0.0 thresholds borders td_markers midway_markers north_gs south_gs\
__runway_approach_lighting_north end tracer align\
__runway_approach_lighting_south end tracer align\
__no_depth_test\
__translation 0.0 0.0 0.0\
__rotate 0 0 0\
\
__-----\
\
__NAME\
__runway_approach_lighting_south\
__SYNOPSIS\
__runway_approach_lighting_south flag [ flag ] [ ... ]\
__DESCRIPTION\
__Defines the 'southern' Approach Lighting System of the earlier created runway. See ##create_runway ( ##new_runway ) parameter.\
__ARGUMENTS\
__flags: <hr>\
__end the 'southern' Approach Lighting System of the runway has End Identification Lights.\
__tracer the 'southern' ALS of the runway has 'tracer shells'.\
__align the 'southern' ALS of the runway has Alignment Indicator Lights.\
__ils_glide the 'southern' ALS of the runway has an 'ILS glide mark lighting'.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Runway 36\
__new_runway 20000.0 2720.0 45.0 0 11 100.0 18 36 200.0 0.0 thresholds borders td_markers midway_markers north_gs south_gs\
__runway_approach_lighting_north end tracer align\
__runway_approach_lighting_south end tracer align\
__no_depth_test\
__translation 0.0 0.0 0.0\
__rotate 0 0 0\
\
__-----\
\
__NAME\
__scene_cant\
__SYNOPSIS\
__scene_cant angle\
__DESCRIPTION\
__Defines the position of the magnetic north.\
__ARGUMENTS\
__angle angle to apply to true north, in degrees.\
__CONTEXT\
__scn\
__EXAMPLE\
__# Cant (magnetic) angle applied to true north.\
__# Example: if geographical direction shown on map view (press 'm') is 309 degrees, magnetic heading shown in cockpit view (press 'F2') will be 309 - scene_cant = 304 degrees.\
__scene_cant 5.0\
\
__-----\
\
__NAME\
__scene_elevation\
__SYNOPSIS\
__scene_elevation elevation\
__DESCRIPTION\
__Defines the elevation of the scene.\
__ARGUMENTS\
__elevation elevation for the scene from sea level, in feet.\
__CONTEXT\
__scn\
__EXAMPLE\
__# Elevation from sea level in feet.\
__scene_elevation 0.0\
\
__-----\
\
__NAME\
__scene_global_position\
__SYNOPSIS\
__scene_global_position x_offset y_offset planet_radius\
__DESCRIPTION\
__Defines position of the center of the scene on the planet.\
__ARGUMENTS\
__x_offset center offset longitude, in degrees.\
__y_offset center offset latitude, in degrees.\
__planet_radius the mean radius of the planet, in meters (usual value: 6371000.0).\
__CONTEXT\
__scn\
__EXAMPLE\
__# Position of the center of the scene on the planet.\
__scene_global_position -118.366 33.63 6371000.0\
\
__-----\
\
__NAME\
__scene_gps\
__SEE\
__##scene_global_position parameter.\
__CONTEXT\
__scn\
\
__-----\
\
__NAME\
__scene_ground_flags\
__SYNOPSIS\
__scene_ground_flags type\
__DESCRIPTION\
__Defines the ground base type.\
__ARGUMENTS\
__type type of ground base. Can be: 'is_water' (or 'iswater').\
__CONTEXT\
__scn\
__EXAMPLE\
__# Ground base\
__scene_ground_flags is_water\
\
__-----\
\
__NAME\
__scene_ground_tile\
__SYNOPSIS\
__scene_ground_tile tilew tileh closerng color_r color_g color_b color_a texture\
__DESCRIPTION\
__Defines the tile of the scene's ground.\
__ARGUMENTS\
__tilew tile width, in meters.\
__tileh tile height, in meters.\
__closerng close range, in meters. If texture is farther from player than this range, it will be replaced by the far solid color.\
__color_r red value of the far solid color (0.00=0%, 1.00=100%).\
__color_g green value of the far solid color (0.00=0%, 1.00=100%).\
__color_b blue value of the far solid color (0.00=0%, 1.00=100%).\
__color_a alpha channel of the far solid color (0.00=fully transparent, 1.00=fully opaque).\
__texture name of the texture file.\
__CONTEXT\
__scn\
__EXAMPLE\
__# Ground base tile\
__scene_ground_tile 1000 1000 10000 0.22 0.36 0.53 1.0 water01_tex \
\
__-----\
\
__NAME\
__scene_map\
__SYNOPSIS\
__scene_map width height texture_file\
__DESCRIPTION\
__Defines the texture used for rendering the scene and its dimensions.\
__ARGUMENTS\
__width width of the texture file, in meters.\
__height height of the texture file, in meters.\
__texture_file path to and name of the texture file.\
__CONTEXT\
__scn\
__EXAMPLE\
__# Dimensions and name of the texture map representing entire scene\
__scene_map 270000 270000 textures/los_angeles/scene_map.tex\
\
__-----\
\
__NAME\
__select_object_by_name\
__SYNOPSIS\
__select_object_by_name name\
__DESCRIPTION\
__Do not use (look for case SAR_PARM_SELECT_OBJECT_BY_NAME: in missionio.c).\
__ARGUMENTS\
__name object name.\
__CONTEXT\
__mis\
__EXAMPLE\
__//Don't use\
\
__-----\
\
__NAME\
__service_ceiling\
__SYNOPSIS\
__service_ceiling altitude\
__DESCRIPTION\
__Service ceiling.\
__ARGUMENTS\
__altitude service ceiling altitude, in feet.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__set_human_mesg_enter\
__SEE\
__##set_human_message_enter parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__set_human_message_enter\
__SYNOPSIS\
__set_human_message_enter message\
__DESCRIPTION\
__Defines the message to be displayed when the human comes aboard of the aircraft.\
__ARGUMENTS\
__message the message you want to be displayed. If parameter name is followed by a '\\' (backslash), then the message is written on the next line.\
__CONTEXT\
__mis scn\
__EXAMPLES\
__# Enter message\
__set_human_message_enter Thanks for saving me!\
__<br>\
__# Enter message\
__set_human_message_enter \\\
__Thanks for saving me!\
\
__-----\
\
__NAME\
__set_welcome_message\
__SYNOPSIS\
__set_welcome_message message\
__DESCRIPTION\
__Defines the message to be displayed at mission startup.\
__ARGUMENTS\
__message the message you want to be displayed. If parameter name is followed by a '\\' (backslash), then the message is written on the next line.\
__CONTEXT\
__scn\
__EXAMPLE\
__name Guadarrama\
__set_welcome_message Press 'e' to start the engines\
\
__-----\
\
__NAME\
__shade_model_flat\
__SYNOPSIS\
__shade_model_flat\
__DESCRIPTION\
__Flat shading.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__shade_model_smooth\
__SYNOPSIS\
__shade_model_smooth\
__DESCRIPTION\
__Smooth shading.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__show_far_day_only\
__SYNOPSIS\
__show_far_day_only\
__DESCRIPTION\
__Show far model day only.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__show_night_model_at_dawn\
__SYNOPSIS\
__show_night_model_at_dawn\
__DESCRIPTION\
__Show night model at dawn.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__show_night_model_at_dusk\
__SYNOPSIS\
__show_night_model_at_dusk\
__DESCRIPTION\
__Show night model at dusk.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__sound_source_new\
__SYNOPSIS\
__sound_source_new name range range_far x y z cutoff heading pitch bank sample_rate_limit sndobj_filename [ sndobj_filename_far ]\
__DESCRIPTION\
__New sound source.\
__ARGUMENTS\
__name sound name, with no space character.\
__range //FIXME: range for close sound file use ?\
__range_far //FIXME: range for far sound file use ?\
__x X position, in meters.\
__y Y position, in meters.\
__z Z position, in meters. //FIXME: or in feet ?\
__cutoff //FIXME: cutoff frequency ?\
__heading sound heading angle, in degrees.\
__pitch sound pitch angle, in degrees.\
__bank sound bank angle, in degrees.\
__sample_rate_limit in Hertz. Should be 11025.\
__sndobj_filename path and filename for close sound.\
__sndobj_filename_far path and filename for far sound. Can be omitted.\
__CONTEXT\
__3d\
__EXAMPLES\
__sound_source_new land_belly        1200.0 1000.0  0 0 0  0.0  0 0 0  11025  sounds/land_belly.wav\
__sound_source_new land_belly_inside 1200.0 1000.0  0 0 0  0.0  0 0 0  11025  sounds/land_belly.wav\
\
__-----\
\
__NAME\
__speed\
__SYNOPSIS\
__speed speed_stall speed_max min_drag overspeed_expected overspeed\
__DESCRIPTION\
__Speed.\
__ARGUMENTS\
__speed_stall stall speed, in miles per hour.\
__speed_max maximum speed, in miles per hour. Must be positive.\
__min_drag minimum drag, in miles per hour. Must be positive.\
__overspeed_expected expected overspeed, in miles per hour. Must be lower than overspeed value.\
__overspeed overspeed, in miles per hour.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__temperature\
__SYNOPSIS\
__temperature temperature_day temperature_dusk/dawn temperature_night\
__DESCRIPTION\
__Temperature.\
__ARGUMENTS\
__temperature_day temperature at day, from 0.0 to 1.0\
__temperature_dusk/dawn temperature at dusk and dawn, from 0.0 to 1.0\
__temperature_night temperature at night, from 0.0 to 1.0\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__texcoord\
__SYNOPSIS\
__texcoord i j k\
__DESCRIPTION\
__Specifies the texture coordinates. This specification must come before the vertex in question and within the data block of ##begin_points , ##begin_lines , ##begin_line_strip , ##begin_line_loop , ##begin_triangles , ##begin_triangle_strip , ##begin_triangle_fan , ##begin_quads , ##begin_quad_strip , or ##begin_polygon . This statement is ignored if no texture was selected.\
__ARGUMENTS\
__i \
__j \
__k \
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__tex_coord\
__SEE\
__##texcoord parameter.\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__texture\
__SEE\
__##texcoord parameter.\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__texture_base_directory\
__SYNOPSIS\
__texture_base_directory path\
__DESCRIPTION\
__Specifies the texture base directory which must be an absolute path (or a path not containing the drive prefix if used on Windows). This item must come before any occurance of texture_load. The program handling this item may choose to ignore this value and impose its own texture base directory. 80 characters maximum.\
__//FIXME really usefull ? Only used in 'hanger01.3d' file, and points to /usr/share/games/SearchAndRescue !\
__ARGUMENTS\
__path path to the texture base directory.\
__CONTEXT\
__mis scn 3d\
__EXAMPLE\
__# Texture base directory\
__//FIXME (see DESCRIPTION above) texture_base_directory /usr/share/sar2\
\
__-----\
\
__NAME\
__texture_load\
__SYNOPSIS\
__texture_load name path priority\
__DESCRIPTION\
__Loads the specified texture 'path' with the given 'priority' (from 0.0 to 1.0, where 1.0 is the highest). If the given 'path' is a relative path then the texture_base_directory will be prefixed to it. The 'name' is an arbitary name used to referance this texture which can later be used to match this in a list of textures. Tip: a *.tex file is a *.tga file (top left origin, without Run Length Encoding).\
__ARGUMENTS\
__name reference name for the texture.\
__path path to and name of the texture *.hf file.\
__priority texture priority, for memory management (0.0=lowest priority, 1.0=highest priority).\
__CONTEXT\
__mis scn 3d\
__EXAMPLE\
__# Load texture images and set named references to them:\
__texture_load water01_tex textures/water01.tex 1.0\
__texture_load building01_tex textures/building01.tex 0.7\
\
__-----\
\
__NAME\
__texture_off\
__SYNOPSIS\
__texture_off\
__DESCRIPTION\
__Disables texturing.\
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__texture_orient_xy\
__SYNOPSIS\
__texture_orient_xy x y dx dy\
__DESCRIPTION\
__Orients the texture along the XY plane, all texcoord statements following this will be ignored since the plane orientation will set the texcoords automatically.\
__ARGUMENTS\
__x texture left border X position, in meters.\
__y texture bottom border Y position, in meters.\
__dx texture X delta, in meters. Must be positive. If dx is lower than object size in X direction, texture will we repeated up to fill it.\
__dy texture Y delta, in meters. Must be positive. If dy is lower than object size in Y direction, texture will we repeated up to fill it.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__texture_orient_xz\
__SYNOPSIS\
__texture_orient_xz x z dx dz\
__DESCRIPTION\
__Orients the texture along the XZ plane, all texcoord statements following this will be ignored since the plane orientation will set the texcoords automatically.\
__ARGUMENTS\
__x texture left border X position, in meters.\
__z texture bottom border Z position, in meters.\
__dx texture X delta, in meters. Must be positive. If dx is lower than object size in X direction, texture will we repeated up to fill it.\
__dz texture Z delta, in meters. Must be positive. If dz is lower than object size in Z direction, texture will we repeated up to fill it.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__texture_orient_yz\
__SYNOPSIS\
__texture_orient_yz y z dy dz\
__DESCRIPTION\
__Orients the texture along the YZ plane, all texcoord statements following this will be ignored since the plane orientation will set the texcoords automatically.\
__ARGUMENTS\
__y texture left border Y position, in meters.\
__z texture bottom border Z position, in meters.\
__dy texture left border Y delta, in meters. Must be positive. If dy is lower than object size in Y direction, texture will we repeated up to fill it.\
__dz texture bottom border Z delta, in meters. Must be positive. If dz is lower than object size in Z direction, texture will we repeated up to fill it.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__texture_select\
__SYNOPSIS\
__texture_select name\
__DESCRIPTION\
__Enables texture and selects the one specified by 'name'.\
__ARGUMENTS\
__name texture name.\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__texture_unselect\
__SEE\
__##texture_off parameter.\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__time_of_day\
__SYNOPSIS\
__time_of_day HH(:MM(:SS))&nbsp;|&nbsp;time_of_day\
__DESCRIPTION\
__Set the mission's start time of the day.\
__ARGUMENTS\
__HH:MM:SS Hours, Minutes, Seconds. Seconds (or minutes and seconds) can be omitted.\
__time_of_day time of the day. Can be: 'day' or 'noon' (equivalent to 12:00), 'dawn' or 'morning' (6:30), 'dusk' or 'evening' (17:30), 'night' or 'midnight' (0:00).\
__CONTEXT\
__mis\
__EXAMPLES\
__# Time of day\
__time_of_day 23:59:59\
__<br>\
__# Time of day\
__time_of_day morning\
\
__-----\
\
__NAME\
__translate\
__SYNOPSIS\
__translate x y z\
__DESCRIPTION\
__Set Cartesian position of the earlier created object.\
__ARGUMENTS\
__x X offset, in meters.\
__y Y offset, in meters.\
__z Z offset, in feet.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Human's position\
__translate 30000.0 41020.0 0.0\
\
__-----\
\
__NAME\
__translate&nbsp;(*.3d)\
__SYNOPSIS\
__translate dx dy dz\
__DESCRIPTION\
__Specifies to push a matrix and translate all primitives by the given values that exist after this primitive but before the next ##untranslate primitive (if any). Translation matrixes are always returned to level 0 at each ##end_model .\
__FIXME: seems to have no effect whatever the model. In an aircraft model, this primitive seems to be superseded by the ##---&nbsp;flight&nbsp;control&nbsp;surfaces&nbsp;list&nbsp;--- x, y and z values.\
__ARGUMENTS\
__dx X offset, in meters.\
__dy Y offset, in meters.\
__dz Z offset, in meters.\
__CONTEXT\
__3d\
__EXAMPLE\
__begin_model standard\
__... (some primitives)\
__translate 3.2 4.1 0.0\
__... (some primitives)\
__##untranslate\
__... (some primitives)\
__end_model standard\
\
__-----\
\
__NAME\
__translate_random\
__SYNOPSIS\
__translate_random r_rand z_rand\
__DESCRIPTION\
__Randomize the position of the earlier positionned object.\
__ARGUMENTS\
__r_rand maximum radius offset on XY plane.\
__z_rand maximum altitude for Z offset.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__create_human default need_rescue\
__# Human's position\
__translate 30000.0 41020.0 0.0\
__# 'Adjust randomly' the human's position\
__translate_random 50.0 0.0\
\
__-----\
\
__NAME\
__translation\
__SEE\
__##translate parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
__NAME\
__translation_random\
__SEE\
__##translate_random parameter.\
__CONTEXT\
__mis scn\
\
__-----\
\
\
__NAME\
__type\
__SYNOPSIS\
__type object_type\
__DESCRIPTION\
__***** Only for reference: do not use 'type' when creating new objects! ***** Note: look for '/* Type */' in objio.c file.\
__ARGUMENTS\
__object_type see ##create_object for objects types.\
__CONTEXT\
__3d\
\
__-----\
\
__NAME\
__unrotate\
__SYNOPSIS\
__unrotate\
__DESCRIPTION\
__Undoes a level of rotation from a ##rotate&nbsp;(*.3d) primitive that occured prior. \
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__untranslate\
__SYNOPSIS\
__untranslate\
__DESCRIPTION\
__Undoes a level of translation from a ##translate&nbsp;(*.3d) primitive that occured prior. \
__ARGUMENTS\
__(none)\
__CONTEXT\
__3d\
__EXAMPLE\
__#\
\
__-----\
\
__NAME\
__version\
__SYNOPSIS\
__version major minor [ release ]\
__DESCRIPTION\
__Defines the version of the file format.\
__For a *.3d file, specifies the V3D model file version (release number is always ignored). This number should always match the version of the libv3d that generated this file.\
__For a mission or a scenery file, program will print a warning to console like: \"File format version 2.5.0 is newer than program version 0.8.2.\"\
__ARGUMENTS\
__major major number of the version.\
__minor minor number of the version.\
__release release number of the version.\
__CONTEXT\
__mis scn 3d\
__EXAMPLE\
__# Version number\
__version 0 8 2\
\
__-----\
\
__NAME\
__weather\
__SYNOPSIS\
__weather weather_type\
__DESCRIPTION\
__Defines mission's weather. Must be specified before mission_scene_file.\
__ARGUMENTS\
__weather_type type of weather. Can be: 'Clear', 'Hazy', 'Scattered', 'Cloudy', 'Overcast', 'Foggy', 'Stormy Sparse', 'Stormy Dense'.<br>Note: weather types are defined in 'weather.ini' file.\
__CONTEXT\
__mis scn\
__EXAMPLE\
__# Weather is clear, today\
__weather Clear\
__# Scenery file\
__mission_scene_file ...\
\
__-----\
\
__NAME\
__wind\
__SYNOPSIS\
__wind heading speed [ gusts ]\
__DESCRIPTION\
__Defines wind. Warning: SaR II >= 2.5.0 only! Must be specified after mission_scene_file. Player tip: when landed, you may have to apply wheel brakes...\
__ARGUMENTS\
__heading direction to which one the wind blows to, in degrees.\
__speed wind speed, in knots.\
__gusts wind is gusty.\
__CONTEXT\
__mis\
__EXAMPLE\
__# Scenery file\
__mission_scene_file ...\
__# Blowing to southeast wind, 27 knots, with gusts.\
__wind 135 27 gusts\
__-----\
";

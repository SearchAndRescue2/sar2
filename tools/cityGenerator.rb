#######################################################################
#   This file is part of Search and Rescue II (SaR2).                 #
#                                                                     #
#   SaR2 is free software: you can redistribute it and/or modify      #
#   it under the terms of the GNU General Public License v.2 as       #
#   published by the Free Software Foundation.                        #
#                                                                     #
#   SaR2 is distributed in the hope that it will be useful, but       #
#   WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See          #
#   the GNU General Public License for more details.                  #
#                                                                     #
#   You should have received a copy of the GNU General Public License #
#   along with SaR2.  If not, see <http://www.gnu.org/licenses/>.     #
#######################################################################

#CITY GENERATOR v0.1

GROUND_LEVEL=0
TRANSLATION_BOTTOM_LEFT = [[27160, -36273], [26583, -37545], [24800, -42100]]
TRANSLATION_TOP_RIGHT =   [[27882, -33900], [28427, -33320], [30450, -31457]]

DENSITY = [0.35, 0.15, 0.02]

MIN_RANGE= [4000,3000,3000]
MAX_RANGE= [15000,9000,5000]

TEXTURES_DAY=["building01_tex","building02_tex","building03_tex","building04_tex","building05_tex"]
TEXTURES_DAY_N=TEXTURES_DAY.size

TEXTURES_NIGHT=["building01_night_tex"]
TEXTURES_NIGHT_N=TEXTURES_NIGHT.size

CENTER=0
CITY=1
OUTSKIRTS=2

MIN_STREET_WIDTH = [60, 50, 50]
MAX_STREET_WIDTH = [80, 100, 100]

MIN_HEIGHT = [250, 100, 70]
MAX_HEIGHT = [515, 400, 150]

MIN_WIDTH = [30, 30, 20]
MAX_WIDTH = [90, 60, 50]

MIN_LENGTH = [30, 30, 20]
MAX_LENGTH = [90, 60, 50]

ROTATE = [false, :random, :random]


def boundedRandom lower_lim,upper_lim,area
    low = lower_lim[area]
    high = upper_lim[area]
    width = high - low
    rand(width) + low
end

def inBounds min,max,current
    current >= min and current <= max
end

def whereAmI? pos_x,pos_y
    #center < city < outskirts
    in_center_x = inBounds TRANSLATION_BOTTOM_LEFT[CENTER][0],TRANSLATION_TOP_RIGHT[CENTER][0],pos_x
    in_center_y = inBounds TRANSLATION_BOTTOM_LEFT[CENTER][1],TRANSLATION_TOP_RIGHT[CENTER][1],pos_y
    return CENTER if in_center_x and in_center_y

    in_city_x = inBounds TRANSLATION_BOTTOM_LEFT[CITY][0],TRANSLATION_TOP_RIGHT[CITY][0],pos_x
    in_city_y = inBounds TRANSLATION_BOTTOM_LEFT[CITY][1],TRANSLATION_TOP_RIGHT[CITY][1],pos_y
    return CITY if in_city_x and in_city_y
    
    in_outskirts_x = inBounds TRANSLATION_BOTTOM_LEFT[OUTSKIRTS][0],TRANSLATION_TOP_RIGHT[OUTSKIRTS][0],pos_x
    in_outskirts_y = inBounds TRANSLATION_BOTTOM_LEFT[OUTSKIRTS][1],TRANSLATION_TOP_RIGHT[OUTSKIRTS][1],pos_y
    return OUTSKIRTS if in_outskirts_x and in_outskirts_y
    return nil
end

def build? where
    rand(0) <= DENSITY[where]
end

def getNew what,place
    case what
    when "width" then boundedRandom MIN_WIDTH,MAX_WIDTH,place
    when "height" then boundedRandom MIN_HEIGHT,MAX_HEIGHT,place
    when "length" then boundedRandom MIN_LENGTH,MAX_LENGTH,place
    when "street" then boundedRandom MIN_STREET_WIDTH,MAX_STREET_WIDTH,place
    when "range" then boundedRandom MIN_RANGE,MAX_RANGE,place
    when "tex_day" then TEXTURES_DAY[rand(TEXTURES_DAY_N)]
    when "tex_night" then TEXTURES_NIGHT[rand(TEXTURES_NIGHT_N)]
    end
end

def buildIt range,length,width,height,tex_day,tex_night,trans_x,trans_y,area
    puts "#                 type     range length width height  walls_texture  walls_night_texture  roof_texture"
    puts "create_premodeled building #{range} #{length} #{width} #{height} #{tex_day} #{tex_night} wall01_tex"
    puts "translation #{trans_x} #{trans_y} #{GROUND_LEVEL}"
    if ROTATE[area] == :random
        puts "rotate #{rand 360} 0 0"
    end
    puts
    puts
end

def newQuarter area
    current_x = TRANSLATION_BOTTOM_LEFT[area][0]
    current_y = TRANSLATION_BOTTOM_LEFT[area][1]

    limit_x = TRANSLATION_TOP_RIGHT[area][0]
    limit_y = TRANSLATION_TOP_RIGHT[area][1]

    while current_y < limit_y do
        max_width = 0;
        while current_x < limit_x do
            length = getNew "length",area
            street = getNew "street",area
            if (whereAmI?(current_x,current_y) == area) and build?(area)
                width = getNew "width",area
                height = getNew "height",area
                range = getNew "range",area
                tex_day = getNew "tex_day",area
                tex_night = getNew "tex_night",area
                trans_x = current_x + (length/2)
                trans_y = current_y + (width/2)
                buildIt range,length,width,height,tex_day,tex_night,trans_x,trans_y,area
                max_width = width if width > max_width
            end
            current_x += length + street
        end
        #reset X
        current_x = TRANSLATION_BOTTOM_LEFT[area][0]
        #incrase Y: max width of building in the row + random street width
        current_y += getNew("street",area) + max_width
    end
end

def newCity
    puts "#OUTSKIRTS"
    puts
    newQuarter OUTSKIRTS
    puts "#CITY"
    puts
    newQuarter CITY
    puts "#CENTER"
    puts
    newQuarter CENTER
end


newCity

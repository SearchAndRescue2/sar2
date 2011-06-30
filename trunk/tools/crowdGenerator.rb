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

#CROWD GENERATOR FOR SaRII

#This script generates crowds of people within a square
#It allows to define a maximum number of people to generate
#and a probability

GROUND_LEVEL = 0
TRANSLATION_BOTTOM_LEFT = [ 27355, -35100 ]
TRANSLATION_TOP_RIGHT = [ 27394, -35077 ]

MAX_PEOPLE = 90
DENSITY = 0.5

SPACE=2.2 #meters
MAX_DEVIATION=0.7 #random

#flags init to false
FLAGS={
    :need_rescue => false,
    :alert => false,
    :aware => false,
    :on_streatcher => false,
    :run_towards => false,
}
FLAGS_PROB={
    :need_rescue => 0,
    :alert => 0.4,
    :aware => 0.4,
    :on_streatcher => 0,
    :run_towards => 0,
}

#true,false,:random
ROTATE=:random

ENTER_MSGS=["It hurts!", "Please help!", "...(unconscious)"]



def get_enter_msg
    s = ENTER_MSGS.size
    return ENTER_MSGS[rand(s)] if s > 0
    ""
end

def go? probabilty
    rand(0) < probabilty
end

def placeHuman x,y,flags,rotate=false
    str = "create_human "
    if flags[:on_streatcher]
        str+= "victim_streatcher_assisted "
    else 
        str+= "default "
    end
    str+= "need_rescue " if flags[:need_rescue]
    str+= "alert " if flags[:alert]
    str+= "lying " if flags[:lying] or flags[:on_streatcher]
    str+= "aware " if flags[:aware]
    str+= "on_streatcher" if flags[:on_streatcher]
    
    puts str

    puts "translation #{x} #{y} #{GROUND_LEVEL}"
    puts "set_human_mesg_enter #{get_enter_msg}" if flags[:need_rescue]

    puts "human_reference player run_towards" if flags[:run_towards]
    
    if rotate
        puts "rotate #{rotate} 0 0"
    end

    puts
    puts
end

def new_deviation
    pos = rand(0) < 0.5
    dev = rand(MAX_DEVIATION*100)/100.0
    return dev if pos
    -dev    
end

def newCrowd

    puts "##### CROWD #####"
    
    current_x = TRANSLATION_BOTTOM_LEFT[0]
    current_y = TRANSLATION_BOTTOM_LEFT[1]

    limit_x = TRANSLATION_TOP_RIGHT[0]
    limit_y = TRANSLATION_TOP_RIGHT[1]

    total=0

    while current_y < limit_y do
        while current_x < limit_x do
            if go? DENSITY
                puts "#PERSON ##{total}"
                total += 1
                x = current_x + new_deviation
                y = current_y + new_deviation
                flags=FLAGS.clone
                flags.each do |key,value|
                    flags[key]=true if go? FLAGS_PROB[key]
                end

            rotate = false
            if ROTATE == :random
                rotate = rand 360
            elsif ROTATE
                rotate = ROTATE
            end
            
            placeHuman x,y,flags,rotate
            end

            current_x += SPACE
            break if total == MAX_PEOPLE
        end
        break if total == MAX_PEOPLE
        current_y += SPACE
        current_x = TRANSLATION_BOTTOM_LEFT[0] 
    end
    
    puts "####### END CROWD #######"
end


newCrowd

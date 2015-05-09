Battle0
=======

server
------

$ ./battle0-server -random

load random world


$ ./battle0 world-island-01.bmp

load battle island


client
------

$ ./battle0-client <server-ip>



keys
----

cursor keys: move map

space: finish moves, switch to next player

t: set tank unit
f: fire
m: move unit

+: zoom in
-: zoom out




Battle0 - fields color codes
============================

field    | HEX     | R G B
---------+---------+------------
desert:   d7dc23    215 220 35
dirt:     444520    68 69 32
forrest:  246223    36 98 35
grass:    22dc21    34 220 33
mountain: dad5bb    218 213 187
snow:     ffffff    255 255 255
water:    2f77d4    47 119 212


bases    | HEX     | RGB
---------+---------+------------
red:      ff0000    255 0 0
blue:     0000ff    0 0 255
yellow:   bef606    190 246 6
green:    00ff00    0 255 0


The game world picture must be in BMP format 512 x 512 pixels.
One pixel per field!

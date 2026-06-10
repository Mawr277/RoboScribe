
from software.GCODEgen.svg_to_gcode import convert_svg_to_gcode



input_file = "software/GCODEgen/test/PG.svg" 
output_file = "software/GCODEgen/test/PG1.gcode" 
skala = 1.0
rozdzielczosc = 1.0

convert_svg_to_gcode(input_file, output_file, skala, rozdzielczosc)
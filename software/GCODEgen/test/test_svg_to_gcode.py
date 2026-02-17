
from software.GCODEgen.svg_to_gcode import convert_svg_to_gcode



input_file = "software/GCODEgen/test/output_path_arial.svg" 
output_file = "software/GCODEgen/test/plik7.gcode" 
skala = 2.0
rozdzielczosc = 0.5


convert_svg_to_gcode(
    input_path=input_file, 
    output_path=output_file,
    skala=skala,
    rozdzielczosc=rozdzielczosc 
)

convert_svg_to_gcode(
    input_path="software/GCODEgen/test/plik.svg",
    output_path="software/GCODEgen/test/plik8.gcode",
)

convert_svg_to_gcode(
    input_path="software/GCODEgen/test/PG.svg",
    output_path="software/GCODEgen/test/plik9.gcode",
)

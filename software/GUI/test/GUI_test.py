
from GUI.Robosribe_GUI import launch_app
from TextToSvg.TextToSvg import text_mode_paths
from GCODEgen.svg_to_gcode import convert_svg_to_gcode

if __name__ == "__main__":
    launch_app(text_mode_paths, convert_svg_to_gcode)
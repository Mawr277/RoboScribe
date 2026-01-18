from TextToSvg import text_mode_paths


w = 1000
h = 1000
text = "abcdefghijk"

text_mode_paths(w,h, text, svg_filename="output_path_arial.svg", font_name="Arial.ttf")

text_mode_paths(w,h, text, svg_filename="output_path_times.svg", font_name="Times New Roman.ttf")
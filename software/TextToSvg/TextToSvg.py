# pip install numpy Pillow svgwrite fonttools
from PIL import ImageFont
import svgwrite
from fontTools.ttLib import TTFont
from fontTools.pens.svgPathPen import SVGPathPen
import os

# STAŁE
FONT_SIZE = 100
MIN_FONT_SIZE = 20
COLOR = (255, 255, 255) # Białe wypełnienie
OUTLINE_COLOR = (0, 0, 0) # Czarny obrys
STROKE_WIDTH = 2 
START_X = 50
START_Y = 150

def text_mode_paths(imageW, imageH, text, svg_filename="output.svg", font_name="arial.ttf"):
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_dir = os.path.join(script_dir, "tests")
    font_path = os.path.join(script_dir, font_name)

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    full_output_path = os.path.join(output_dir, svg_filename)

    font_size = FONT_SIZE

    # Sprawdzenie i ładowanie czcionki
    try:
        pil_font = ImageFont.truetype(font_path, FONT_SIZE)
    except IOError:
        print(f"Błąd ładowania czcionki '{font_path}'. Użycie domyślnej 'arial.ttf'.")
        font_path = os.path.join(script_dir, "Arial.ttf")
        pil_font = ImageFont.truetype(font_path, FONT_SIZE)

    # Ładowanie czcionki przez FontTools (do wyciągania krzywych/ścieżek)
    try:
        tt_font = TTFont(font_path)
        glyph_set = tt_font.getGlyphSet()
        cmap = tt_font.getBestCmap() 
        units_per_em = tt_font['head'].unitsPerEm 
    except Exception as e:
        print(f"Nie udało się załadować FontTools: {e}")
        return False

    x = START_X
    y = START_Y
    
    dwg = svgwrite.Drawing(full_output_path, size=(imageW, imageH))

    #dopasowanie rozmiaru
    words = text.split(" ")
  
    while font_size >= MIN_FONT_SIZE:
        pil_font = ImageFont.truetype(font_path, font_size)
        max_word_width = 0
        space_width = pil_font.getbbox(" ")[2] - pil_font.getbbox(" ")[0]

        for word in words:
            word_width = sum(pil_font.getbbox(c)[2]-pil_font.getbbox(c)[0] for c in word) + space_width
            if word_width > max_word_width:
                max_word_width = word_width

        if word_width <= imageW  - 2 * START_X:
            break
        
        font_size -= 2

    # Aktualizacja parametrów
    pil_font = ImageFont.truetype(font_path, font_size)
    line_height = font_size * 1.2
    space_width = pil_font.getbbox(" ")[2] - pil_font.getbbox(" ")[0]
    
    # Obliczanie skali dla ścieżek SVG
    scale_factor = font_size / units_per_em

    for word in words:
        word_width = sum(pil_font.getbbox(c)[2] - pil_font.getbbox(c)[0] for c in word) + space_width

        # Nowa linia, jeżeli się nie mieści
        if x + word_width > imageW - START_X:
            x = START_X
            y += line_height

        # Rysowanie słowa znak po znaku jako PATH
        for char in word + " ":
            #Obliczamy szerokość znaku
            bbox = pil_font.getbbox(char)
            letter_width = bbox[2] - bbox[0] if bbox else space_width 

            # Pobieramy kształt (path) z FontTools
            glyph_name = cmap.get(ord(char))
            
            if glyph_name and char.strip(): # Jeśli znak istnieje i nie jest pusty
                pen = SVGPathPen(glyph_set)
                try:
                    glyph = glyph_set[glyph_name]
                    glyph.draw(pen)
                    path_data = pen.getCommands()

                    grp = dwg.g(transform=f"translate({x}, {y}) scale({scale_factor}, {-scale_factor})")
                    
                    grp.add(dwg.path(
                        d=path_data,
                        fill=svgwrite.rgb(*COLOR),
                        stroke=svgwrite.rgb(*OUTLINE_COLOR),
                        stroke_width=STROKE_WIDTH / scale_factor
                    ))
                    dwg.add(grp)
                    
                except KeyError:
                    print(f"Brak glifu dla znaku: {char}")

            x += letter_width

    try:
        dwg.save()
        print(f"SVG zapisano jako {full_output_path}")
        return True 
    except Exception as e:
        print(f"Nie udało się zapisać pliku SVG: {e}")
        return False 


# pip install numpy Pillow svgwrite fonttools
import numpy as np
from PIL import ImageFont
import svgwrite
from fontTools.ttLib import TTFont
from fontTools.pens.svgPathPen import SVGPathPen
import os

def text_mode_paths(imageW, imageH, text, svg_filename="output.svg", font_name="arial.ttf"):
    
    # Parametry czcionki
    font_size = 100
    min_font_size = 20

    # Sprawdzenie i ładowanie czcionki
    try:
        pil_font = ImageFont.truetype(font_name, font_size)
    except IOError:
        print(f"Błąd ładowania czcionki '{font_name}'. Użycie domyślnej 'arial.ttf'.")
        font_name = 'arial.ttf'
        pil_font = ImageFont.truetype(font_name, font_size)

    # Ładowanie czcionki przez FontTools (do wyciągania krzywych/ścieżek)
    try:
        tt_font = TTFont(font_name)
        glyph_set = tt_font.getGlyphSet()
        cmap = tt_font.getBestCmap() 
        units_per_em = tt_font['head'].unitsPerEm 
    except Exception as e:
        print(f"Nie udało się załadować FontTools: {e}")
        return

    # Pozycja startowa tekstu
    start_x, y = 50, 150
    x = start_x
    
    # Parametry graficzne
    color = (255, 255, 255) # Białe wypełnienie
    outline_color = (0, 0, 0) # Czarny obrys
    stroke_width = 2 


    dwg = svgwrite.Drawing(svg_filename, size=(imageW, imageH))

    #dopasowanie rozmiaru
    words = text.split(" ")
    space_width = pil_font.getbbox(" ")[2] - pil_font.getbbox(" ")[0]

    while font_size >= min_font_size:
        pil_font = ImageFont.truetype(font_name, font_size)
        max_word_width = 0
        space_width = pil_font.getbbox(" ")[2] - pil_font.getbbox(" ")[0]

        for word in words:
            word_width = sum(pil_font.getbbox(c)[2]-pil_font.getbbox(c)[0] for c in word) + space_width
            if word_width > max_word_width:
                max_word_width = word_width

        if word_width <= w - 2 * start_x:
            break
        
        font_size -= 2

    # Aktualizacja parametrów
    pil_font = ImageFont.truetype(font_name, font_size)
    line_height = font_size * 1.2
    space_width = pil_font.getbbox(" ")[2] - pil_font.getbbox(" ")[0]
    
    # Obliczanie skali dla ścieżek SVG
    scale_factor = font_size / units_per_em

    for word in words:
        # Szerokość słowa
        word_width = sum(pil_font.getbbox(c)[2] - pil_font.getbbox(c)[0] for c in word) + space_width

        # Nowa linia, jeżeli się nie mieści
        if x + word_width > w - start_x:
            x = start_x
            y += line_height

        # Rysowanie słowa znak po znaku jako PATH
        for char in word + " ":
            # 1. Obliczamy szerokość znaku w PIL, żeby wiedzieć, gdzie przesunąć kursor
            bbox = pil_font.getbbox(char)
            letter_width = bbox[2] - bbox[0] if bbox else space_width # obsługa spacji

            # 2. Pobieramy kształt (path) z FontTools
            # Spacja zazwyczaj nie ma kształtu, więc ją pomijamy w rysowaniu, ale przesuwamy x
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
                        fill=svgwrite.rgb(*color),
                        stroke=svgwrite.rgb(*outline_color),
                        stroke_width=stroke_width / scale_factor
                    ))
                    dwg.add(grp)
                    
                except KeyError:
                    print(f"Brak glifu dla znaku: {char}")

            x += letter_width

    dwg.save()
    print(f"SVG (jako krzywe) zapisano jako {svg_filename}")


#Uruchomienie
h, w = (1000, 1000)
text_mode_paths(w,h, "aąęść źżół path test", svg_filename="output_path_arial.svg", font_name="arial.ttf")
text_mode_paths(w,h, "aąęść źżół path test", svg_filename="output_path_times.svg", font_name="Times New Roman.ttf")

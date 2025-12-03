#pip install numpy Pillow svgwrite
import numpy as np
import cv2 as cv2
from PIL import Image, ImageDraw, ImageFont
import svgwrite

def text_mode(output_image, text, svg_filename="output.svg", font_name = "arial.ttf"):
    
    # czcionka
    font_size = 100
    min_font_size = 20

    if font_name.lower() == 'times.ttf':
        svg_font_family = 'Times New Roman'
    elif font_name.lower() == 'arial.ttf':
        svg_font_family = 'Arial'
    else:
        #defualt
        font_name = 'arial.ttf'
        svg_font_family = 'Arial'

    try:
        # Ładowanie czcionki po raz pierwszy
        font = ImageFont.truetype(font_name, font_size)
    except IOError:
        print(f"Błąd ładowania czcionki '{font_name}'. Użycie domyślnej 'Arial'.")
        font_name = 'arial.ttf'
        svg_font_family = 'Arial'
        font = ImageFont.truetype(font_name, font_size)
    

    # pozycja startowa tekstu
    start_x, y = 50, 150
    x = start_x
    line_height = font_size * 1.2 
    space_width = font.getbbox(" ")[2] - font.getbbox(" ")[0]

    color = (255, 255, 255)
    outline_color = (0, 0, 0)
    stroke_width = 4

    w, h = output_image.size

    dwg = svgwrite.Drawing(svg_filename, size=(w, h))

    words = text.split(" ")

    while font_size >= min_font_size:
        font = ImageFont.truetype(font_name, font_size)
        max_word_width = 0

        for word in words:
            word_width = sum(font.getbbox(c)[2]-font.getbbox(c)[0] for c in word) + space_width
            if word_width > max_word_width:
                max_word_width = word_width

        if word_width <= w - 2 *start_x:
            break
        
        font_size -= 2

    #aktualizacja parametrow
    font = ImageFont.truetype(font_name, font_size)
    line_height = font_size * 1.2 
    space_width = font.getbbox(" ")[2] - font.getbbox(" ")[0]

    for word in words:
        #szerokosc slowa
        word_width = sum(font.getbbox(c)[2] - font.getbbox(c)[0] for c in word) + space_width

        #nowa linia jezeli sie nie miesci
        if x + word_width > w - start_x:
            x = start_x
            y += line_height

        #zapis w svg
        for char in word + " ":
            bbox = font.getbbox(char)
            letter_width = bbox[2] - bbox[0]

            dwg.add(
                dwg.text(
                    char,
                    insert=(x, y),
                    fill=svgwrite.rgb(*color),
                    stroke=svgwrite.rgb(*outline_color),
                    stroke_width=stroke_width,
                    font_size=font_size,
                    font_family=svg_font_family
                )
            )
            x += letter_width

    dwg.save()
    print(f"SVG zapisano jako {svg_filename}")



h,w = (1000, 1000)
img_text = Image.new('RGB', (w, h), color='white')

text_mode(img_text, "aąęść źżół abcdefghij klmnopqr stuwx yz", svg_filename="output_times.svg", font_name="times.ttf")
text_mode(img_text, "aąęść źżół abcdefghij klmnopqr stuwx yz", svg_filename="output_arial.svg", font_name="arial.ttf")
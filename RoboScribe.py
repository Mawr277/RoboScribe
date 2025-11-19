#pip install numpy Pillow svgwrite
import numpy as np
import cv2 as cv2
from PIL import Image, ImageDraw, ImageFont
import svgwrite

def text_mode(output_image, text,svg_filename="output.svg"):
    #czcionka
    font_size =100

    #pozycja startowa tekstu
    x, y = 50, 100

    # Kolor konturu i jego grubość
    color = (255,255,255)
    outline_color = (0,0,0)
    stroke_width =4

    dwg = svgwrite.Drawing(svg_filename, size=(w, h))

    #dodajemy tekst do SVG
    dwg.add(
        dwg.text(
            text,
            insert=(x, y),
            fill=svgwrite.rgb(*color),
            stroke=svgwrite.rgb(*outline_color),
            stroke_width=stroke_width,
            font_size=font_size,
            font_family="Arial"
        )
    )


    dwg.save()
    print(f"SVG zapisano jako {svg_filename}")



h,w = (2000, 2000)
img_text = Image.new('RGB', (w, h), color='white')

text_mode(img_text, "aąęśćźżółabcdefghijklmnopqrstuwxyz")
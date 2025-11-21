#pip install numpy Pillow svgwrite
import numpy as np
import cv2 as cv2
from PIL import Image, ImageDraw, ImageFont
import svgwrite

def text_mode(output_image, text,svg_filename="output.svg"):
    #czcionka
    font_size =100
    font = ImageFont.truetype("arial.ttf", font_size)

    #pozycja startowa tekstu
    start_x, y = 50, 150
    x = start_x

    # Kolor konturu i jego grubość
    color = (255,255,255)
    outline_color = (0,0,0)
    stroke_width =4

    dwg = svgwrite.Drawing(svg_filename, size=(w, h))

    for char in text:

        # szerokość litery z faktycznej czcionki
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
                font_family="Arial"
            )
        )
        x += letter_width

    dwg.save()
    print(f"SVG zapisano jako {svg_filename}")



h,w = (2000, 2000)
img_text = Image.new('RGB', (w, h), color='white')

text_mode(img_text, "aąęść źżół abcdefghijklmnopqrstuwxyz")
#pip install numpy Pillow svgwrite
import numpy as np
import cv2 as cv2
from PIL import Image, ImageDraw, ImageFont

def text_mode(output_image, text):
    #przygotowanie obrazu
    draw = ImageDraw.Draw(output_image)
    w,h = output_image.size
    
    #czcionka
    font = ImageFont.truetype("arial.ttf", size=100)

    #pozycja startowa tekstu
    x, y = 50, 100

    # Kolor konturu i jego grubość
    color = (255,255,255)
    outline_color = (0,0,0)
    stroke_width =4
    draw.text((x, y), text, font=font, fill=color,stroke_width=stroke_width, stroke_fill=outline_color )



h,w = (2000, 2000)
img_text = Image.new('RGB', (w, h), color='white')

text_mode(img_text, "ąęśćźżółabcdefghijklmnopqrstuwxyz")
img_text.show()

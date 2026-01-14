## @file TextToSvg.py
#  @author Wit Fornalik (wit.fornalik@gmail.com)
#  @brief Moduł do konwersji tekstu na pliki SVG.
#  @details Wykorzystane biblioteki: svgwrite, fontTools, Pillow
#  @version 1.00
#  @date 2026-11-01
#  @copyright Copyright (c) 2026

# pip install numpy Pillow svgwrite fonttools
from PIL import ImageFont
import svgwrite
from fontTools.ttLib import TTFont
from fontTools.pens.svgPathPen import SVGPathPen
import os

# --- STAŁE ---

##  Początkowy rozmiar czcionki
FONT_SIZE = 100

##  Minimalny dopuszczalny rozmiar czcionki podczas skalowania tekstu
MIN_FONT_SIZE = 20

##  Kolor wypełnienia tekstu w formacie RGB (R, G, B).
COLOR = (255, 255, 255) 

##  Kolor konturu tekstu w formacie RGB (R, G, B).
OUTLINE_COLOR = (0, 0, 0)

##  Grubość obrysu 
STROKE_WIDTH = 2 

##  Margines początkowy X
START_X = 50

##  Margines początkowy osi Y
START_Y = 150

def text_mode_paths(imageW, imageH, text, svg_filename="output.svg", font_name="arial.ttf"):
    """!
    @brief Generuje plik SVG zawierający tekst przekonwertowany na krzywe.
    
    @details Funkcja automatycznie dopasowuje rozmiar czcionki, aby tekst zmieścił się
    w podanej szerokości obrazu. Jeśli tekst jest zbyt szeroki, jest zawijany do nowej linii.
    Glify są wyciągane bezpośrednio z pliku czcionki (.ttf) jako ścieżki wektorowe.

    @param imageW       (int) Szerokość docelowego obrazka/obszaru roboczego SVG.
    @param imageH       (int) Wysokość docelowego obrazka/obszaru roboczego SVG.
    @param text         (str) Tekst do wyrenderowania.
    @param svg_filename (str) Nazwa pliku wyjściowego (domyślnie "output.svg").
                              Plik zostanie zapisany w podkatalogu 'tests'.
    @param font_name    (str) Nazwa pliku czcionki (np. "arial.ttf").
    @note nazwa pliku czcionki (np. "arial.ttf") musi znajdować się w tym samym katalogu co skrypt.
    @return (bool) Zwraca True, jeśli plik został pomyślnie wygenerowany i zapisany.
                   Zwraca False w przypadku błędu (np. brak pliku czcionki, błąd zapisu).
    """
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_dir = os.path.join(script_dir, "tests")
    font_path = os.path.join(script_dir, font_name)

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    full_output_path = os.path.join(output_dir, svg_filename)

    font_size = FONT_SIZE

    # Sprawdzenie i ładowanie czcionki do PIL (do obliczeń szerokości)
    try:
        pil_font = ImageFont.truetype(font_path, FONT_SIZE)
    except IOError:
        print(f"Błąd ładowania czcionki '{font_path}'. Użycie domyślnej 'arial.ttf'.")
        # Fallback - upewnij się, że masz arial.ttf w folderze lub wskaż ścieżkę systemową
        font_path = os.path.join(script_dir, "Arial.ttf") 
        try:
            pil_font = ImageFont.truetype(font_path, FONT_SIZE)
        except IOError:
            print("Nie znaleziono nawet czcionki zapasowej.")
            return False

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

    # Algorytm dopasowania rozmiaru czcionki (zmniejszanie aż się zmieści w linii)
    # UWAGA: Ten algorytm sprawdza najdłuższe słowo, nie całą linię.
    words = text.split(" ")
  
    while font_size >= MIN_FONT_SIZE:
        pil_font = ImageFont.truetype(font_path, font_size)
        max_word_width = 0
        space_width = pil_font.getbbox(" ")[2] - pil_font.getbbox(" ")[0]

        for word in words:
            # Oblicz szerokość słowa
            word_width = sum(pil_font.getbbox(c)[2]-pil_font.getbbox(c)[0] for c in word) + space_width
            if word_width > max_word_width:
                max_word_width = word_width

        # Jeśli najdłuższe słowo mieści się w marginesach, przerywamy pętlę
        if max_word_width <= imageW  - 2 * START_X:
            break
        
        font_size -= 2

    # Aktualizacja parametrów po dopasowaniu rozmiaru
    pil_font = ImageFont.truetype(font_path, font_size)
    line_height = font_size * 1.2
    space_width = pil_font.getbbox(" ")[2] - pil_font.getbbox(" ")[0]
    
    # Obliczanie skali dla ścieżek SVG (jednostki fontu -> piksele)
    scale_factor = font_size / units_per_em

    for word in words:
        # Obliczanie szerokości aktualnego słowa
        word_width = sum(pil_font.getbbox(c)[2] - pil_font.getbbox(c)[0] for c in word) + space_width

        # Zawijanie wiersza (Word Wrap)
        if x + word_width > imageW - START_X:
            x = START_X
            y += line_height

        # Rysowanie słowa znak po znaku jako PATH
        for char in word + " ":
            # Obliczamy szerokość znaku przy użyciu PIL
            bbox = pil_font.getbbox(char)
            letter_width = bbox[2] - bbox[0] if bbox else space_width 

            # Pobieramy kształt (path) z FontTools
            glyph_name = cmap.get(ord(char))
            
            if glyph_name and char.strip(): # Jeśli znak istnieje i nie jest pusty (nie jest spacją)
                pen = SVGPathPen(glyph_set)
                try:
                    glyph = glyph_set[glyph_name]
                    glyph.draw(pen)
                    path_data = pen.getCommands()

                    # Grupa transformacji: przesunięcie (x, y) i skalowanie (font units -> px)
                    # -scale_factor dla Y, ponieważ SVG ma oś Y w dół, a fonty w górę
                    grp = dwg.g(transform=f"translate({x}, {y}) scale({scale_factor}, {-scale_factor})")
                    
                    grp.add(dwg.path(
                        d=path_data,
                        fill=svgwrite.rgb(*COLOR),
                        stroke=svgwrite.rgb(*OUTLINE_COLOR),
                        stroke_width=STROKE_WIDTH / scale_factor # Kompensacja skalowania dla grubości linii
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
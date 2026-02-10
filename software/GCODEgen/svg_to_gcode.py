#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@file svg_to_gcode.py
@brief Konwersja SVG do G-code dla robota RoboScribe.

Ten skrypt konwertuje elementy SVG na polecenia G-code,
które mogą być wykorzystane do sterowania robotem RoboScribe.

@author Bartosz Chabros
@version 0.1
@date 2026-01-26
@copyright Copyright (c) 2026
"""

import argparse
from svgelements import SVG, Rect, Circle, Ellipse, Line, SimpleLine, Polyline, Polygon, Text, Group
import numpy as np
import math

# ---- stałe globalne ----
L1 = 70.0   
L2 = 70.0
Rmax = L1 + L2

def probkowanie(path, rozdzielczosc):
    """
    @brief Przetwarza ścieżkę na zbiór punktów z zadaną rozdzielczością
    @param path Obiekt ścieżki SVG do próbkowania
    @param rozdzielczosc Maksymalna odległość między sąsiednimi punktami
    @return Lista krotek (x, y) reprezentujących punkty próbkowania
    """
    punkty = []
    dlugosc_sciezki = path.length()
    if dlugosc_sciezki is None or rozdzielczosc <= 0:
        return []
    #TODO: rozważyć jak ma działać rozdzielczość
    #n = max(1, int(dlugosc_sciezki / rozdzielczosc))
    n = int(dlugosc_sciezki / rozdzielczosc)

    for i in range(n + 1):
        p = path.point(i / n)
        punkty.append((p.real, p.imag))
    return punkty

def przetwarzaj_rect(element):
    """
    @brief Przetwarza element prostokąta
    @details Generuje listę punktów narożników prostokąta w kolejności: lewy-górny → prawy-górny → prawy-dolny → lewy-dolny → zamknięcie
    @param element Obiekt Rect z biblioteki svgelements
    @return Lista kropek (x, y) reprezentujących narożniki prostokąta
    @note TODO: uwzględnić zaokrąglone narożniki w przyszłości
    """
    #TODO:uwzględnić zaokrąglone narożniki w przyszłości
    x = element.x or 0
    y = element.y or 0
    w = element.width or 0
    h = element.height or 0
    
    # Punkty narożników prostokąta: lewy-górny -> prawy-górny -> prawy-dolny -> lewy-dolny -> zamknięcie
    punkty = [
        (x, y),
        (x + w, y),
        (x + w, y + h),
        (x, y + h),
        (x, y)  # zamknięcie
    ]
    return punkty

def przetwarzaj_circle(element, rozdzielczosc):
    """
    @brief Przetwarza element koła
    @details Generuje punkty rozmieszczone równomiernie na obwodzie koła
    @param element Obiekt Circle z biblioteki svgelements
    @param rozdzielczosc Maksymalna odległość między punktami
    @return Lista krotek (x, y) reprezentujących punkty na okręgu
    """
    cx = element.cx or 0
    cy = element.cy or 0
    r = element.r or 0
    
    # Liczba segmentów na podstawie Rozdzielczości
    segments = max(int(2 * math.pi * r / rozdzielczosc), 8) if rozdzielczosc > 0 else 8
    
    punkty = []
    for i in range(segments + 1):
        angle = (i / segments) * 2 * math.pi
        x = cx + r * math.cos(angle)
        y = cy + r * math.sin(angle)
        punkty.append((x, y))
    
    return punkty

def przetwarzaj_ellipse(element, rozdzielczosc):
    """
    @brief Przetwarza element elipsy
    @details Generuje punkty rozmieszczone równomiernie na obwodzie elipsy
    @param element Obiekt Ellipse z biblioteki svgelements
    @param rozdzielczosc Maksymalna odległość między punktami
    @return Lista krotek (x, y) reprezentujących punkty na elipsie
    """
    cx = element.cx or 0
    cy = element.cy or 0
    rx = element.rx or 0
    ry = element.ry or 0
    
    # Liczba segmentów na podstawie średniej Rozdzielczości
    segments = max(int(2 * math.pi * max(rx, ry) / rozdzielczosc), 8) if rozdzielczosc > 0 else 8
    
    punkty = []
    for i in range(segments + 1):
        angle = (i / segments) * 2 * math.pi
        x = cx + rx * math.cos(angle)
        y = cy + ry * math.sin(angle)
        punkty.append((x, y))
    
    return punkty

def przetwarzaj_line(element):
    """
    @brief Przetwarza element linii prostej
    @param element Obiekt Line lub SimpleLine
    @return Lista dwóch krotek (x1, y1) i (x2, y2) reprezentujących końce linii
    """
    x1 = element.x1 or 0
    y1 = element.y1 or 0
    x2 = element.x2 or 0
    y2 = element.y2 or 0
    
    return [(x1, y1), (x2, y2)]
 
def przetwarzaj_polyline(element):
    """
    @brief Przetwarza element polilinii (otwartej)
    @param element Obiekt Polyline z biblioteki svgelements
    @return Lista krotek (x, y) reprezentujących wierzchołki linii
    """
    if hasattr(element, 'points') and element.points:
        return list(element.points)
    return []

def przetwarzaj_polygon(element):
    """
    @brief Przetwarza element wielokąta (zamknięty)
    @details Automatycznie zamyka wielokąt łącząc ostatni punkt z pierwszym
    @param element Obiekt Polygon z biblioteki svgelements
    @return Lista krotek (x, y) reprezentujących wierzchołki wielokąta (zamknięty)
    """
    if hasattr(element, 'points') and element.points:
        punkty = list(element.points)
        if punkty and punkty[0] != punkty[-1]:
            punkty.append(punkty[0])  # zamknięcie
        return punkty
    return []

def bounding_box(punkty):
    xs = [p[0] for p in punkty]
    ys = [p[1] for p in punkty]
    return min(xs), max(xs), min(ys), max(ys)
        
def wyznacz_baze_robota(punkty, margines=20):
    xmin, xmax, ymin, ymax = bounding_box(punkty)
    Bx = (xmin + xmax) / 2
    By = ymin - margines   # ZA elementem
    return Bx, By
        
def do_ukladu_lokalnego(x, y, Bx, By):
    return x - Bx, y - By
        
def kinematyka_odwrotna(x, y):
    """
    @brief Oblicza kinematykę odwrotną dla zadanego punktu (x, y).
    @details Używa konfiguracji z "łokciem w dół" i ogranicza kąt bazowy do lewej półpłaszczyzny.
    @param x Współrzędna x w lokalnym układzie odniesienia robota.
    @param y Współrzędna y w lokalnym układzie odniesienia robota.
    @return Krotka (kąt_bazy, kąt_łokcia) w stopniach lub None, jeśli punkt jest nieosiągalny.
    """
    dist_sq = x**2 + y**2
    
    # Sprawdzenie, czy punkt jest w ogóle osiągalny
    if dist_sq > Rmax**2 or dist_sq < (L1 - L2)**2:
        #print(f"Punkt ({x:.2f}, {y:.2f}) jest poza fizycznym zasięgiem robota.")
        return None
        
    D = (dist_sq - L1**2 - L2**2) / (2 * L1 * L2)

    # Zabezpieczenie przed błędami precyzji zmiennoprzecinkowej
    if not -1 <= D <= 1:
        #print(f"Punkt ({x:.2f}, {y:.2f}) jest poza fizycznym zasięgiem robota (błąd precyzji).")
        return None

    # Obliczenie kąta łokcia (v2) dla konfiguracji "łokieć w dół" (zakres -180 do 0 stopni)
    v2_rad = np.arctan2(-np.sqrt(1 - D**2), D)

    # Obliczenie kąta podstawy (v1)
    v1_rad = np.arctan2(y, x) - np.arctan2(L2 * np.sin(v2_rad), L1 + L2 * np.cos(v2_rad))
    
    v1_deg = np.degrees(v1_rad) % 360  # Normalizacja do zakresu 0-360
    v2_deg = np.degrees(v2_rad)

    # Sprawdzenie limitu dla lewej strony (90 < v1 < 270 stopni)
    if not (90 < v1_deg < 270):
        #print(f"Punkt ({x:.2f}, {y:.2f}): Kąt v1 ({v1_deg:.1f}°) poza zakresem (90, 270).")
        return None

    return v1_deg, v2_deg

class SVGProcessor:
    def __init__(self, rozdzielczosc, skala):
        self.rozdzielczosc = rozdzielczosc
        self.skala = skala
        self.gcode_wyjsciowy = []

    def przetwarzaj_element(self, element):
        punkty = []
        
        if isinstance(element, Group):
            for child in element:
                self.przetwarzaj_element(child)
            return
        elif hasattr(element, "d"):
            punkty = probkowanie(element, self.rozdzielczosc)
        elif isinstance(element, Rect):
            punkty = przetwarzaj_rect(element)
        elif isinstance(element, Circle):
            punkty = przetwarzaj_circle(element, self.rozdzielczosc)
        elif isinstance(element, Ellipse):
            punkty = przetwarzaj_ellipse(element, self.rozdzielczosc)
        elif isinstance(element, (Line, SimpleLine)):
            punkty = przetwarzaj_line(element)
        elif isinstance(element, Polyline):
            punkty = przetwarzaj_polyline(element)
        elif isinstance(element, Polygon):
            punkty = przetwarzaj_polygon(element)
        elif isinstance(element, Text):
            raise ValueError(f"Nieobsługiwany element: {type(element)} - przekonwertuj tekst na ścieżkę w edytorze SVG.")
        else:
            raise ValueError(f"Nieobsługiwany element: {type(element)}")

        if not punkty:
            raise ValueError(f"lista punktów jest pusta dla elementu: {type(element)}")
        
        # Skalowanie punktów
        punkty = [(x * self.skala, y * self.skala) for x, y in punkty]

        Bx, By = wyznacz_baze_robota(punkty)
        
        self.gcode_wyjsciowy.append(f"G00 X{Bx:.2f} Y{By:.2f}")
        
        for i, (x, y) in enumerate(punkty):
            xl, yl = do_ukladu_lokalnego(x, y, Bx, By)
            ik = kinematyka_odwrotna(xl, yl)

            if ik is None:
                continue

            kat_baza, kat_lokcia = ik

            if i == 0:
                self.gcode_wyjsciowy.append("G01 Z0")
                #zasadniczo w standardze G-code nie ma interpolacji osiowej, wiec załozymy
                #że poruszamy sie według interpolacji kołowej
            self.gcode_wyjsciowy.append(f"G03 T{kat_baza:.2f} V{kat_lokcia:.2f}")

        self.gcode_wyjsciowy.append("G01 Z1")

def convert_svg_to_gcode(input_path, output_path, skala=1.0, rozdzielczosc=1.0):
    """
    Główna funkcja konwertująca plik SVG na G-code.
    """
    try:
        svg = SVG.parse(input_path)
    except FileNotFoundError:
        raise FileNotFoundError(f"Błąd: Nie znaleziono pliku wejściowego: {input_path}")
    except Exception as e:
        raise Exception(f"Błąd podczas parsowania pliku SVG: {e}")

    processor = SVGProcessor(rozdzielczosc=rozdzielczosc, skala=skala)
    
    for element in svg.elements():
        processor.przetwarzaj_element(element)

    try:
        with open(output_path, "w") as f:
            f.write("\n".join(processor.gcode_wyjsciowy))
        print(f"Gotowe! Wygenerowano plik {output_path}")
    except IOError as e:
        raise IOError(f"Błąd podczas zapisu do pliku wyjściowego: {e}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Konwertuje plik SVG na G-code dla robota RoboScribe.")
    parser.add_argument("input_path", nargs='?', default="software/GCODEgen/test/test_shapes.svg",
                        help="Ścieżka do wejściowego pliku SVG.")
    parser.add_argument("output_path", nargs='?', default="software/GCODEgen/symulacja/plik7.gcode",
                        help="Ścieżka do wyjściowego pliku G-code.")
    parser.add_argument("--skala", type=float, default=1.0,
                        help="Skala konwersji współrzędnych SVG na jednostki G-code.")
    parser.add_argument("--rozdzielczosc", type=float, default=1.0,
                        help="Rozdzielczość próbkowania - maksymalna odległość między punktami.")
    
    args = parser.parse_args()
    
    convert_svg_to_gcode(args.input_path, args.output_path, args.skala, args.rozdzielczosc)
    
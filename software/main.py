## @file main.py
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
    d = math.sqrt(x*x + y*y)

    if d > Rmax or d < abs(L1 - L2):
        return None

    alpha = math.acos((L1**2 + L2**2 - d**2) / (2*L1*L2))
    theta2 = math.pi - alpha

    beta = math.atan2(y, x)
    gamma = math.acos((L1**2 + d**2 - L2**2) / (2*L1*d))
    theta1 = beta - gamma

    t1 = math.degrees(theta1)
    t2 = math.degrees(theta2)

    if not (0 <= t1 <= 180 and 0 <= t2 <= 180):
        return None

    return t1, t2

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
            print(f"Nieobsługiwany element: {type(element)} - przekonwertuj tekst na ścieżkę w edytorze SVG.")
            return
        else:
            print(f"Nieobsługiwany element: {type(element)}")
            return

        if not punkty:
            print(f"lista punktów jest pusta dla elementu: {type(element)}")
            return
        
        # Skalowanie punktów
        punkty = [(x * self.skala, y * self.skala) for x, y in punkty]

        Bx, By = wyznacz_baze_robota(punkty)
        
        self.gcode_wyjsciowy.append(f"G0 X{Bx:.2f} Y{By:.2f}")
        
        for i, (x, y) in enumerate(punkty):
            xl, yl = do_ukladu_lokalnego(x, y, Bx, By)
            ik = kinematyka_odwrotna(xl, yl)

            if ik is None:
                continue

            t1, t2 = ik

            if i == 0:
                self.gcode_wyjsciowy.append("G0 Z0")

            self.gcode_wyjsciowy.append(f"M1 T1={t1:.2f} T2={t2:.2f}")

        self.gcode_wyjsciowy.append("G0 Z1")

def convert_svg_to_gcode(input_path, output_path, skala=1.0, rozdzielczosc=1.0):
    """
    Główna funkcja konwertująca plik SVG na G-code.
    """
    try:
        svg = SVG.parse(input_path)
    except FileNotFoundError:
        print(f"Błąd: Nie znaleziono pliku wejściowego: {input_path}")
        return
    except Exception as e:
        print(f"Błąd podczas parsowania pliku SVG: {e}")
        return

    processor = SVGProcessor(rozdzielczosc=rozdzielczosc, skala=skala)
    
    for element in svg.elements():
        processor.przetwarzaj_element(element)

    try:
        with open(output_path, "w") as f:
            f.write("\n".join(processor.gcode_wyjsciowy))
        print(f"Gotowe! Wygenerowano plik {output_path}")
    except IOError as e:
        print(f"Błąd podczas zapisu do pliku wyjściowego: {e}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Konwertuje plik SVG na G-code dla robota RoboScribe.")
    parser.add_argument("input_path", nargs='?', default="software/test/output_path_arial.svg",
                        help="Ścieżka do wejściowego pliku SVG.")
    parser.add_argument("output_path", nargs='?', default="software/test/rysunek.gcode",
                        help="Ścieżka do wyjściowego pliku G-code.")
    parser.add_argument("--skala", type=float, default=1.0,
                        help="Skala konwersji współrzędnych SVG na jednostki G-code.")
    parser.add_argument("--rozdzielczosc", type=float, default=1.0,
                        help="Rozdzielczość próbkowania - maksymalna odległość między punktami.")
    
    args = parser.parse_args()
    
    convert_svg_to_gcode(args.input_path, args.output_path, args.skala, args.rozdzielczosc)
    
## @file main.py

from svgelements import SVG, Rect, Circle, Ellipse, Line, SimpleLine, Polyline, Polygon, Text
import numpy as np
import math

# ---- konfiguracja ----
## @brief Skala konwersji współrzędnych SVG na jednostki G-code
skala = 1.0

## @brief Rozdzielczość próbkowania - maksymalna odległość między punktami
Rozdzielczosc = 1 



# ---- wczytanie SVG ----
## @brief Wczytana zawartość pliku SVG
svg = SVG.parse("software\\test\\output_times.svg")

## @brief Lista poleceń G-code do zapisania do pliku
gcode_wyjsciowy = []

def probkowanie(path, rozdzielczosc):
    ## @brief Przetwarza ścieżkę na zbiór punktów z zadaną rozdzielczością
    ## @param path Obiekt ścieżki SVG do próbkowania
    ## @param rozdzielczosc Maksymalna odległość między sąsiednimi punktami
    ## @return Lista krotek (x, y) reprezentujących punkty próbkowania
    punkty = []
    dlugosc_sciezki = path.length()
    #TODO: rozważyć jak ma działać rozdzielczość
    #n = max(1, int(dlugosc_sciezki / rozdzielczosc))
    n = int(dlugosc_sciezki / rozdzielczosc)

    for i in range(n+1):
        p = path.point(i / n)
        punkty.append((p.real, p.imag))
    return punkty


def przetwarzaj_rect(element):
    ## @brief Przetwarza element prostokąta
    ## @details Generuje listę punktów narożników prostokąta w kolejności: lewy-górny → prawy-górny → prawy-dolny → lewy-dolny → zamknięcie
    ## @param element Obiekt Rect z biblioteki svgelements
    ## @return Lista krotek (x, y) reprezentujących narożniki prostokąta
    ## @note TODO: uwzględnić zaokrąglone narożniki w przyszłości
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

def przetwarzaj_circle(element):
    ## @brief Przetwarza element koła
    ## @details Generuje punkty rozmieszczone równomiernie na obwodzie koła
    ## @param element Obiekt Circle z biblioteki svgelements
    ## @return Lista krotek (x, y) reprezentujących punkty na okręgu
    cx = element.cx or 0
    cy = element.cy or 0
    r = element.r or 0
    
    # Liczba segmentów na podstawie Rozdzielczości
    segments = max(int(2 * math.pi * r / Rozdzielczosc), 8)
    
    punkty = []
    for i in range(segments + 1):
        angle = (i / segments) * 2 * math.pi
        x = cx + r * math.cos(angle)
        y = cy + r * math.sin(angle)
        punkty.append((x, y))
    
    return punkty

def przetwarzaj_ellipse(element):
    ## @brief Przetwarza element elipsy
    ## @details Generuje punkty rozmieszczone równomiernie na obwodzie elipsy
    ## @param element Obiekt Ellipse z biblioteki svgelements
    ## @return Lista krotek (x, y) reprezentujących punkty na elipsie
    cx = element.cx or 0
    cy = element.cy or 0
    rx = element.rx or 0
    ry = element.ry or 0
    
    # Liczba segmentów na podstawie średniej Rozdzielczości
    segments = max(int(2 * math.pi * max(rx, ry) / Rozdzielczosc), 8)
    
    punkty = []
    for i in range(segments + 1):
        angle = (i / segments) * 2 * math.pi
        x = cx + rx * math.cos(angle)
        y = cy + ry * math.sin(angle)
        punkty.append((x, y))
    
    return punkty

def przetwarzaj_line(element):
    ## @brief Przetwarza element linii prostej
    ## @param element Obiekt Line lub SimpleLine
    ## @return Lista dwóch krotek (x1, y1) i (x2, y2) reprezentujących końce linii
    x1 = element.x1 or 0
    y1 = element.y1 or 0
    x2 = element.x2 or 0
    y2 = element.y2 or 0
    
    return [(x1, y1), (x2, y2)]

def przetwarzaj_polyline(element):
    ## @brief Przetwarza element polilinii (otwartej)
    ## @param element Obiekt Polyline z biblioteki svgelements
    ## @return Lista krotek (x, y) reprezentujących wierzchołki polilinii
    if hasattr(element, 'points') and element.points:
        return list(element.points)
    return []

def przetwarzaj_polygon(element):
    ## @brief Przetwarza element wielokąta (zamknięty)
    ## @details Automatycznie zamyka wielokąt łącząc ostatni punkt z pierwszym
    ## @param element Obiekt Polygon z biblioteki svgelements
    ## @return Lista krotek (x, y) reprezentujących wierzchołki wielokąta (zamknięty)
    if hasattr(element, 'points') and element.points:
        punkty = list(element.points)
        if punkty and punkty[0] != punkty[-1]:
            punkty.append(punkty[0])  # zamknięcie
        return punkty
    return []

def generuj_gcode_dla_punktow(punkty):
    ## @brief Generuje polecenia G-code dla listy punktów
    ## @details Tworzy sekwencję poleceń: G0 (szybki ruch), G1 (ruch z rysowaniem), G0 Z1 (podniesienie)
    ## @param punkty Lista krotek (x, y) reprezentujących współrzędne do narysowania
    ## @return void (modyfikuje globalną listę gcode_wyjsciowy)
    if not punkty:
        return
    
    # Przejście do pierwszego punktu bez rysowania
    x0, y0 = punkty[0]
    x_val = round(x0 * skala, 2)
    y_val = round(y0 * skala, 2)
    gcode_wyjsciowy.append(f"G0 X{x_val} Y{y_val}")
    
    gcode_wyjsciowy.append('G0 Z0')  # Opuszczenie pisaka
    
    # Rysowanie pozostałych punktów
    for x, y in punkty[1:]:
        x_val = round(x * skala, 2)
        y_val = round(y * skala, 2)
        gcode_wyjsciowy.append(f"G1 X{x_val} Y{y_val}")

for element in svg.elements():
    punkty = []
    
    if hasattr(element, "d"):  # element ma ścieżkę (Path)
        punkty = probkowanie(element, Rozdzielczosc)
    elif isinstance(element, Rect):
        punkty = przetwarzaj_rect(element)
    elif isinstance(element, Circle):
        punkty = przetwarzaj_circle(element)
    elif isinstance(element, Ellipse):
        punkty = przetwarzaj_ellipse(element)
    elif isinstance(element, (Line, SimpleLine)):
        punkty = przetwarzaj_line(element)
    elif isinstance(element, Polyline):
        punkty = przetwarzaj_polyline(element)
    elif isinstance(element, Polygon):
        punkty = przetwarzaj_polygon(element)
    elif isinstance(element, Text):
         print(f"Nieobsługiwany element: {type(element)} - przekonwertuj tekst na ścieżkę w edytorze SVG.")
    else:
        print(f"Nieobsługiwany element: {type(element)}")
        continue
    
    generuj_gcode_dla_punktow(punkty)
    if punkty:
        gcode_wyjsciowy.append("G0 Z1")  # Podniesienie pisaka po zakończeniu kształtu

#TODO: zamiana współrzędnych X Y na kąty dla serwomechanizmów rysujących

# ---- zapis ----
with open("software\\test\\rysunek.gcode", "w") as f:
    f.write("\n".join(gcode_wyjsciowy))

print("Gotowe! Wygenerowano plik rysunek.gcode")

from svgelements import SVG, Rect, Circle, Ellipse, Line, SimpleLine, Polyline, Polygon
import numpy as np
import math

# ---- konfiguracja ----
skala = 1.0
Rozdzielczosc = 1 

# ---- wczytanie SVG ----
svg = SVG.parse("test_shapes.svg")

gcode_wyjsciowy = []

def probkowanie(path, rozdzielczosc):
    """Przetwarza ścieżkę na zbiór punktów z zadaną rozdzielczością."""
    punkty = []
    dlugosc_sciezki = path.length()
    n = int(dlugosc_sciezki / rozdzielczosc)

    for i in range(n+1):
        p = path.point(i / n)
        punkty.append((p.real, p.imag))
    return punkty

def dodaj_punkt_gcode(x, y):
    """Dodaje punkt do G-code z skalowaniem."""
    x_val = round(x * skala, 2)
    y_val = round(y * skala, 2)
    return f"G1 X{x_val} Y{y_val}"

def przetwarzaj_rect(element):
    """Przetwarza element Rect - generuje punkty narożników."""
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
    """Przetwarza element Circle - generuje punkty na okręgu."""
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
    """Przetwarza element Ellipse - generuje punkty na elipsie."""
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
    """Przetwarza element Line/SimpleLine."""
    x1 = element.x1 or 0
    y1 = element.y1 or 0
    x2 = element.x2 or 0
    y2 = element.y2 or 0
    
    return [(x1, y1), (x2, y2)]

def przetwarzaj_polyline(element):
    """Przetwarza element Polyline."""
    if hasattr(element, 'points') and element.points:
        return list(element.points)
    return []

def przetwarzaj_polygon(element):
    """Przetwarza element Polygon - zamknięty polyline."""
    if hasattr(element, 'points') and element.points:
        punkty = list(element.points)
        if punkty and punkty[0] != punkty[-1]:
            punkty.append(punkty[0])  # zamknięcie
        return punkty
    return []

def generuj_gcode_dla_punktow(punkty):
    """Generuje G-code dla listy punktów."""
    if not punkty:
        return
    
    # Przejście do pierwszego punktu bez rysowania
    x0, y0 = punkty[0]
    x_val = round(x0 * skala, 2)
    y_val = round(y0 * skala, 2)
    gcode_wyjsciowy.append(f"G0 X{x_val} Y{y_val}")
    
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
    
    generuj_gcode_dla_punktow(punkty)




# ---- zapis ----
with open("rysunek.gcode", "w") as f:
    f.write("\n".join(gcode_wyjsciowy))

print("Gotowe! Wygenerowano plik rysunek.gcode")

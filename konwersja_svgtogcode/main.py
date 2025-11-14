from svgelements import SVG
import numpy as np

# ---- konfiguracja ----
skala = 1.0
Rozdzielczosc = 1 

# ---- wczytanie SVG ----
svg = SVG.parse("plik.svg")

gcode_wyjsciowy = []

def probkowanie(path, rozdzielczosc):
    punkty = []
    dlugosc_sciezki = path.length()
    n = int(dlugosc_sciezki / rozdzielczosc)

    for i in range(n+1):
        p = path.point(i/ n)
        punkty.append((p.real, p.imag))
    return punkty

for element in svg.elements():
    #TODO dodać obsługę innych elementów SVG Rect,Ellipse,Circle,Line (SimpleLine),Polyline,Polygon
    if hasattr(element, "d"):  # element ma ścieżkę
        pts = probkowanie(element, Rozdzielczosc)
        #TODO przemyśleć czy to ma sens
        # przejście do początku bez rysowania
        if pts:
            x0, y0 = pts[0]
            x_val = round(x0*skala, 2) #zokrąglanie do 2 miejsc po przecinku
            y_val = round(y0*skala, 2)
            gcode_wyjsciowy.append(f"G0 X{ x_val } Y{ y_val }")

        # rysowanie
        for x, y in pts[1:]:
            x_val = round(x*skala, 2)
            y_val = round(y*skala, 2)
            gcode_wyjsciowy.append(f"G1 X{ x_val } Y{ y_val } ")




# ---- zapis ----
with open("rysunek.gcode", "w") as f:
    f.write("\n".join(gcode_wyjsciowy))

print("Gotowe! Wygenerowano plik rysunek.gcode")

import os
import re
import matplotlib.pyplot as plt
import numpy as np

# Konfiguracja parametrów robota
# Znalezienie ścieżki do pliku GCODE względem lokalizacji tego skryptu
script_dir = os.path.dirname(os.path.abspath(__file__))
NAZWA_PLIKU = os.path.join(script_dir, "plik8.gcode")
L1 = 70.0
L2 = 70.0

# Listy na obliczone współrzędne końcówki (efektora)
x_coords = []
y_coords = []

# Wyrażenie regularne do wyciągania wartości T i V z linii (wielkość liter nie ma znaczenia)
pattern = re.compile(r'[tT](\d+)\s+[vV](\d+)')

# Sprawdzenie czy plik istnieje przed próbą otwarcia
if not os.path.exists(NAZWA_PLIKU):
    print(f"Błąd: Plik '{NAZWA_PLIKU}' nie został znaleziony w bieżącym katalogu.")
else:
    # Wczytywanie trajektorii z pliku
    with open(NAZWA_PLIKU, "r", encoding="utf-8") as file:
        for line_num, line in enumerate(file, 1):
            match = pattern.search(line)
            if match:
                # Odczytanie przetłumaczonych kątów bazy (T) i łokcia (V)
                T_human = float(match.group(1))
                V_human = float(match.group(2))
                
                # 1. Konwersja powrotna do fizycznego układu współrzędnych SVG
                v1_deg = 270.0 - T_human
                v2_deg = -V_human
                
                # Zamiana na radiany
                v1_rad = np.radians(v1_deg)
                v2_rad = np.radians(v2_deg)
                
                # 2. Kinematyka prosta (Forward Kinematics) dla układu dwuczłonowego
                x = L1 * np.cos(v1_rad) + L2 * np.cos(v1_rad + v2_rad)
                y = L1 * np.sin(v1_rad) + L2 * np.sin(v1_rad + v2_rad)
                
                x_coords.append(x)
                y_coords.append(y)

    # Rysowanie wykresu, jeśli udało się wczytać jakiekolwiek punkty
    if x_coords:
        plt.figure(figsize=(9, 9))
        
        # Rysowanie pełnej trajektorii robota
        plt.plot(x_coords, y_coords, color='#1f77b4', linestyle='-', linewidth=2, label='Ścieżka robota')
        
        # Oznaczenie punktu startowego i końcowego
        plt.scatter(x_coords[0], y_coords[0], color='#2ca02c', s=120, zorder=5, label=f'Start ({x_coords[0]:.1f}, {y_coords[0]:.1f})')
        plt.scatter(x_coords[-1], y_coords[-1], color='#d62728', s=120, zorder=5, label=f'Koniec ({x_coords[-1]:.1f}, {y_coords[-1]:.1f})')
        
        # Oznaczenie punktu montażu bazy robota (0,0)
        plt.scatter(0, 0, color='black', marker='x', s=150, zorder=6, label='Baza robota (0,0)')

        # Formatowanie układu pod standard SVG (Oś Y skierowana w dół ekranu)
        plt.gca().invert_yaxis() 
        
        # Siatka i linie osi układu współrzędnych
        plt.grid(True, linestyle='--', alpha=0.5)
        plt.axhline(0, color='black', linewidth=0.8, alpha=0.7)
        plt.axvline(0, color='black', linewidth=0.8, alpha=0.7)
        
        # Zachowanie proporcji 1:1, by kształt nie był zniekształcony
        plt.axis('equal')

        plt.title(f'Trajektoria końcówki wygenerowana z pliku: {NAZWA_PLIKU}', fontsize=12, pad=15)
        plt.xlabel('Oś X (w prawo)')
        plt.ylabel('Oś Y (w dół - standard SVG)')
        plt.legend(loc='upper right')
        plt.show()
    else:
        print(f"Przetworzono plik, ale nie znaleziono żadnych linii zawierających parametry T i V.")
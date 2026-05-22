#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
@file better.py
@brief Wizualizacja ruchu robota na podstawie G-code.

Ten skrypt interpretuje G-code wygenerowany dla robota RoboScribe,
oblicza kolejne pozycje jego ramion za pomocą kinematyki prostej
i tworzy animowaną wizualizację ruchu.
"""

import matplotlib.pyplot as plt
import matplotlib.animation as animation
import re
import math


L1 = 70.0
L2 = 70.0


def forward_kinematics(theta1_deg, theta2_deg):
    """
    Oblicza współrzędne (x, y) łokcia i końcówki roboczej na podstawie
    kątów przegubów (kinematyka prosta).

    Args:
        theta1_deg (float): Kąt przegubu bazowego w stopniach.
        theta2_deg (float): Kąt przegubu łokciowego w stopniach.

    Returns:
        tuple: Krotka zawierająca dwie krotki: ((elbow_x, elbow_y), (end_x, end_y)).
    """
    #zamiana kątów z maciejowego układu współrzędnych na SVG 
    theta1_deg = theta1_deg + 90
    theta2_deg = -theta2_deg 
    
    theta1_rad = math.radians(theta1_deg)
    theta2_rad = math.radians(theta2_deg)  # Kąt względny drugiego ramienia

    elbow_x = L1 * math.cos(theta1_rad)
    elbow_y = L1 * math.sin(theta1_rad)

    end_x = elbow_x + L2 * math.cos(theta1_rad + theta2_rad)
    end_y = elbow_y + L2 * math.sin(theta1_rad + theta2_rad)

    return (elbow_x, elbow_y), (end_x, end_y)

def parse_gcode(file_path):
    """
    Analizuje plik G-code i zwraca listę punktów definiujących ruch robota.

    Args:
        file_path (str): Ścieżka do pliku G-code.

    Returns:
        list: Lista słowników, gdzie każdy słownik reprezentuje jeden krok
              i zawiera pozycję bazy, lokalne pozycje ramion oraz status pisaka.
    """
    all_points = []
    pen_is_down = False
    current_base_x, current_base_y = 0, 0

    with open(file_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            if line.startswith('G00'):
                pen_is_down = False
                match = re.search(r'X([\d\.\-]+)\s+Y([\d\.\-]+)', line)
                if match:
                    current_base_x = float(match.group(1))
                    current_base_y = float(match.group(2))
            elif line.startswith('G01 Z0'):
                pen_is_down = True
            elif line.startswith('G01 Z1'):
                pen_is_down = False
            elif line.startswith('G03'):
                match = re.search(r'T([\d\.\-]+)\s+V([\d\.\-]+)', line)
                if match:
                    theta1 = float(match.group(1))
                    theta2 = float(match.group(2))
                    
                    (elbow_local, end_local) = forward_kinematics(theta1, theta2)
                    
                    all_points.append({
                        'base_pos': (current_base_x, current_base_y),
                        'elbow_local': elbow_local,
                        'end_local': end_local,
                        'is_drawing': pen_is_down
                    })
    return all_points

def main():
    """
    Główna funkcja uruchamiająca interpreter G-code i wizualizację.
    """
    gcode_file = 'software/GCODEgen/test/L_output.gcode'
    
    try:
        all_points = parse_gcode(gcode_file)
    except FileNotFoundError:
        print(f"Błąd: Nie znaleziono pliku G-code: '{gcode_file}'")
        print("Upewnij się, że najpierw wygenerowałeś ten plik, uruchamiając poprzednie polecenie.")
        return

    if not all_points:
        print("Nie znaleziono żadnych ruchów rysujących w pliku G-code.")
        return

    fig, ax = plt.subplots(figsize=(12, 10))
    ax.set_aspect('equal')
    ax.set_xlabel("Współrzędna X")
    ax.set_ylabel("Współrzędna Y")
    ax.grid(True)

    # Rysowanie odwiedzonych punktów końcówki narzędzia
    draw_x = []
    draw_y = []
    move_x = []
    move_y = []

    for p in all_points:
        world_x = p['end_local'][0] + p['base_pos'][0]
        world_y = p['end_local'][1] + p['base_pos'][1]
        if p['is_drawing']:
            draw_x.append(world_x)
            draw_y.append(-world_y)
        else:
            move_x.append(world_x)
            move_y.append(-world_y)

    if draw_x:
        ax.scatter(draw_x, draw_y, c='blue', s=12, label='Punkty rysowane', zorder=2)
    if move_x:
        ax.scatter(move_x, move_y, c='gray', s=8, alpha=0.5, label='Ruch bez rysowania', zorder=1)

    # Elementy ramienia robota do animacji
    link1, = ax.plot([], [], 'r-', lw=4, solid_capstyle='round', label='Ramię 1 (L1)')
    link2, = ax.plot([], [], 'g-', lw=4, solid_capstyle='round', label='Ramię 2 (L2)')
    base_joint, = ax.plot([], [], 'ko', markersize=10, label='Baza robota')
    elbow_joint, = ax.plot([], [], 'ro', markersize=6)
    end_effector_joint, = ax.plot([], [], 'go', markersize=6)
    
    
    world_points_x = []
    world_points_y = []
    for p in all_points:
        base_x, base_y = p['base_pos']
        elbow_x = p['elbow_local'][0] + base_x
        elbow_y = p['elbow_local'][1] + base_y
        end_x = p['end_local'][0] + base_x
        end_y = p['end_local'][1] + base_y
        
        world_points_x.extend([base_x, elbow_x, end_x])
        world_points_y.extend([-base_y, -elbow_y, -end_y])  

    if not world_points_x or not world_points_y:
        print("Brak punktów do narysowania.")
        return
        
    margin = 20  # Dodatkowy margines
    ax.set_xlim(min(world_points_x) - margin, max(world_points_x) + margin)
    ax.set_ylim(min(world_points_y) - margin, max(world_points_y) + margin)

    def animate(i):
        """Funkcja aktualizująca klatkę animacji."""
        p = all_points[i]
        base_pos = p['base_pos']
        
        
        base_pos_plot = (base_pos[0], -base_pos[1])
        elbow_pos_plot = (p['elbow_local'][0] + base_pos[0], -(p['elbow_local'][1] + base_pos[1]))
        end_pos_plot = (p['end_local'][0] + base_pos[0], -(p['end_local'][1] + base_pos[1]))

        base_joint.set_data([base_pos_plot[0]], [base_pos_plot[1]])
        link1.set_data([base_pos_plot[0], elbow_pos_plot[0]], [base_pos_plot[1], elbow_pos_plot[1]])
        link2.set_data([elbow_pos_plot[0], end_pos_plot[0]], [elbow_pos_plot[1], end_pos_plot[1]])
        elbow_joint.set_data([elbow_pos_plot[0]], [elbow_pos_plot[1]])
        end_effector_joint.set_data([end_pos_plot[0]], [end_pos_plot[1]])
        
        return link1, link2, base_joint, elbow_joint, end_effector_joint

    # Tworzenie i wyświetlanie animacji
    anim = animation.FuncAnimation(fig, animate, frames=len(all_points),
                                   interval=30, blit=True, repeat=True)

    # # Zapisywanie animacji do pliku .gif
    # try:
    #     print("Zapisywanie animacji do pliku 'roboscribe_animation.gif'...")
    #     # Do zapisu jako GIF wymagana jest biblioteka Pillow.
    #     # Zainstaluj ją, jeśli jej brakuje: pip install Pillow
    #     anim.save('roboscribe_animation.gif', writer='pillow', fps=30, dpi=100)
    #     print("Animacja została pomyślnie zapisana.")
    # except Exception as e:
    #     print("\nBŁĄD: Nie udało się zapisać animacji.")
    #     print(f"Szczegóły błędu: {e}")
    #     print("Upewnij się, że masz zainstalowaną bibliotekę 'Pillow' (pip install Pillow).")

    ax.legend()
    plt.title("Wizualizacja ruchu robota na podstawie G-code")

    # Zmiana etykiet osi Y na wartości dodatnie, aby pasowały do SVG
    formatter = plt.FuncFormatter(lambda val, loc: f'{-val:g}')
    ax.yaxis.set_major_formatter(formatter)

    plt.show()

if __name__ == '__main__':
    main()

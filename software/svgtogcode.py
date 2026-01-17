# run_conversion.py

# 1. Zaimportuj funkcję 'convert_svg_to_gcode' z pliku 'main.py'.
#    Ponieważ oba pliki są w tym samym folderze, wystarczy prosta instrukcja importu.
from main import convert_svg_to_gcode


# 2. Zdefiniuj ścieżki i parametry dla nowego zadania.
#    Możesz użyć dowolnych plików i ustawień.
input_file = "software/test/test.svg"  # Plik wejściowy
output_file = "software/test/wynik_drugiej_konwersji.gcode" # Nazwa dla nowego pliku wyjściowego
skala = 2.0
rozdzielczosc = 0.5

# 3. Wywołaj zaimportowaną funkcję z nowymi parametrami.
convert_svg_to_gcode(
    input_path=input_file, 
    output_path=output_file, 
)

print(f"Zakończono. Wynik zapisano w pliku: {output_file}")

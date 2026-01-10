import sys
import os
import ctypes
from PyQt6.QtWidgets import (
    QApplication,
    QWidget,
    QLabel,
    QLineEdit,
    QPushButton,
    QVBoxLayout,
    QHBoxLayout,
    QComboBox,
)
from PyQt6.QtGui import QIcon
from PyQt6.QtSvgWidgets import QSvgWidget

def add_line_edit(parent_layout, label_text, placeholder=""):
    """
    Tworzy Label + QLineEdit, układa je w rzędzie i dodaje do parent_layout.
    Zwraca obiekt QLineEdit, aby można go było przypisać do zmiennej.
    """
    # 1. Tworzenie elementów
    label = QLabel(label_text)
    line_edit = QLineEdit()
    line_edit.setPlaceholderText(placeholder)
    
    # 2. Układanie ich obok siebie
    row_layout = QHBoxLayout()
    row_layout.addWidget(label)
    row_layout.addWidget(line_edit)
    
    # 3. Dodanie rzędu do głównego pionowego układu
    parent_layout.addLayout(row_layout)
    
    # 4. WAŻNE: Zwracamy line_edit, żebyś mógł go używać w logice
    return line_edit

def add_combo_box(parent_layout, label_text, items):
    label = QLabel(label_text)
    combo = QComboBox()
    for text,data in items:
        combo.addItem(text, data)

    row_layout = QHBoxLayout()
    row_layout.addWidget(label)
    row_layout.addWidget(combo)

    parent_layout.addLayout(row_layout)
    return combo

class MyApp(QWidget):
    def __init__(self, SvgToText):
        super().__init__()

        # funkcje
        self.funkcja_logiki = SvgToText

        self.init_ui()

    def init_ui(self):

        main_layout = QVBoxLayout()
        
        #ikona programu
        folder = os.path.dirname(__file__)
        icon_dir = os.path.join(folder, 'logo_Roboscribe.png')
        app.setWindowIcon(QIcon(icon_dir))

        #pola tekstowe
        self.text_input = add_line_edit(main_layout, "Tekst:", "np: Roboscribe")
        self.w_input = add_line_edit(main_layout, "Szerokość:", "np: 300")
        self.h_input = add_line_edit(main_layout, "Wysokość", "np: 400")
        self.file_name_input = add_line_edit(main_layout, "Nazwa pliku wyjściowego:" , "np: output.svg")

        #font
        lista_czcionek = [
            ("Arial", "arial.ttf"),
            ("Times New Roman", "Times New Roman.ttf"),
        ]
        self.font_input = add_combo_box(main_layout, "Czcionka:", lista_czcionek)

        #przycisk
        self.button = QPushButton("generuj")
        self.button.clicked.connect(self.on_button_click)
        main_layout.addWidget(self.button)



        self.setLayout(main_layout)
        #okno
        self.setWindowTitle("Roboscribe")
        self.setGeometry(300, 300, 400, 200)

    def on_button_click(self):

        # Pobranie danych z pól
        text = self.text_input.text()
        w = self.w_input.text()
        h = self.h_input.text()
        czcionka = self.font_input.currentData()
        raw_name = self.file_name_input.text().strip()

        if not raw_name: 
            raw_name = "output" # Domyślna nazwa jak nic nie wpisano
            
        if not raw_name.lower().endswith(".svg"):
            file_name = raw_name + ".svg"
        else:
            file_name = raw_name

        #placeholder
        # Przykładowa logika
        print("Tekst:", text)
        print("w:", w)
        print("h:", h)
        print("font:", czcionka)
        print("nazwa pliku:", file_name)
        wynik = self.funkcja_logiki(text,w,h, czcionka, file_name)

        # Opcjonalnie: wypisujemy wynik, jeśli logika coś zwróciła
        if wynik:
             print("GUI: Otrzymano wynik z funkcji:", wynik)


# --- Testowanie tego pliku ---
if __name__ == "__main__":
    myappid = 'roboscribe.gui'

    app = QApplication(sys.argv)

    try:
        ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(myappid)
    except ImportError:
        pass # Ignorujemy, jeśli to nie Windows
    
    #placeholder do sprawdzenia czy dziala
    def placeholder_svg_to_text(t, w, h, font_name="arial.ttf", file_name= "output.svg"):
        print(f"--- LOGIKA (TEST): Dostałem {t}, wymiary {w}x{h} ---, font_name {font_name}, file_name = {file_name}")
        return "Gotowy SVG"

    # Przekazujemy tę funkcję do okna
    window = MyApp(placeholder_svg_to_text)
    
    window.show()
    sys.exit(app.exec())
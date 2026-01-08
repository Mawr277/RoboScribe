import sys
from PyQt6.QtWidgets import (
    QApplication,
    QWidget,
    QLabel,
    QLineEdit,
    QPushButton,
    QVBoxLayout,
    QHBoxLayout,
    QComboBox
)

class MyApp(QWidget):
    def __init__(self, SvgToText):
        super().__init__()
        self.funkcja_logiki = SvgToText
        self.init_ui()

    def init_ui(self):
        #labele
        self.text_label = QLabel("Tekst:")
        self.w_label = QLabel("w:")
        self.h_label = QLabel("h:")
        self.font_label = QLabel("czcionka:")
        self.file_name_label = QLabel("nazwa pliku:")

        #pola tekstowe
        self.text_input = QLineEdit()
        self.w_input = QLineEdit()
        self.h_input = QLineEdit()
        self.file_name_input = QLineEdit()

        #wybor czcionki
        self.font_input = QComboBox()
        self.font_input.addItem("Arial", "arial.ttf")
        self.font_input.addItem("Times New Roman", "Times New Roman.ttf")

        #przycisk
        self.button = QPushButton("Kliknij mnie")
        self.button.clicked.connect(self.on_button_click)

        #layout
        main_layout = QVBoxLayout()

        text_layout = QHBoxLayout()
        text_layout.addWidget(self.text_label)
        text_layout.addWidget(self.text_input)

        w_layout = QHBoxLayout()
        w_layout.addWidget(self.w_label)
        w_layout.addWidget(self.w_input)

        h_layout = QHBoxLayout()
        h_layout.addWidget(self.h_label)
        h_layout.addWidget(self.h_input)

        font_layout = QHBoxLayout()
        font_layout.addWidget(self.font_label)
        font_layout.addWidget(self.font_input)

        file_name_layout = QHBoxLayout()
        file_name_layout.addWidget(self.file_name_label)
        file_name_layout.addWidget(self.file_name_input)

        main_layout.addLayout(text_layout)
        main_layout.addLayout(w_layout)
        main_layout.addLayout(h_layout)
        main_layout.addLayout(font_layout)
        main_layout.addLayout(file_name_layout)
        main_layout.addWidget(self.button)

        self.setLayout(main_layout)

        #okno
        self.setWindowTitle("Moje GUI PyQt")
        self.setGeometry(300, 300, 400, 200)

    def on_button_click(self):

        # Pobranie danych z pól
        text = self.text_input.text()
        w = self.w_input.text()
        h = self.h_input.text()
        czcionka = self.font_input.currentData()
        file_name = self.file_name_input.text() + ".svg"

        # Przykładowa logika
        print("Tekst:", text)
        print("w:", w)
        print("h:", h)
        print("font:", czcionka)
        print("nazwa pliku:", file_name)
        wynik = self.funkcja_logiki(text,w,h)

        # Opcjonalnie: wypisujemy wynik, jeśli logika coś zwróciła
        if wynik:
             print("GUI: Otrzymano wynik z funkcji:", wynik)


# --- Testowanie tego pliku ---
if __name__ == "__main__":
    app = QApplication(sys.argv)
    
    #placeholder do sprawdzenia czy dziala
    def placeholder_svg_to_text(t, w, h, font_name="arial.ttf"):
        print(f"--- LOGIKA (TEST): Dostałem {t}, wymiary {w}x{h} ---")
        return "Gotowy SVG"

    # Przekazujemy tę funkcję do okna
    window = MyApp(placeholder_svg_to_text)
    
    window.show()
    sys.exit(app.exec())
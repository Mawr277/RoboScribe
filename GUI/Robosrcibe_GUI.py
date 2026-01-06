import sys
from PyQt6.QtWidgets import (
    QApplication,
    QWidget,
    QLabel,
    QLineEdit,
    QPushButton,
    QVBoxLayout,
    QHBoxLayout
)

class MyApp(QWidget):
    def __init__(self):
        super().__init__()
        self.init_ui()

    def init_ui(self):
        # --- Etykiety ---
        self.text_label = QLabel("Tekst:")
        self.w_label = QLabel("w:")
        self.h_label = QLabel("h:")

        # --- Pola tekstowe ---
        self.text_input = QLineEdit()
        self.w_input = QLineEdit()
        self.h_input = QLineEdit()

        # --- Przycisk ---
        self.button = QPushButton("Kliknij mnie")
        self.button.clicked.connect(self.on_button_click)

        # --- Layouty ---
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

        main_layout.addLayout(text_layout)
        main_layout.addLayout(w_layout)
        main_layout.addLayout(h_layout)
        main_layout.addWidget(self.button)

        self.setLayout(main_layout)

        # --- Okno ---
        self.setWindowTitle("Moje GUI PyQt")
        self.setGeometry(300, 300, 400, 200)

    def on_button_click(self):
        # Pobranie danych z pól
        text = self.text_input.text()
        w = self.w_input.text()
        h = self.h_input.text()

        # Przykładowa logika
        print("Tekst:", text)
        print("w:", w)
        print("h:", h)

        # Tu możesz wywołać dowolną funkcję
        # np. self.my_function(text, w, h)

    # def my_function(self, text, w, h):
    #     pass


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MyApp()
    window.show()
    sys.exit(app.exec())
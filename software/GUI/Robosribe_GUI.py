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
from PyQt6.QtGui import QIcon, QIntValidator
from PyQt6.QtSvgWidgets import QSvgWidget

LISTA_CZCIONEK = [
        ("Arial", "arial.ttf"),
        ("Times New Roman", "Times New Roman.ttf"),
    ]
APP_TITLE = "Roboscribe"
APP_ID = 'roboscribe.gui'
DEFAULT_WINDOW_SIZE = (1200,1000)

def launch_app(Create_SVG_logic, Create_GCODE_logic):
    myappid = APP_ID
    app = QApplication(sys.argv)
    try:
        ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(myappid)
    except ImportError:
        pass 
    window = MyApp(Create_SVG_logic, Create_GCODE_logic)
    window.show()
    sys.exit(app.exec())


def add_line_edit(parent_layout, label_text, placeholder=""):
    #element typu line_edit
    label = QLabel(label_text)
    line_edit = QLineEdit()
    line_edit.setPlaceholderText(placeholder)
    
    row_layout = QHBoxLayout()
    row_layout.addWidget(label)
    row_layout.addWidget(line_edit)
    parent_layout.addLayout(row_layout)
    
    return line_edit

def add_combo_box(parent_layout, label_text, items):
    #element typu combo_box
    label = QLabel(label_text)
    combo = QComboBox()
    for text,data in items:
        combo.addItem(text, data)

    #layour
    row_layout = QHBoxLayout()
    row_layout.addWidget(label)
    row_layout.addWidget(combo)
    parent_layout.addLayout(row_layout)

    return combo

def add_button(parent_layout,text, callback):
    button = QPushButton(text)
    button.clicked.connect(callback)
    parent_layout.addWidget(button)

    return button
    
class MyApp(QWidget):
    def __init__(self, function1, function2):
        super().__init__()

        # funkcje
        self.TextToSVG = function1
        self.SVGToGcode = function2 #funkcja ciabrosa
        self.base_path = os.path.dirname(os.path.abspath(__file__))

        self.setup_window()
        self.setup_ui()

    def setup_window(self):
        #okno
        self.setWindowTitle(APP_TITLE)
        self.resize(*DEFAULT_WINDOW_SIZE)

        #ustawienie ikony programu
        icon_path = os.path.join(self.base_path, 'logo_Roboscribe.png')
        if os.path.exists(icon_path):
            self.setWindowIcon(QIcon(icon_path))

    def setup_ui(self):
        #lewy panel
        left_widget = self.left_panel()

        #prawy panel
        self.svg_widget = QSvgWidget()
        self.svg_widget.setStyleSheet("background-color: #ffffff; border: 1px solid #cccccc;")

        #glowny layout
        main_layout = QHBoxLayout()
        main_layout.addWidget(left_widget,1)
        main_layout.addWidget(self.svg_widget,3)
        self.setLayout(main_layout)

    def left_panel(self):
        left_layout = QVBoxLayout()

        #pola do czesci SVG
        self.text_input = add_line_edit(left_layout, "Tekst:", "np: Roboscribe")
        self.w_input = add_line_edit(left_layout, "Szerokość:", "np: 300")
        self.h_input = add_line_edit(left_layout, "Wysokość", "np: 400")
        self.file_name_input = add_line_edit(left_layout, "Nazwa pliku wyjściowego:" , "np: output.svg")
        validator = QIntValidator(100, 9999, self)
        self.w_input.setValidator(validator)
        self.h_input.setValidator(validator)
        self.font_input = add_combo_box(left_layout, "Czcionka:", LISTA_CZCIONEK)
        self.Gen_SVG_button = add_button(left_layout, "Generuj podgląd",self.on_Gen_SVG_button_click)

        #pola do czesci GCODE
        self.scale = add_line_edit(left_layout,"Skala", "np:1")
        self.resolution = add_line_edit(left_layout,"Rozdzielczość", "np: 1")
        self.Gen_GCODE_button = add_button(left_layout, "Generuj GCODE", self.on_Gen_GCODE_button_click)

        #layout
        left_layout.addStretch() 
        left_container = QWidget()
        left_container.setLayout(left_layout)
        left_container.setMaximumWidth(300)
        return left_container


    def on_Gen_SVG_button_click(self):

        # Pobranie danych z pól
        text = self.text_input.text()
        raw_w = self.w_input.text()
        raw_h = self.h_input.text()
        czcionka = self.font_input.currentData()
        raw_name = self.file_name_input.text().strip()

        w = int(raw_w) if raw_w else 0
        h = int(raw_h) if raw_h else 0

        if not raw_name: 
            raw_name = "output" #domyślna nazwa
            
        if not raw_name.lower().endswith(".svg"):
            file_name = raw_name + ".svg"
        else:
            file_name = raw_name

        tests_dir = os.path.abspath(os.path.join(self.base_path, '..', 'TextToSvg', 'tests'))

        if not os.path.exists(tests_dir):
            os.makedirs(tests_dir)

        full_path = os.path.join(tests_dir, file_name)

        self.TextToSVG(w,h, text,file_name,czcionka)

        #trzeba zmienic placeholder (output_arial2.svg na file_name)
        if os.path.exists(full_path):
            self.svg_widget.load(full_path)
        else:
            print(f"Błąd: Plik {file_name} nie został znaleziony.")

        return full_path

    def on_Gen_GCODE_button_click(self):
        path_to_svg = self.on_Gen_SVG_button_click()
        if not path_to_svg:
            print("Nie można wygenerować GCODE - błąd pliku SVG.")
            return
        path_to_gcode = path_to_svg.replace(".svg", ".gcode")
        try:
            self.SVGToGcode(path_to_svg, path_to_gcode)
        except Exception as e:
            print(f"Wystąpił błąd podczas konwersji: {e}")

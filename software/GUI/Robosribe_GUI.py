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
    QFileDialog, 
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

    base_path = os.path.dirname(os.path.abspath(__file__))
    qss_file = os.path.join(base_path, 'style.qss')
    
    if os.path.exists(qss_file):
        with open(qss_file, "r") as f:
            app.setStyleSheet(f.read())
    
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

    #layout
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

def add_folder_selection(parent_layout, label_text, initial_path, button_text, callback):
    label = QLabel(label_text)
    line_edit = QLineEdit(initial_path)
    line_edit.setReadOnly(True) 
    parent_layout.addWidget(label)
    parent_layout.addWidget(line_edit)
    add_button(parent_layout, button_text, callback)
    line = QLabel("<hr>")
    parent_layout.addWidget(line)
    
    return line_edit

class MyApp(QWidget):
    def __init__(self, function1, function2):
        super().__init__()

        # funkcje
        self.TextToSVG = function1
        self.SVGToGcode = function2 

        self.base_path = os.path.dirname(os.path.abspath(__file__))

        test_folder_path = os.path.join(self.base_path, "test")
        if not os.path.exists(test_folder_path):
            os.makedirs(test_folder_path)
        self.working_directory = test_folder_path

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

        #glowny layout
        main_layout = QHBoxLayout()
        main_layout.addWidget(left_widget,1)
        main_layout.addWidget(self.svg_widget,3)
        self.setLayout(main_layout)

    def left_panel(self):
        left_layout = QVBoxLayout()
        
        #sciezka zapisu
        self.folder_display = add_folder_selection(
            parent_layout=left_layout,
            label_text="Folder zapisu:",
            initial_path=self.working_directory,
            button_text="Zmień folder zapisu",
            callback=self.select_working_folder
        )

        #pola do czesci SVG
        validator_size = QIntValidator(100, 9999, self)
        
        self.text_input = add_line_edit(left_layout, "Tekst:", "np: Roboscribe")
        self.w_input = add_line_edit(left_layout, "Szerokość:", "np: 300")
        self.h_input = add_line_edit(left_layout, "Wysokość", "np: 400")
        self.file_name_input = add_line_edit(left_layout, "Nazwa pliku wyjściowego:" , "np: output.svg")
        self.font_input = add_combo_box(left_layout, "Czcionka:", LISTA_CZCIONEK)
        self.Gen_SVG_button = add_button(left_layout, "Generuj podgląd",self.on_Gen_SVG_button_click)

        self.w_input.setValidator(validator_size)
        self.h_input.setValidator(validator_size)

        line = QLabel("<hr>")
        left_layout.addWidget(line)
        
        #pola do czesci GCODE
        validator_gcode = QIntValidator(1,10)
        self.scale = add_line_edit(left_layout,"Skala", "np:1")
        self.resolution = add_line_edit(left_layout,"Rozdzielczość", "np: 1")
        self.Gen_GCODE_button = add_button(left_layout, "Generuj GCODE", self.on_Gen_GCODE_button_click)

        self.scale.setValidator(validator_gcode)
        self.resolution.setValidator(validator_gcode)
        
        #layout
        left_layout.addStretch() 
        left_container = QWidget()
        left_container.setLayout(left_layout)
        left_container.setMaximumWidth(400)
        return left_container

    def select_working_folder(self):
        folder = QFileDialog.getExistingDirectory(self, "Wybierz folder zapisu", self.working_directory)
        if folder:
            self.working_directory = folder
            self.folder_display.setText(folder) 

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

        full_path = os.path.join(self.working_directory, file_name)

        self.TextToSVG(w,h, text,full_path,czcionka)

        if os.path.exists(full_path):
            self.svg_widget.load(full_path)
        else:
            print(f"Błąd: Plik {full_path} nie został znaleziony.")

        return full_path

    def on_Gen_GCODE_button_click(self):
        path_to_svg = self.on_Gen_SVG_button_click()

        raw_scale = self.scale.text()
        raw_resolution = self.resolution.text()
        scale = int(raw_scale) if raw_scale else 1
        resolution = int(raw_resolution) if raw_resolution else 1

        if not path_to_svg:
            print("Nie można wygenerować GCODE - błąd pliku SVG.")
            return
        path_to_gcode = path_to_svg.replace(".svg", ".gcode")
        try:
            self.SVGToGcode(path_to_svg, path_to_gcode,scale, resolution)
        except Exception as e:
            print(f"Wystąpił błąd podczas konwersji: {e}")

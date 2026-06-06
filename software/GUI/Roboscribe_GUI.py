import sys
import os
import ctypes
import requests
from PyQt6.QtWidgets import (
    QMainWindow,
    QApplication,
    QWidget,
    QLabel,
    QLineEdit,
    QPushButton,
    QVBoxLayout,
    QHBoxLayout,
    QComboBox,
    QFileDialog, 
    QTextEdit,
    QScrollArea 
)
from PyQt6.QtGui import QIcon, QIntValidator
from PyQt6.QtSvgWidgets import QSvgWidget
from PyQt6.QtCore import QTimer
from urllib.parse import parse_qs

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
    
    #zeby dzialalo na linux a
    if sys.platform == 'win32':
        try:
            ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID(myappid)
        except Exception:
            pass

    window = MyApp(Create_SVG_logic, Create_GCODE_logic)
    window.show()
    sys.exit(app.exec())

def add_header(parent_layout, text):
    label = QLabel(text)
    label.setObjectName("appTitle") 
    parent_layout.addWidget(label)
    return label

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
    line = QLabel("<hr>")

    parent_layout.addWidget(label)
    parent_layout.addWidget(line_edit)
    add_button(parent_layout, button_text, callback)
    parent_layout.addWidget(line)
    
    return line_edit

class MyApp(QMainWindow):
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

        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        #lewy panel
        left_widget = self.left_panel()

        #prawy panel
        self.svg_widget = QSvgWidget()

        #glowny layout
        main_layout = QHBoxLayout()
        central_widget.setLayout(main_layout)

        main_layout.addWidget(left_widget,1)
        main_layout.addWidget(self.svg_widget,3)

    def left_panel(self):
        left_layout = QVBoxLayout()

        add_header(left_layout, "ROBOSCRIBE")
        
        #sciezka zapisu
        self.folder_display = add_folder_selection(
            parent_layout=left_layout,
            label_text="Folder zapisu:",
            initial_path=self.working_directory,
            button_text="Zmień folder zapisu",
            callback=self.select_working_folder
        )

        #walidatory
        validator_size = QIntValidator(100, 9999, self)
        validator_gcode = QIntValidator(1,10)
        
        #pola do czesci SVG
        
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
        self.scale = add_line_edit(left_layout,"Skala", "np:1")
        self.resolution = add_line_edit(left_layout,"Rozdzielczość", "np: 1")
        self.Gen_GCODE_button = add_button(left_layout, "Generuj GCODE", self.on_Gen_GCODE_button_click)

        self.scale.setValidator(validator_gcode)
        self.resolution.setValidator(validator_gcode)

        #pole do czesci esp32
        line2 = QLabel("<hr>")
        left_layout.addWidget(line2)
        add_header(left_layout, "POŁĄCZENIE Z ESP32")

        # Pole do wpisania IP
        self.ip_input = add_line_edit(left_layout, "IP ESP32:", "192.168.0.117")
        self.ip_input.setText("192.168.0.117") # Domyślna wartość z Twojego skryptu

        #pole do wpisywania nazwy pliku docelowego
        self.target_file_input = add_line_edit(left_layout, "Plik w ESP32 (do startu/usunięcia):", "np: output.gcode")

        #przycisk wysyłania na serwer
        self.Upload_button = add_button(left_layout, "Wyślij WYGENEROWANY GCODE", self.on_upload_click)
        self.Upload_custom_button = add_button(left_layout, "Wybierz plik z komputera i wyślij", self.on_upload_custom_file_click)

        #przyciski sterowania stanem
        row_state = QHBoxLayout()
        self.btn_start = QPushButton("START")
        self.btn_pause = QPushButton("PAUSE")
        self.btn_stop = QPushButton("STOP")
        
        row_state.addWidget(self.btn_start)
        row_state.addWidget(self.btn_pause)
        row_state.addWidget(self.btn_stop)
        left_layout.addLayout(row_state)

        self.btn_start.clicked.connect(lambda: self.set_robot_state(1))
        self.btn_pause.clicked.connect(lambda: self.set_robot_state(2))
        self.btn_stop.clicked.connect(lambda: self.set_robot_state(0))

        #sterowanie ręczne
        line3 = QLabel("<hr>")
        left_layout.addWidget(line3)
        add_header(left_layout, "STEROWANIE RĘCZNE")
        
        self.base_angle = add_line_edit(left_layout, "Kąt podstawy:", "np: 90")
        self.arm_angle = add_line_edit(left_layout, "Kąt ramienia:", "np: 45")
        self.tool_angle = add_line_edit(left_layout, "Kąt narzędzia:", "np: 0")
        self.x_coord = add_line_edit(left_layout, "Wsp. X:", "np: 100")
        self.y_coord = add_line_edit(left_layout, "Wsp. Y:", "np: 50")
        
        self.btn_send_controls = add_button(left_layout, "Wyślij pozycję", self.on_manual_control_click)

        # 2. DANE I ZARZĄDZANIE PLIKAMI
        line4 = QLabel("<hr>")
        left_layout.addWidget(line4)
        add_header(left_layout, "DANE I ZARZĄDZANIE")

        self.btn_delete = add_button(left_layout, "Usuń obecny plik z ESP32", self.on_delete_file_click)

        self.accel_label = QLabel("Akcelerometr: Brak danych")
        left_layout.addWidget(self.accel_label)
        self.btn_get_data = add_button(left_layout, "Start pobierania danych z akcelerometru", self.toggle_accel_data)

        # Inicjalizacja timera
        self.accel_timer = QTimer(self)
        self.accel_timer.timeout.connect(self.fetch_accel_data)

        #log console
        line5 = QLabel("<hr>")
        left_layout.addWidget(line5)
        add_header(left_layout, "LOGI SYSTEMOWE")
        
        self.log_console = QTextEdit()
        self.log_console.setReadOnly(True)
        self.log_console.setMinimumHeight(120)
        left_layout.addWidget(self.log_console)

        #layout
        left_layout.addStretch() 
        left_container = QWidget()
        left_container.setLayout(left_layout)
        
        
        #scroll area
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setWidget(left_container)
        scroll_area.setMaximumWidth(420) 
        
        return scroll_area

    def log_message(self, message):
        self.log_console.append(message)
        
        #automatyczne przewijanie na sam dół
        scrollbar = self.log_console.verticalScrollBar()
        scrollbar.setValue(scrollbar.maximum())

    def select_working_folder(self):
        folder = QFileDialog.getExistingDirectory(self, "Wybierz folder zapisu", self.working_directory)
        if folder:
            self.working_directory = folder
            self.folder_display.setText(folder) 

    def on_Gen_SVG_button_click(self):

        text = self.text_input.text()
        raw_w = self.w_input.text()
        raw_h = self.h_input.text()
        czcionka = self.font_input.currentData()
        raw_name = self.file_name_input.text().strip()

        w = int(raw_w) if raw_w else 0
        h = int(raw_h) if raw_h else 0

        if not raw_name: 
            raw_name = "output"     #domyślna nazwa
            
        if not raw_name.lower().endswith(".svg"):
            file_name = raw_name + ".svg"
        else:
            file_name = raw_name

        full_path = os.path.join(self.working_directory, file_name)

        self.TextToSVG(w,h, text,full_path,czcionka)

        if os.path.exists(full_path):
            self.svg_widget.load(full_path)
        else:
            self.log_message(f"[BŁĄD] Plik {full_path} nie został znaleziony.")

        return full_path

    def on_Gen_GCODE_button_click(self):
        path_to_svg = self.on_Gen_SVG_button_click()

        raw_scale = self.scale.text()
        raw_resolution = self.resolution.text()
        scale = int(raw_scale) if raw_scale else 1
        resolution = int(raw_resolution) if raw_resolution else 1

        if not path_to_svg:
            self.log_message("[BŁĄD] Nie można wygenerować GCODE - błąd pliku SVG.")
            return
        path_to_gcode = path_to_svg.replace(".svg", ".gcode")
        self.last_gcode_path = path_to_gcode
        try:
            self.SVGToGcode(path_to_svg, path_to_gcode,scale, resolution)
            self.log_message(f"Wygenerowano GCODE: {path_to_gcode}")
        except Exception as e:
            self.log_message(f"[BŁĄD] Wystąpił błąd podczas konwersji: {e}")

    def get_esp_url(self):
        """Zwraca bazowy adres URL na podstawie wpisanego IP."""
        ip = self.ip_input.text().strip()
        return f"http://{ip}"

    def on_upload_click(self):
        """Wysyła ostatnio wygenerowany plik GCODE na serwer ESP32."""
        if not hasattr(self, 'last_gcode_path') or not os.path.exists(self.last_gcode_path):
            self.log_message("[BŁĄD] Najpierw wygeneruj plik GCODE!")
            return

        filename = os.path.basename(self.last_gcode_path)
        url = f"{self.get_esp_url()}/upload/{filename}"

        try:
            self.log_message(f"Wysyłanie {filename} na {url}...")
            with open(self.last_gcode_path, "rb") as f:
                file_data = f.read()
            
            response = requests.post(url, data=file_data, timeout=5)
            self.log_message(f"Odpowiedź serwera: {response.text}")
        except requests.exceptions.RequestException as e:
            self.log_message(f"[BŁĄD POŁĄCZENIA] podczas wysyłania: {e}")

    def set_robot_state(self, state_val):
        """Wysyła polecenie zmiany stanu do robota (0: STOP, 1: START, 2: PAUSE)."""
        url = f"{self.get_esp_url()}/state"
        payload = {'statusCode': state_val}
        
        if state_val == 1:
            filename = self.target_file_input.text().strip()
            if not filename:
                self.log_message("[BŁĄD] Wpisz w odpowiednie pole nazwę pliku, który chcesz uruchomić!")
                return
            payload['filename'] = filename
        
        try:
            self.log_message(f"Wysyłanie stanu {state_val} na {url}...")
            response = requests.post(url, data=payload, timeout=3)
            self.log_message(f"Odpowiedź serwera: {response.text}")
        except requests.exceptions.RequestException as e:
            self.log_message(f"[BŁĄD POŁĄCZENIA] podczas zmiany stanu: {e}")

    def on_manual_control_click(self):
        """Pobiera dane z pól tekstowych i wysyła komendę sterowania ręcznego do ESP32."""
        url = f"{self.get_esp_url()}/controls"
        
        payload = {
            'base_angle': self.base_angle.text() or "0",
            'arm_angle': self.arm_angle.text() or "0",
            'tool_angle': self.tool_angle.text() or "0",
            'x_coord': self.x_coord.text() or "0",
            'y_coord': self.y_coord.text() or "0"
        }
        
        try:
            self.log_message(f"Wysyłanie sterowania ręcznego na {url}...")
            response = requests.post(url, data=payload, timeout=3)
            self.log_message(f"Odpowiedź serwera: {response.text}")
        except requests.exceptions.RequestException as e:
            self.log_message(f"[BŁĄD KOMUNIKACJI] podczas wysyłania pozycji: {e}")

    def on_delete_file_click(self):
        """Wysyła prośbę o usunięcie pliku GCODE z pamięci robota."""
        filename = self.target_file_input.text().strip()
        
        if not filename:
            self.log_message("[BŁĄD] Wpisz w odpowiednie pole nazwę pliku, który chcesz usunąć z ESP32!")
            return
            
        url = f"{self.get_esp_url()}/delete/{filename}"
        
        try:
            self.log_message(f"Usuwanie pliku {filename} z {url}...")
            response = requests.post(url, timeout=5)
            self.log_message(f"Odpowiedź serwera: {response.text}")
        except requests.exceptions.RequestException as e:
            self.log_message(f"[BŁĄD] usuwania pliku: {e}")

    def toggle_accel_data(self):
        """Włącza lub wyłącza ciągłe pobieranie danych z akcelerometru."""
        if self.accel_timer.isActive():
            self.accel_timer.stop()
            self.btn_get_data.setText("Start pobierania ciągłego (Akcelerometr)")
            self.log_message("Zatrzymano odczyt z akcelerometru.")
        else:
            #Zapytanie co 500ms
            self.accel_timer.start(500) 
            self.btn_get_data.setText("Stop pobierania (Akcelerometr)")
            self.log_message("Rozpoczęto ciągły odczyt z akcelerometru...")

    def fetch_accel_data(self):
        """Pobiera dane w tle ."""
        url = f"{self.get_esp_url()}/accel_data"
        try:
            #timeout 1s
            response = requests.get(url, timeout=1) 
            
            parsed_data = parse_qs(response.text)
            
            x = parsed_data.get('x_accel', ['0'])[0]
            y = parsed_data.get('y_accel', ['0'])[0]
            z = parsed_data.get('z_accel', ['0'])[0]
            
            info = f"Akcelerometr: X={x}, Y={y}, Z={z}"
            self.accel_label.setText(info)
            
        except requests.exceptions.RequestException:
            #zatrzymanie w razie błedu
            self.accel_timer.stop()
            self.btn_get_data.setText("Start pobierania danych z akcelerometru")
            self.accel_label.setText("Akcelerometr: Błąd połączenia (Zatrzymano)")
            self.log_message("[BŁĄD] Utracono połączenie z akcelerometrem. Odczyt zatrzymany.")

    def on_upload_custom_file_click(self):
        """Otwiera okno dialogowe, pozwala wybrać plik i wysyła go na ESP32."""
        #otwarcie okna wyboru pliku
        file_path, _ = QFileDialog.getOpenFileName(
            self,
            "Wybierz plik GCODE do wysłania",
            self.working_directory,
            "GCODE Files (*.gcode);;Wszystkie pliki (*)"
        )

        if not file_path:
            return

        filename = os.path.basename(file_path)
        url = f"{self.get_esp_url()}/upload/{filename}"

        try:
            self.log_message(f"Wysyłanie wybranego pliku {filename} na {url}...")
            #odczytanie zawartości wybranego pliku z dysku
            with open(file_path, "rb") as f:
                file_data = f.read()
            
            # Wysłanie na serwer
            response = requests.post(url, data=file_data, timeout=5)
            self.log_message(f"Odpowiedź serwera: {response.text}")
        except requests.exceptions.RequestException as e:
            self.log_message(f"[BŁĄD POŁĄCZENIA] podczas wysyłania: {e}")
        except FileNotFoundError:
            self.log_message("[BŁĄD] Nie można odnaleźć wybranego pliku.")
import os
import requests
from urllib.parse import parse_qs
from enum import Enum
from dataclasses import asdict, dataclass

esp32_ip = "192.168.0.117"
esp32_addr = f"http://{esp32_ip}"
set_state_url = esp32_addr + "/state"
delete_url = esp32_addr + "/delete/"
upload_url = esp32_addr + "/upload/"
control_url = esp32_addr + "/controls"
accel_data_url = esp32_addr + "/accel_data"

example_file = "L.gcode"

class state(Enum):
    STOP = 0
    START = 1
    PAUSE = 2

@dataclass
class Controls:
    base_angle: int = 0
    arm_angle: int = 0
    tool_angle: int = 0
    x_coord: int = 0
    y_coord: int = 0

def set_state(roboState: state):
    payload = {'statusCode': roboState.value}

    try:
        response = requests.post(set_state_url, data=payload)
        print(response.text)
    except requests.exceptions.RequestException as e:
        print(e)

def manual_control(movement_data: Controls):
    payload = asdict(movement_data)

    try:
        response = requests.post(control_url, data=payload)
        print(response.text)
    except requests.exceptions.RequestException as e:
        print(e)

def get_data():
    try:
        response = requests.get(accel_data_url, timeout=10)
        
        parsed_data = parse_qs(response.text)

        x_accel = int(parsed_data.get('x_accel', ['0'])[0])
        y_accel = int(parsed_data.get('y_accel', ['0'])[0])
        z_accel = int(parsed_data.get('z_accel', ['0'])[0])

        print(x_accel, y_accel, z_accel)
    except requests.exceptions.RequestException as e:
        print(e)

def delete_file(filename: str):
    try:
        response = requests.post(delete_url + filename)
        print(response.text)
    except requests.exceptions.RequestException as e:
        print(e)

def upload_file(filename: str):
    with open(filename, "rb") as file:
        file_data = file.read()

    try:
        response = requests.post(upload_url + filename, data=file_data)
        print(response.text)
    except FileNotFoundError:
        print("Local file not found.")  
    except requests.exceptions.RequestException as e:
        print(e)

set_state(state.START)
set_state(state.STOP)
set_state(state.PAUSE)
set_state(state.START)
set_state(state.PAUSE)

delete_file(example_file)
upload_file(example_file)

my_input = Controls();
manual_control(my_input)

get_data()

import requests
from datetime import datetime

UBIDOTS_TOKEN = "BBUS-VKMrY9nBvYdn5QpmgN2jsZQ6beUh3E"
DEVICE_LABEL = "solar-panel-tracker"
BASE_URL = "https://industrial.api.ubidots.com/api/v1.6"
HEADERS = {
    "X-Auth-Token": UBIDOTS_TOKEN,
    "Content-Type": "application/json"
}

# Variabel dan data
VARIABLE_SERVO_H = "servo_h"
VARIABLE_SERVO_V = "servo_v"
VARIABLE_LDR = "ldr"
posisiServo2 = 90
posisiServo1 = 45
sensor_mean = 512

def get():
    results = {}
    for var_id in [VARIABLE_LDR, VARIABLE_SERVO_V, VARIABLE_SERVO_H]:
        url = f"{BASE_URL}/devices/{DEVICE_LABEL}/{var_id}/lv"
        response = requests.get(url, headers=HEADERS)
        if response.status_code == 200:
            results[var_id] = response.json()
        else:
            results[var_id] = 0
    
    now = datetime.now()
    results['hour'] = now.hour
    results['minute'] = now.minute
    results['dayofweek'] = now.weekday()
    return results
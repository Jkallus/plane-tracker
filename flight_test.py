import math
import time
import json
import paho.mqtt.client as mqtt
from FlightRadar24.api import FlightRadar24API
from geopy import distance
fr_api = FlightRadar24API()
logan = fr_api.get_airport("KBOS")

class tracking_data:
    pass

# Apartment location
x = -71.059081
y = 42.352255
home = (x, y)


# Apartment rectangle
x1 = -71.071
y1 = 42.349
x2 = -70.906691
y2 =  42.104463

bounds = f'{y1},{y2},{x1},{x2}'

airline_callsign_to_name = {
    'JBU': 'Jet Blue',
    'UAL': 'United',
    'DAL': 'Delta',
    'ASA': 'Alaska',
    'AAL': 'American',
    'AFR': 'Air France',
    'BAW': 'British Airways',
    'KAP': 'Cape Air',
    'SAS': 'Scandinavian',
    'EIN': 'Aer Lingus',
    'SWA': 'Southwest',
    'EJA': 'NetJets',
    'NKS': 'Spirit',
    'AAY': 'Allegiant',
    'POE': 'Porter Airlines',
    'KLM': 'KLM',
    'DLH': 'Lufthansa',
    'VIR': 'Virgin Atlantic'
}

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")

client = mqtt.Client()
client.on_connect = on_connect

client.connect("192.168.8.198", 1883, 60)


while True:
    processed_results = []
    results = fr_api.get_flights(bounds = bounds)
    for result in results:
        single_result = {}
        single_result["Aircraft"] = result.aircraft_code
        single_result["Speed"] = result.ground_speed
        location = (result.longitude, result.latitude)
        single_result["Distance"] = distance.distance(home, location).miles
        if result.airline_icao in airline_callsign_to_name:
            single_result["FlightNumber"] = f'{airline_callsign_to_name[result.airline_icao]}'
        else:
            single_result["FlightNumber"] = result.callsign
        single_result["FlightNumber"] = single_result["FlightNumber"][:9]
        if result.heading > 100 and result.heading < 300:
            single_result["Direction"] = "Out"
        else:
            single_result["Direction"] = "In"
        processed_results.append(single_result)

    processed_results.sort(key=lambda x : x['Distance'])
    print('Results:')
    json_str = json.dumps(processed_results)
    print(json_str)
    client.publish("flight_data_json", json_str)        
    time.sleep(1)

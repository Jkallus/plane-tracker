import math
import time
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
#x1 = -73.0
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
    'DLH': 'Lufthansa'

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
        data = tracking_data()
        data.aircraft = result.aircraft_code
        location = (result.longitude, result.latitude)
        data.miles_to_me = distance.distance(home, location).miles
        data.speed = result.ground_speed
        if result.airline_icao in airline_callsign_to_name:
            data.title = f'{airline_callsign_to_name[result.airline_icao]}'
        else:
            data.title = result.callsign
        if result.heading > 100 and result.heading < 300:
            data.direction = "Out"
        else:
            data.direction = "In"
        processed_results.append(data)

    processed_results.sort(key=lambda x : x.miles_to_me)
    print('Results:')
    publish_text = ''
    for result in processed_results:
        text = f'{result.title} {result.aircraft} {result.miles_to_me:.01f} {result.direction}\n' 
        print(text)
        publish_text = publish_text + text + ' '
    client.publish("flight_data", publish_text)        
    time.sleep(1)

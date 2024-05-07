# python 3.11

import random
import json
from paho.mqtt import client as mqtt_client


broker = 'csse4011-iot.zones.eait.uq.edu.au'
port = 1883
topic = "s4580286"
# Generate a Client ID with the subscribe prefix.
client_id = f'subscribe-{random.randint(0, 100)}'
um_distance = 0

def replace_quotes(input_string):
    return input_string.replace("'", '"')

def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    # client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        buf = msg.payload.decode()
        print(f"Received `{buf}` from `{msg.topic}` topic")
        output_str = replace_quotes(buf)
        ## Check JSON format 
        #convert string to  object
        json_object = json.loads(output_str)
        #check new data type
        print(output_str)

        #access data in dict
        um_distance = json_object["data"]
        print(um_distance)

    client.subscribe(topic)
    client.on_message = on_message


def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()


if __name__ == '__main__':
    run()

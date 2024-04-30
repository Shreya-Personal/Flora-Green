import serial
import time 
import json
import re
import ast
import random
import math

from paho.mqtt import client as mqtt_client

## Serial Connection 
serialPort = serial.Serial(port = "/dev/ttyACM0", baudrate=115200,
                           bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)
serialString = ""                           # Used to hold data coming over UART

##MQTT Broker details  
broker = 'csse4011-iot.zones.eait.uq.edu.au'
port = 1883
topic = "s4580286"
# Generate a Client ID with the subscribe prefix.
client_id = f'subscribe-{random.randint(0, 100)}'

## Globals 
temp = 0 
co2 = 0
sound = 0
temp_thres = 29
co2_thres = 400
sound_thres = 10
petal = 0
b = 0

def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    # client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def publish(client):
    msg_count = 1
    while True:
        time.sleep(0.5)
        msg = '{time":'+str(round(time.time()))+',"petal":' + str(petal) + ',"b":' + str(b) +'}'
        result = client.publish(topic, msg)
        # result: [0, 1]
        status = result[0]
        if status == 0:
            print(f"Send `{msg}` to topic `{topic}`")
            break
        else:
            print(f"Failed to send message to topic {topic}")
        msg_count += 1
        if msg_count > 5:
            break


def replace_quotes(input_string):
    return input_string.replace("'", '"')

def map_co2_to_scale(co2_value, co2_threshold):
    scale_range = 800  # Range mapped to the scale
    midpoint_value = 5  # Midpoint level on the 1-10 scale
    
    #Calculate grad 
    scale_half = scale_range/2
    grad = 10/((co2_threshold+scale_half)-(co2_threshold-scale_half))
    c = 10-grad*(co2_threshold+scale_half)
    mapped_value = round(co2_value*grad + c)
    
    return mapped_value


##### NEEDS THIS LOOP TO UPDATE GLOBALS 
def main():
    global temp, co2, sound, petal, b
    ##serialPort.write(b"view -a\n")
    while(1):

        # Wait until there is data waiting in the serial buffer and update values
        if(serialPort.in_waiting > 0):

            # Read data out of the buffer until a carraige return / new line is found
            serialString = serialPort.readline()
            # Print the contents of the serial data
            buf = serialString.decode('Ascii')
            input_string = replace_quotes(str(buf))
            json_string = 0
            
            if "JSON" in input_string:
                json_string = input_string.split("JSON")[1]
                print(json_string)
                json_object = json.loads(str(json_string))

                #access data in dict
                temp = json_object["Temp"]
                co2 = json_object["Co2"]
                sound = json_object["Sound"]
                # print(temp, sound, co2)
                time.sleep(1)
            else: 
                #print(buf)
                time.sleep(.01)
        ## First - define petal 
        if (temp_thres > temp/10) and (sound_thres > sound): 
            petal = 0
        else: 
            petal = 1
        
        #Second - define b based on threshold 
        b = map_co2_to_scale(co2, co2_thres)
        msg = '{time":'+str(round(time.time()))+',"petal":' + str(petal) + ',"b":' + str(b) +'}'
        print(msg)
        # Publish to mqtt 
        client = connect_mqtt()
        client.loop_start()
        publish(client)
        client.loop_stop()

                

if __name__ == '__main__':
    main()
        

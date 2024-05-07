import serial
import time 
import json
import re
import ast
import random
import math
from paho.mqtt import client as mqtt_client
from filterpy.kalman import KalmanFilter ## Kalman Initialisation 
from filterpy.common import Q_discrete_white_noise
import numpy as np

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
temp = 0.00 
co2 = 0.00
sound = 0.00
temp_thres = 35.0
co2_thres = 500.0
sound_thres = 10.0
petal = 0
b = 0

def initialize_kalman_filter(temp,co2,sound):
    """
    Initializes and configures a Kalman filter for sensor fusion.

    Returns:
    KalmanFilter: Initialized Kalman filter object.
    """
    # Define Kalman filter parameters
    dt = 1.0  # Time step

    # Define initial state vector [temperature, CO2, sound]
    x = np.array([[temp], [co2], [sound]])

    # Define Kalman filter
    kf = KalmanFilter(dim_x=3, dim_z=3)
    kf.x = x
    kf.F = np.array([[1, dt, 0],
                     [0, 1, dt],
                     [0, 0, 1]])
    kf.H = np.eye(3)
    kf.P *= 0.00001  # Adjust the initial covariance matrix for higher accuracy
    kf.R *= 0.001  # Adjust the measurement noise covariance matrix
    q = Q_discrete_white_noise(dim=2, dt=dt, var=0.01)  # Adjust the process noise
    kf.Q = np.diag([q[0, 0], q[1, 1], 0.01])  # Default process noise for CO2 and sound
    kf.Q[0, 0] = 2.0  # Increase process noise for temperature measurement
    
    return kf


def kalman_filter_update(kf, temp, co2, sound):
    """
    Update the Kalman filter with new sensor measurements.

    Args:
    kf (KalmanFilter): Initialized Kalman filter object.
    temp (float): Temperature measurement.
    co2 (float): CO2 measurement.
    sound (float): Sound measurement.

    Returns:
    np.ndarray: Updated state vector from the Kalman filter.
    """
    z = np.array([[temp], [co2], [sound]])
    kf.predict()
    kf.update(z)
    return kf.x


def petal_decision_logic(temp, co2, sound):
    """
    Determine whether the petals should be open or closed based on sensor readings.

    Args:
    temp (float): Temperature reading.
    co2 (float): CO2 level reading.
    sound (float): Sound level reading.

    Returns:
    int: 1 if petals should be open, 0 if petals should be closed.
    """
    print("TEMP", temp, co2, sound)
    global temp_thres, co2_thres, sound_thres
    if temp > temp_thres or co2 > co2_thres or sound > sound_thres:
        return 1  # Petals closed
    else:
        return 0  # Petals open


def connect_mqtt() -> mqtt_client:
    """
    Connect to the MQTT broker and return the MQTT client object.

    Returns:
    mqtt_client: MQTT client object.
    """
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    # client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client


def publish(client):
    """
    Publishes messages to the MQTT broker.

    Args:
    client (mqtt_client): MQTT client object.

    Returns:
    None
    """
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
    #Replaces single quotes with double quotes for JSON 
    return input_string.replace("'", '"')

def map_co2_to_scale(co2_value, co2_threshold):
    """
    Maps a CO2 value to a scale from 0 to 10 based on a threshold.

    Args:
    co2_value (float): The CO2 value to map.
    co2_threshold (float): The threshold value for CO2.

    Returns:
    int: The mapped value on the scale from 0 to 10.
    """
    scale_range = 800  # Range mapped to the scale
    midpoint_value = 5  # Midpoint level on the 1-10 scale
    
    #Calculate grad 
    scale_half = scale_range/2
    grad = 10/((co2_threshold-scale_half)-(co2_threshold+scale_half))
    c = 10-grad*(co2_threshold-scale_half)
    mapped_value = round(co2_value*grad + c)
    
    if (mapped_value > 10):
        mapped_value = 10
    if (mapped_value < 0): 
        mapped_value = 0

    return mapped_value


 
def main():
    """
    Main function for processing sensor data, applying Kalman filtering, making petal status decisions,
    and publishing data to an MQTT broker.

    This function initializes the Kalman filter, reads sensor data from a serial port, processes the data
    using the Kalman filter, determines petal status based on filtered data and threshold values,
    and publishes the petal status and other data to an MQTT broker.

    Global variables used:
    - temp: Current temperature reading
    - co2: Current CO2 level reading
    - sound: Current sound level reading
    - petal: Petal status (1 for open, 0 for closed)
    - b: Mapped CO2 value representing LED brightness

    Returns:
    None
    """
    global temp, co2, sound, petal, b
    serialRec = 0
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
                temp = int(round(json_object["Temp"]/10))
                co2 = json_object["Co2"]
                sound = json_object["Sound"]
                if serialRec == 0: 
                    kf = initialize_kalman_filter(temp,co2,sound)
                    serialRec = 1
                time.sleep(.01)
            else: 
                #print(buf)
                time.sleep(.01)

        if serialRec == 1:
            #Process data  
            x_filtered = kalman_filter_update(kf, temp, co2, sound)

            # Determine petal status based on Kalman filter output
            petal = petal_decision_logic(x_filtered[0, 0], x_filtered[1, 0], x_filtered[2, 0])

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
        

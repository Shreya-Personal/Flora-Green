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

import tkinter as tk
from tkinter import *

MIN_TEMP = 0
MAX_TEMP = 50

MIN_SOUND = 0
MAX_SOUND = 100

MIN_CO2 = 0
MAX_CO2 = 1000

FLOWER_PETAL_LEVEL = 0
FLOWER_LED_LEVEL = 0
  

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
# Measurements  
temp_meas = 0.00 
co2_meas= 0.00
sound_meas = 0.00

# Thresholds 
temp_thres = 35.0
co2_thres = 500.0
sound_thres = 10.0
temp_min_thres = 0 
co2_min_thres = 0
sound_min_thres = 0

# Status 
petal_status = 0
b_status = 0

temp_meas = 30.00 
co2_meas= 400.00
sound_meas = 10.00

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
    # print("TEMP", temp, co2, sound)
    global temp_thres, co2_thres, sound_thres, temp_min_thres, sound_min_thres, co2_min_thres
    if temp > temp_thres or co2 > co2_thres or sound > sound_thres or temp < temp_min_thres or sound < sound_min_thres or co2 < co2_min_thres:
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
        msg = '{time":'+str(round(time.time()))+',"p":' + str(petal_status) + ',"b":' + str(b_status) +'}'
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
    
    if (mapped_value > 9):
        mapped_value = 9
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
    global temp_meas, co2_meas, sound_meas, petal_status, b_status, temp_thres, co2_thres, sound_thres, temp_min_thres, sound_min_thres, co2_min_thres
    serialRec = 0 ################# REMEMBER TO CHANGE TO 0

    # MQTT Connect 
    client = connect_mqtt()

    ## GUI INITIALISATION
    # Create the main window
    root = tk.Tk()
    root.geometry("600x600")
    root.title("CSSE4011 Mechanical Flower Monitor")

    # Set background colors for frames
    root.configure(bg='#B2FFB5')  # Light green color
    left_frame = Frame(root, bg='#B2FFB5')
    right_frame = Frame(root, bg='#B2FFB5')  # Light green color
    left_frame.pack(side=LEFT, fill=BOTH, expand=True) 
    right_frame.pack(side=RIGHT, fill=Y)

    # Frame for Temperature Sliders
    temp_frame = Frame(left_frame, bg='#E1FFE4', bd=2, relief="ridge")  # Light green color, border
    temp_frame.pack(side=TOP, padx=20, pady=10, fill=BOTH, expand=True)  # Fill both x and y directions, expand to fill available space
    Label(temp_frame, text="Temperature Settings", bg='#E1FFE4').pack(side=TOP, padx=10, pady=5)  # Label for the box

    min_temp = Scale(temp_frame, from_=MIN_TEMP, to=MAX_TEMP, label="Min Temp", orient=HORIZONTAL, length=300)  # Adjusted length
    min_temp.pack(side=TOP, padx=10, pady=5)
    max_temp = Scale(temp_frame, from_=MIN_TEMP, to=MAX_TEMP, label="Max Temp", orient=HORIZONTAL, length=300)  # Adjusted length
    max_temp.pack(side=TOP, padx=10, pady=5)

    # Frame for CO2 Sliders
    co2_frame = Frame(left_frame, bg='#E1FFE4', bd=2, relief="ridge")  # Light green color, border
    co2_frame.pack(side=TOP, padx=20, pady=10, fill=BOTH, expand=True)  # Fill both x and y directions, expand to fill available space
    Label(co2_frame, text="CO2 Settings", bg='#E1FFE4').pack(side=TOP, padx=10, pady=5)  # Label for the box

    min_co2 = Scale(co2_frame, from_=MIN_CO2, to=MAX_CO2, label="Min CO2", orient=HORIZONTAL, length=300)  # Adjusted length
    min_co2.pack(side=TOP, padx=10, pady=5)
    max_co2 = Scale(co2_frame, from_=MIN_CO2, to=MAX_CO2, label="Max CO2", orient=HORIZONTAL, length=300)  # Adjusted length
    max_co2.pack(side=TOP, padx=10, pady=5)

    # Frame for Sound Sliders
    sound_frame = Frame(left_frame, bg='#E1FFE4', bd=2, relief="ridge")  # Light green color, border
    sound_frame.pack(side=TOP, padx=20, pady=10, fill=BOTH, expand=True)  # Fill both x and y directions, expand to fill available space
    Label(sound_frame, text="Sound Settings", bg='#E1FFE4').pack(side=TOP, padx=10, pady=5)  # Label for the box

    min_sound = Scale(sound_frame, from_=MIN_SOUND, to=MAX_SOUND, label="Min Sound", orient=HORIZONTAL, length=300)  # Adjusted length
    min_sound.pack(side=TOP, padx=10, pady=5)
    max_sound = Scale(sound_frame, from_=MIN_SOUND, to=MAX_SOUND, label="Max Sound", orient=HORIZONTAL, length=300)  # Adjusted length
    max_sound.pack(side=TOP, padx=10, pady=5)

    # Right Frame: Sensor readings and status
    sensor_frame = Frame(right_frame, bg='#B2FFB5', bd=6, relief="ridge")
    sensor_frame.pack(side=TOP, pady=120 ,  padx=20)
    temp = Label(sensor_frame, text="TEMPERATURE: 0 - 0 degC", bg='#B2FFB5', padx=20, pady=20)
    temp.pack(side=TOP)
    sound = Label(sensor_frame, text="SOUND LEVEL: 0 - 0 dB", bg='#B2FFB5',padx=20, pady=20)
    sound.pack(side=TOP)
    co2 = Label(sensor_frame, text="CO2 LEVEL: 0 - 0 ppm", bg='#B2FFB5',padx=20, pady=20)
    co2.pack(side=TOP)

    status_frame = Frame(right_frame, bg='#B2FFB5')
    status_frame.pack(side=BOTTOM, pady=20)
    petal = Label(status_frame, text="FLOWER PETAL STATUS: 0", bg='#B2FFB5')
    petal.pack(side=TOP)
    led = Label(status_frame, text="FLOWER LED STATUS: 0", bg='#B2FFB5')
    led.pack(side=TOP)

    temp_text = "TEMPERATURE: {temp} degC"
    led_text = "FLOWER LED STATUS: {led}"
    co2_text = "CO2 LEVEL: {co2} ppm"
    sound_text = "SOUND LEVEL: {sound} dB"
    petal_text = "FLOWER PETAL STATUS: {petal}"
    ##serialPort.write(b"view -a\n")
    # kf = initialize_kalman_filter(temp_meas,co2_meas,sound_meas)
    while(1):
        time.sleep(.1)
        # Wait until there is data waiting in the serial buffer and update values
        if(serialPort.in_waiting > 0):
            # Read data out of the buffer until a carraige return / new line is found
            serialString = serialPort.readline()
            # Print the contents of the serial data
            buf = serialString.decode('Ascii')
            input_string = replace_quotes(str(buf))
            json_string = 0
            try: 
                if "JSON" in input_string:
                    json_string = input_string.split("JSON")[1]
                    print(json_string)
                    json_object = json.loads(str(json_string))

                    #access data in dict
                    temp_meas = int(round(json_object["Temp"]/10))
                    co2_meas = json_object["Co2"]
                    sound_meas = json_object["Sound"]
                    if serialRec == 0: 
                        kf = initialize_kalman_filter(temp_meas,co2_meas,sound_meas)
                        serialRec = 1
                    time.sleep(.01)
                    serialRec = 1
            except: 
                #print(buf)
                time.sleep(.01)
                serialRec = 0

        if serialRec == 1:
            #Process data  
            x_filtered = kalman_filter_update(kf, temp_meas, co2_meas, sound_meas)

            # Determine petal status based on Kalman filter output
            petal_status = petal_decision_logic(x_filtered[0, 0], x_filtered[1, 0], x_filtered[2, 0])

            #Second - define b based on threshold 
            b_status = map_co2_to_scale(x_filtered[1, 0], co2_thres)
            msg = '{time":'+str(round(time.time()))+',"petal":' + str(petal_status) + ',"b":' + str(b_status) +'}'
            print(msg)
            
            # update sensor measurement labels
            temp.config(text=temp_text.format(temp=round(x_filtered[0, 0],2)))
            sound.config(text=sound_text.format(sound=round(x_filtered[2, 0],2)))
            co2.config(text=co2_text.format(co2=round(x_filtered[1, 0])))

            #Update Vars
            temp_thres = max_temp.get()
            temp_min_thres = min_temp.get()
            sound_thres = max_sound.get()
            sound_min_thres = min_sound.get()
            co2_thres = max_co2.get()
            co2_min_thres = min_co2.get()

            # update status labels
            petal.config(text=petal_text.format(petal=petal_status))
            led.config(text=led_text.format(led=b_status))
            root.update()

            # Publish to mqtt 
            client.loop_start()
            publish(client)
            client.loop_stop()

                
if __name__ == '__main__':
    main()
        

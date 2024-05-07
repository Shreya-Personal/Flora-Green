import serial
import time 
import json
import re
import ast

serialPort = serial.Serial(port = "/dev/ttyACM0", baudrate=115200,
                           bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)
serialString = ""                           # Used to hold data coming over UART

pos00 = 0
pos20 = 0
pos40 = 0
pos42 = 0 
pos44 = 0
pos24 = 0 
pos04 = 0
pos02 = 0

def extract_second_number(tup_str):
    try:
        # Remove any leading zeros from the tuple string before evaluation
        tup_str = tup_str.replace("(0", "(")
        tup = ast.literal_eval(tup_str)  # Convert the modified string to a tuple
        if isinstance(tup, tuple) and len(tup) > 1:
            return f"{tup[1]}"  # Return the second number as a string
        else:
            return None  # or any other value to indicate that the second element doesn't exist
    except (SyntaxError, ValueError) as e:
        print(f"Error: {e}")
        return None


def update_globals(input_str): 
    #pos 1 x 
    #pos 2 y 
    #pos 4 rssi value 
    # print("1st", input_str[1] , input_str[2], extract_second_number(input_str))
    if (input_str[1] == '0' and input_str[2] == '0'): 
        global pos00
        pos00 = extract_second_number(input_str)
    if (input_str[1] == '2' and input_str[2] == '0'): 
        global pos20
        pos20 = extract_second_number(input_str)
    if (input_str[1] == '4' and input_str[2] == '0'): 
        global pos40
        pos40 = extract_second_number(input_str)
    if (input_str[1] == '4' and input_str[2] == '2'): 
        global pos42
        pos42 = extract_second_number(input_str)
    if (input_str[1] == '4' and input_str[2] == '4'): 
        global pos44
        pos44 = extract_second_number(input_str)
    if (input_str[1] == '2' and input_str[2] == '4'):
        global pos24
        pos24 = extract_second_number(input_str)
    if (input_str[1] == '0' and input_str[2] == '4'): 
        global pos04
        pos04 = extract_second_number(input_str)
    if (input_str[1] == '0' and input_str[2] == '2'): 
        global pos02
        pos02 = extract_second_number(input_str)


def replace_quotes(input_string):
    return input_string.replace("'", '"')


##### NEEDS THIS LOOP TO UPDATE GLOBALS 
##serialPort.write(b"view -a\n")
while(1):

    # Wait until there is data waiting in the serial buffer
    if(serialPort.in_waiting > 0):

        
        # Read data out of the buffer until a carraige return / new line is found
        serialString = serialPort.readline()
        # Print the contents of the serial data
        buf = serialString.decode('Ascii')
        input_string = str(buf)
        json_string = 0
        
        if "JSON" in input_string:
            count = input_string.count("JSON")
            
            for i in range(count-1): 
                json_string = input_string.split("JSON")[i+1]
                update_globals(json_string)
                print(json_string)
        else: 
            #print(buf)
            time.sleep(0.5)
            
    





        

import os
import time
import urllib
import json
import httplib
import requests
 
key = '5JMJTTVV0C6TN36R'  # API from ThingSpeak.com

 
# get the data from arduino by using read001.py
port = serial.Serial("/dev/ttyACM0", baudrate=9600, timeout=None)

 
def send_data():
    while True:
        line = port.readline()
        arr = line.split()
        
        
        if arr[0] == 'IN':
            gps_data = "NO RESPONSE"
            vib_data = float(arr[1])

                

        elif arr[0] == 'OUT':
            gps_data = arr[1]
            vib_data = float(arr[2])

        else:
            continue


        try:
            print(" ")
            print "[+] ThingSpeak"
            conn.request("POST", "/update", params, headers)
            response = conn.getresponse()
            print vib_data
            print response.status, response.reason
            data = response.read()
            conn.close()
 
            # sending data to Node.js, ip by using ifconfig
            print "[+] Node.js"
            payload = {'data':vib_data, 'gps':gps_data}
            r = requests.get('http://localhost:3000/logone', params=payload)
            print r.status_code
 
        except:
            print "connection failed"
        break
 
 
if __name__ == "__main__":

    time.sleep(10)
    
    while True:
        send_data()

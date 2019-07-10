import os
import time
import serial
import urllib
import json
import httplib
import requests
 
key = 'VHFU09Y9N4I94H3A'  # API from ThingSpeak.com

 
# get the data from arduino by using read001.py
port = serial.Serial("/dev/ttyACM0", baudrate=9600, timeout=None)

 
def send_data():
    while True:

        line = port.readline()
        arr = line.split()
        
        if 'IN' == arr[0]:
            gps_data = "UNKNOWN"
            vib_data = float(arr[1])
            
            headers = {"Content-typZZe": "application/x-www-form-urlencoded","Accept": "text/plain"}
            params = urllib.urlencode({'field1': vib_data, 'key':key })
            conn = httplib.HTTPConnection("api.thingspeak.com:80")

                

        elif 'OUT' == arr[0]:
            gps_data = '"' + arr[1] + '"'
            vib_data = float(arr[2])
            
            headers = {"Content-typZZe": "application/x-www-form-urlencoded","Accept": "text/plain"}
            params = urllib.urlencode({'field1': vib_data, 'key':key })
            conn = httplib.HTTPConnection("api.thingspeak.com:80")
            
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


if __name__ == "__main__" :
    #time.sleep(30)
    
    while True:
        send_data()


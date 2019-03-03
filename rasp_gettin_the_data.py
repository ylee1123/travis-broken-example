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
        time.sleep(10)
        line = port.readline()
        arr = line.split()

        headers = {"Content-typZZe": "application/x-www-form-urlencoded","Accept": "text/plain"}
        params = urllib.urlencode({'field1': temp, 'key':key })
        conn = httplib.HTTPConnection("api.thingspeak.com:80")
        
        if 'inside' in arr:
            gps_lat = 0.0
            gps_lon = 0.0
            vib_data = float(arr[2])

                

        if 'outside' in arr:
            gps_lat = float(arr[2])
            gps_lon = float(arr[3])
            vib_data = float(arr[5])


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
            r = requests.get('http://localhost:3000/logone', params={'data':vib_data, 'lat':gps_lat, 'lon':gps_lon})
            print r.status_code
 
        except:
            print "connection failed"
        break
 
 
if __name__ == "__main__":
    
    while True:
        send_data()
        time.sleep(5)

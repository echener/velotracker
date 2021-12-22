import paho.mqtt.client as mqtt
import mysql.connector
import datetime
import json

broker_url = "localhost"
broker_port = 1883

mydb = mysql.connector.Connect(
        host="",
        user="",
        password="",
        database=""
)

def on_connect(client, userdata, flags, rc):
        print("Connected With Result Code "+ str(rc))

def on_message(client, userdata, message):
        jsonData  =json.loads(message.payload.decode())
        print("Tracker: " + jsonData["trck"])
        print("latitude: " + str(jsonData["lat"]))
        print("longitude: " + str(jsonData["lng"]))

        timestamp = '{:%Y-%m-%d %H:%M:%S}'.format(datetime.datetime.now())
        sql = "INSERT INTO " + jsonData["trck"] + " VALUES (\"" + timestamp + "\", " + str(jsonData["lat"]) +", "+ str(jsonData["lng"]) + ")"
        print(sql)
        mydb.cursor().execute(sql)
        mydb.commit()
        print(jsonData)
        file = open("/home/pi/volkszaehler.org/htdocs/velotracker/location.json", "w")
        file.write(message.payload.decode())
        file.close

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(broker_url, broker_port)

print("connected")

client.subscribe("velotracker/tracker", qos=0)

client.publish(topic="velotracker/test", payload="TestingPayload", qos=1, retain=False)

client.loop_forever()

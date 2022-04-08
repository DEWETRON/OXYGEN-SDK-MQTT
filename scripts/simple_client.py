import paho.mqtt.client as mqtt
import time
import json
import math

client = mqtt.Client()
client.connect("127.0.0.1")
start = time.time()

while True:
    t = time.time() - start

    payload = {
        "property-1": math.sin(2*math.pi*1*t),
        "property-2": {
            "nested-1": {
                "nested-2": math.sin(2*math.pi*2*t)
            }
        }
    }

    client.publish("/my/topic/as/json", json.dumps(payload))
    time.sleep(0.1)

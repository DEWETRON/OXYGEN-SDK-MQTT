import paho.mqtt.client as mqtt
import time
import numpy as np
from cbor2 import dumps
from numpy import random
import pygame

pygame.init()
TIMER_EVENT = pygame.USEREVENT + 1
pygame.time.set_timer(TIMER_EVENT, 100)

# Client
client = mqtt.Client()
client.connect("127.0.0.1")

# Samples
t = np.linspace(0.0, 1, num=500)
sin = 1*np.sin(2*np.pi*5*t)

packet_idx = 0

# Time
timestamp = time.time()

def sample():
    global timestamp, packet_idx, t, sin, client

    timestamp = time.time()

    # Sampling with 500 Hz - every 100 mS 50 Samples are send to the broker
    idx = 50*packet_idx
    data = sin[idx:idx + 50]
    print(f"Time: {timestamp}, {idx}:{idx + 50}, {len(data)}")
    
    payload_sin = {
        "timestamp": timestamp,
        "data": data.tolist()
    }

    packet_idx = (packet_idx + 1) % 10
    client.publish("/cbor/sin/500", dumps(payload_sin), 2)
    client.loop()

while True:
    event = pygame.event.wait()
    if event.type == TIMER_EVENT:
        sample()
    elif event.type == pygame.QUIT:
        break

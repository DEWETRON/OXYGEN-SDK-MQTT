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
t = np.linspace(0.0, 1, num=10000)
cos = 1*np.cos(2*np.pi*3*t) + 1*np.cos(2*np.pi*8*t)

packet_idx = 0

# Time
timestamp = time.time()

def sample():
    global timestamp, packet_idx, t, cos, client

    timestamp = time.time()
    
    # Sampling with 10 kHz - every 100 mS 1000 Samples are send to the broker
    idx = 1000*packet_idx
    data = cos[idx:idx + 1000]
    print(f"Time: {timestamp}, {idx}:{idx + 1000}, {len(data)}")
    
    payload_cos = {
        "timestamp": timestamp,
        "data": data.tolist()
    }

    packet_idx = (packet_idx + 1) % 10
    client.publish("/cbor/cos", dumps(payload_cos), 2)
    client.loop()

while True:
    event = pygame.event.wait()
    if event.type == TIMER_EVENT:
        sample()
    elif event.type == pygame.QUIT:
        break

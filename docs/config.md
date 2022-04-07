# Configuring the Plugin

When you add the Plugin to your OXYGEN setup, you must select a configuration file. Currently, there is no graphical user interface to create the configuration. However, this documentation includes several examples for the most common usecases.

The configuration is given as a JSON document of the following form:

```json
{
    "version": "0.1",
    "servers": [
        ...
    ],
    "topics": {
        ...
    }
}
```

The `version` tag is a constant and must be given. The config-file version must be supported by the plugin release. Within the document, you must specify at least one server (future versions of the plugin might support a backup strategy if a server is not reachable) and several topics.

## Topics
A server must contain the following properties:
```json
...
"servers": [
    {
        "description": "Local development Broker (Mosquitto)",
        "url": "127.0.0.1:1883"
    }
]
...
```

The `description` is optional, the `url` is a mandatory property.

## Topics
You can publish and subscribe to several topics using the plugin.

### Subscribe
You can subscribe to an arbitrary number of Topics using the plugin. Each topic specified will result in one or more OXYGEN channels, depending on the payload decoder.

```json
...
"/my/topic": {
    "QoS": 0,
    "subscribe": {
        "sampling" {
            "type": "async",
        },
        "payload": {
            "{decoder}": {
                ...
            }
        }
    }
}
...
```

The `QoS` property specifies the quality of service used for this subscription. 

The `sampling` property specifies how to deal with incoming payload. Normally, you will use `async` as the sampling mode as this will simply add a datapoint to OXYGEN whenever a packet arrives. The `sync` mode requires a more advanced [protocol](cbor_sync_decoder.md) between the sender and the plugin. 

The `payload` property specifies the payload decoder.

For details about the decoders, refer to:
- [JSON Payload](json_decoder.md)
- [Plain Text Payload](text_plain_decoder.md)
- [The CBOR-SYNC Protocol](cbor_sync_decoder.md)

To get started quickly, have a look at the following examples.

## Example: Subscribe to a plain-text payload in async sampling mode
```json
{
    "version": "0.1",
    "servers": [
        {
            "description": "Local development Broker (Mosquitto)",
            "url": "127.0.0.1:1883"
        }
    ],
    "topics": {
        "/my/topic/as/text": {
            "description": "Subscribing to an example Topic providing a schema for ASCII Text.",
            "subscribe": {
                "sampling": {
                    "type": "async"
                },
                "payload": {
                    "text/plain": {
                        "schema": {
                            "type": "number",
                            "range": {
                                "min": -1,
                                "max": 1
                            }
                        },
                        "example": 1.26
                    }
                }
            }
        }
    }
}
```

The selected decoder is `text/plain`, hence the payload must be an ASCII string which is interpreted corresponding to the selected `type`:
- `number` is interpreted as datatype double
- `integer` is interpreted as a signed integer
- `string` is interpreted as an ASCII string (e.g. debug messages)

the `range` property specifies expected minimum and maximum values of the channel.
This topic will result in a single OXYGEN channel.

## Example: Subscribe to a JSON-Payload with multiple channels per message
```json
{
    "version": "0.1",
    "servers": [
        {
            "description": "Local development Broker (Mosquitto)",
            "url": "127.0.0.1:1883"
        }
    ],
    "topics": {
        "/my/topic/as/json": {
            "description": "Subscribing to an example Topic providing a schema for JSON.",
            "subscribe": {
                "sampling": {
                    "type": "async"
                },
                "payload": {
                    "text/json": {
                        "schema": {
                            "property-1": {
                                "type": "number",
                                "range": {
                                    "min": -1,
                                    "max": 1
                                }
                            },
                            "property-2": {
                                "type": "object",
                                "properties": {
                                    "nested-1": {
                                        "type": "object",
                                        "properties": {
                                            "nested-2": {
                                                "type": "number",
                                                "range": {
                                                    "min": -1,
                                                    "max": 1
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
```

This more complex example will interpret incoming payload as a JSON string. The given configuration will result in two OXYGEN channels. A payload example looks like the following:
```json
{
    "property-1": 1.27,
    "property-2": {
        "nested-1": {
            "nested-2": 1.27
        }
    }
}
```

The JSON paths `/property-1` and `/property-2/nested-1/nested-2` will be interpreted as OXYGEN channels with the corresponding datatype and range.

## Example: The CBOR-Sync Protocol
```json
{
    "version": "0.1",
    "servers": [
        {
            "description": "Local development Broker (Mosquitto)",
            "url": "127.0.0.1:1883"
        }
    ],
    "topics": {
        "/my/sync/cbor/channel": {
            "QoS": 2,
            "subscribe": {
                "sampling": {
                    "type": "sync",
                    "sample-rate": 10000.0,
                    "clock": "gPTP"
                },
                "payload": {
                    "cbor/json/sync": {
                        "schema": {
                            "type": "number",
                            "range": {
                                "min": -2.5,
                                "max": 2.5
                            }
                        }
                    }
                }
            }
        }
    }
}
```

The following parameters have been added for the sync channel:
- `sample-rate` specifies the default sampling rate of the incoming datastream
- `clock` specifies a clock domain if several producers share a common clock

(more details can be found [here](cbor_sync_decoder.md))

The CBOR-Sync protocol allows to send e.g. constantly sampled analog to digital converters from a Microcontroller to OXYGEN. The plugin will take care to align the incoming sample stream with the OXYGEN time by resampling the channels. The protocol is simple and efficient and encoded using CBOR. 

The CBOR-Sync protocol further allows to send data from multiple independent nodes (e.g. multiple microcontrollers) by introducing a shared clock. A typical example would be using gPTP to syncronize multiple node to a common global time. The plugin will then align the incoming streams with each other.

The corresponding payload in JSON is given as follows:

```json
{
    "timestamp": 0.25,
    "data": [1, 2, 3, 4...]
}
```

Where `timestamp` corresponds to a monotonic clock domain and data is an array of samples. At runtime, the number of samples per MQTT message must not change. The number of samples furthermore most correspond `sample-rate`. The actual sample rate can deviate from the OXYGEN base frequency, hence the incoming samples are resampled to ensure no time drift between the OXYGEN time and the node time is given.

## Example: Subscribe to simple debug messages 

```json
{
    "version": "0.1",
    "servers": [
        {
            "description": "Local development Broker (Mosquitto)",
            "url": "127.0.0.1:1883"
        }
    ],
    "topics": {
        "/debug": {
            "description": "Subscribe to a Text-Message.",
            "subscribe": {
                "sampling": {
                    "type": "async"
                },
                "payload": {
                    "text/plain": {
                        "schema": {
                            "type": "string"
                        }
                    }
                }
            }
        }
    }
}
```

Every plain text (e.g. `Hello World`) send on the topic `/debug` will end up as a simple text-message in OXYGEN.



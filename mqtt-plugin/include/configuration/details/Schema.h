#pragma once
#include <nlohmann/json.hpp>

namespace plugin::mqtt::config::details
{
/**
 * @brief The schema used to validate JSON configuration-files
 */
inline nlohmann::json configuration_file_schema = R"(
{
    "$schema": "http://json-schema.org/draft-04/schema#",
    "title": "Oxygen-MQTT Configuration",
    "description": "The configuration for Oxygen MQTT plugin.",
    "type": "object",
    "properties": {
        "version": {
            "type": "string",
            "description": "The version of the config-file-schema. Must be supported by the plugin-release.",
            "enum": [
                "0.1"
            ]
        },
        "servers": {
            "description": "Servers the plugin will try to connect to in given order.",
            "type": "array",
            "minItems": 1,
            "uniqueItems": true,
            "items": {
                "type": "object",
                "properties": {
                    "url": {
                        "type": "string",
                        "description": "The Server-URL"
                    }
                },
                "required": [
                    "url"
                ]
            }
        },
        "topics": {
            "$ref": "#/definitions/topics"
        }
    },
    "required": [
        "servers",
        "topics",
        "version"
    ],
    "definitions": {
        "topics": {
            "type": "object",
            "patternProperties": {
                "^/": {
                    "$ref": "#/definitions/topic-item"
                }
            }
        },
        "topic-item": {
            "type": "object",
            "properties": {
                "publish": {
                    "$ref": "#/definitions/operation-publish"
                },
                "subscribe": {
                    "$ref": "#/definitions/operation-subscribe"
                },
                "QoS": {
                    "description": "The MQTT Quality of Service for operations done on this topic",
                    "type": "integer"
                }
            },
            "oneOf": [
                {
                    "required": [
                        "publish"
                    ]
                },
                {
                    "required": [
                        "subscribe"
                    ]
                }
            ]
        },
        "operation-publish": {
            "type": "object",
            "properties": {
                "sampling": {
                    "$ref": "#/definitions/sampling-publish"
                },
                "payload": {
                    "$ref": "#/definitions/payload-publish"
                }
            },
            "required": [
                "sampling",
                "payload"
            ]
        },
        "operation-subscribe": {
            "type": "object",
            "properties": {
                "sampling": {
                    "$ref": "#/definitions/sampling-subscribe"
                },
                "payload": {
                    "$ref": "#/definitions/payload-subscribe"
                }
            },
            "required": [
                "sampling",
                "payload"
            ]
        },
        "payload-publish": {
            "type": "object",
            "properties": {
                "type": {
                    "type": "string",
                    "description": "The datatype of the Oxygen-Channel.",
                    "enum": [
                        "number",
                        "integer",
                        "string"
                    ]
                },
                "samples-per-packet": {
                    "description": "The number of samples within every MQTT payload packet. Only used for sampling-mode sync.",
                    "type": "integer"
                }
            },
            "required": [
                "type"
            ]
        },
        "payload-subscribe": {
            "type": "object",
            "properties": {
                "text/json": {
                    "$ref": "#/definitions/json-subscribe"
                },
                "text/plain": {
                    "$ref": "#/definitions/text-subscribe"
                },
                "cbor/json/sync": {
                    "$ref": "#/definitions/cbor-json-sync-subscribe"
                }
            },
            "oneOf": [
                {
                    "required": [
                        "text/json"
                    ]
                },
                {
                    "required": [
                        "text/plain"
                    ]
                },
                {
                    "required": [
                        "cbor/json/sync"
                    ]
                }
            ]
        },
        "text-subscribe": {
            "description": "Interpreting payload as plain text. E.g.: my/mqtt/topic/1.25",
            "type": "object",
            "properties": {
                "schema": {
                    "description": "The schema of the payload",
                    "type": "object",
                    "properties": {
                        "type": {
                            "description": "The datatype of the payload: how payload as ASCII should be interpreted by the plugin.",
                            "type": "string",
                            "enum": [
                                "number",
                                "integer",
                                "string"
                            ]
                        },
                        "range": {
                            "$ref": "#/definitions/range"
                        }
                    }
                }
            },
            "required": [
                "schema"
            ]
        },
        "cbor-json-sync-subscribe": {
            "description": "Interpreting payload as CBOR JSON complying to sync-protocol. TODO: Write specification.",
            "type": "object",
            "properties": {
                "schema": {
                    "type": "object",
                    "properties": {
                        "type": {
                            "description": "Datatype used within the cbor-sync protocol.",
                            "type": "string",
                            "enum": [
                                "number",
                                "integer"
                            ]
                        },
                        "range": {
                            "$ref": "#/definitions/range"
                        }
                    }
                }
            },
            "required": [
                "schema"
            ]
        },
        "json-subscribe": {
            "description": "Interpreting payload as JSON-Data. Every property in schema becomes a channel in Oxygen grouped by the Topic.",
            "type": "object",
            "properties": {
                "schema": {
                    "$ref": "#/definitions/json-type"
                }
            },
            "required": [
                "schema"
            ]
        },
        "json-type": {
            "patternProperties": {
                "^[A-Za-z][A-Za-z0-9_-]*$": {
                    "$ref": "#/definitions/json-schema"
                }
            }
        },
        "json-schema": {
            "type": "object",
            "properties": {
                "type": {
                    "type": "string",
                    "description": "If type is object, then JSON-Payload is nested, else this will become a channel in Oxygen.",
                    "enum": [
                        "object",
                        "integer",
                        "number"
                    ]
                },
                "range": {
                    "$ref": "#/definitions/range"
                },
                "properties": {
                    "patternProperties": {
                        "^[A-Za-z][A-Za-z0-9_-]*$": {
                            "$ref": "#/definitions/json-schema"
                        }
                    }
                }
            },
            "anyOf": [
                {
                    "properties": {
                        "type": {
                            "enum": [
                                "object"
                            ]
                        }
                    },
                    "required": [
                        "type",
                        "properties"
                    ]
                },
                {
                    "properties": {
                        "type": {
                            "enum": [
                                "integer",
                                "number",
                                "string"
                            ]
                        }
                    },
                    "required": [
                        "type"
                    ]
                }
            ]
        },
        "sampling-subscribe": {
            "type": "object",
            "properties": {
                "type": {
                    "type": "string",
                    "description": "The sampling mode of the subscription - corresponds to the underlying payload protocol",
                    "enum": [
                        "async",
                        "sync"
                    ]
                },
                "sample-rate": {
                    "description": "Sampling rate for sync-channels - corresponds to the underlying payload protocol.",
                    "type": "number"
                },
                "clock": {
                    "description": "Synced channels can share a common clock domain (depending on the procol). This domains are identified by their clock name",
                    "type": "string"
                }
            },
            "required": [
                "type"
            ],
            "oneOf": [
                {
                    "properties": {
                        "type": {
                            "type": "string",
                            "enum": [
                                "async"
                            ]
                        }
                    }
                },
                {
                    "properties": {
                        "type": {
                            "type": "string",
                            "enum": [
                                "sync"
                            ]
                        }
                    },
                    "required": [
                        "sample-rate"
                    ]
                }
            ]
        },
        "sampling-publish": {
            "type": "object",
            "properties": {
                "type": {
                    "description": "The sampling mode of the Oxygen-Channel to publish",
                    "type": "string",
                    "enum": [
                        "sync",
                        "async"
                    ]
                },
                "downsampling-factor": {
                    "description": "Downsample Oxygen-Sync channels by given factor.",
                    "type": "integer"
                }
            },
            "required": [
                "type"
            ]
        },
        "range": {
            "type": "object",
            "properties": {
                "min": {
                    "description": "Minimum value of range",
                    "type": "number"
                },
                "max": {
                    "description": "Maximum value of range",
                    "type": "number"
                },
                "unit": {
                    "description": "Unit of range",
                    "type": "string"
                }
            },
            "required": [
                "min",
                "max"
            ]
        }
    }
}
)"_json;
}

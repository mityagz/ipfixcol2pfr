const jsonSchemaUDP = {
    $schema: "http://json-schema.org/draft-07/schema#",
    title: "UDP input",
    desription: "UDP input plugin",
    type: "object",
    properties: {
        name: {
            type: "string",
            default: "UDP input"
        },
        plugin: {
            type: "string",
            const: "udp"
        },
        params: {
            type: "object",
            properties: {
                localPort: {
                    type: "integer",
                    minimum: 0,
                    default: 4739
                },
                localIPAddress: {
                    type: "string",
                    default: ""
                },
                connectionTimeout: {
                    type: "integer",
                    default: 600
                },
                templateLifeTime: {
                    type: "integer",
                    default: 1800
                },
                optionsTemplateLifeTime: {
                    type: "integer",
                    default: 1800
                }
            },
            required: ["localPort", "localIPAddress"]
        }
    },
    required: ["name", "plugin", "params"]
};
const jsonSchemaTCP = {
    $schema: "http://json-schema.org/draft-07/schema#",
    title: "TCP input",
    desription: "TCP input plugin",
    type: "object",
    properties: {
        name: {
            type: "string",
            default: "TCP input"
        },
        plugin: {
            type: "string",
            const: "tcp"
        },
        params: {
            type: "object",
            properties: {
                localPort: {
                    type: "integer",
                    minimum: 0,
                    default: 4739
                },
                localIPAddress: {
                    type: "string",
                    default: ""
                }
            },
            required: ["localPort", "localIPAddress"]
        }
    },
    required: ["name", "plugin", "params"]
};
const jsonSchemaAnonymization = {
    $schema: "http://json-schema.org/draft-07/schema#",
    title: "Flow anonymization",
    desription: "Flow anonymization plugin",
    type: "object",
    properties: {
        name: {
            type: "string",
            default: "Flow anonymization"
        },
        plugin: {
            type: "string",
            const: "anonymization"
        },
        params: {
            type: "object",
            properties: {
                type: {
                    type: "string",
                    enum: ["CryptoPAn", "Truncation"]
                },
                key: {
                    type: "string",
                    default: ""
                }
            },
            required: ["type", "key"]
        }
    },
    required: ["name", "plugin", "params"]
};
const jsonSchemaJSON = {
    $schema: "http://json-schema.org/draft-07/schema#",
    title: "JSON output",
    desription: "JSON output plugin",
    type: "object",
    properties: {
        name: {
            type: "string",
            default: "JSON output"
        },
        plugin: {
            type: "string",
            const: "json"
        },
        params: {
            type: "object",
            properties: {
                tcpFlags: {
                    type: "string",
                    default: "formatted",
                    enum: ["formatted", "raw"]
                },
                timestamp: {
                    type: "string",
                    default: "formatted",
                    enum: ["formatted", "unix"]
                },
                protocol: {
                    type: "string",
                    default: "formatted",
                    enum: ["formatted", "raw"]
                },
                ignoreUnknown: {
                    type: "boolean",
                    default: true,
                    enum: [true, false]
                },
                ignoreOptions: {
                    type: "boolean",
                    default: true,
                    enum: [true, false]
                },
                nonPrintableChar: {
                    type: "boolean",
                    default: true,
                    enum: [true, false]
                },
                outputs: {
                    type: "object",
                    properties: {
                        server: {
                            type: "array",
                            minItems: 1,
                            items: {
                                type: "object",
                                properties: {
                                    name: {
                                        type: "string"
                                    },
                                    port: {
                                        type: "integer"
                                    },
                                    blocking: {
                                        type: "boolean",
                                        enum: [true, false]
                                    }
                                },
                                required: ["name", "port", "blocking"]
                            }
                        },
                        send: {
                            type: "array",
                            minItems: 1,
                            items: {
                                type: "object",
                                properties: {
                                    name: {
                                        type: "string"
                                    },
                                    ip: {
                                        type: "string"
                                    },
                                    port: {
                                        type: "integer"
                                    },
                                    protocol: {
                                        type: "string",
                                        enum: ["TCP", "UDP"]
                                    },
                                    blocking: {
                                        type: "boolean",
                                        enum: [true, false]
                                    }
                                },
                                required: ["name", "ip", "port", "protocol", "blocking"]
                            }
                        },
                        file: {
                            type: "array",
                            minItems: 1,
                            items: {
                                type: "object",
                                properties: {
                                    name: {
                                        type: "string"
                                    },
                                    path: {
                                        type: "string"
                                    },
                                    prefix: {
                                        type: "string"
                                    },
                                    timeWindow: {
                                        type: "integer",
                                        default: 300,
                                        minimum: 60
                                    },
                                    timeAlignment: {
                                        type: "string",
                                        enum: ["yes", "no"]
                                    }
                                },
                                required: ["name", "path", "prefix", "timeWindow", "timeAlignment"]
                            }
                        },
                        print: {
                            type: "array",
                            minItems: 1,
                            maxItems: 1,
                            items: {
                                type: "object",
                                properties: {
                                    name: {
                                        type: "string"
                                    }
                                },
                                required: ["name"]
                            }
                        }
                    },
                    minProperties: 1
                }
            },
            required: [
                "tcpFlags",
                "timestamp",
                "protocol",
                "ignoreUnknown",
                "ignoreOptions",
                "nonPrintableChar",
                "outputs"
            ]
        }
    },
    required: ["name", "plugin", "params"]
};
const jsonSchemaDummy = {
    $schema: "http://json-schema.org/draft-07/schema#",
    title: "Dummy output",
    desription: "Dummy output plugin",
    type: "object",
    properties: {
        name: {
            type: "string",
            default: "Dummy output"
        },
        plugin: {
            type: "string",
            const: "dummy"
        },
        params: {
            type: "object",
            properties: {
                delay: {
                    type: "integer",
                    minimum: 0
                }
            },
            required: ["delay"]
        }
    },
    required: ["name", "plugin", "params"]
};
const jsonSchemaLNF = {
    $schema: "http://json-schema.org/draft-07/schema#",
    title: "LNF storage",
    desription: "LNF storage plugin",
    type: "object",
    properties: {
        name: {
            type: "string",
            default: "LNF storage"
        },
        plugin: {
            type: "lnfstore",
            const: "anonymization"
        },
        params: {
            type: "object",
            properties: {
                storagePath: {
                    type: "string"
                },
                compress: {
                    type: "string",
                    default: "no",
                    enum: ["yes", "no"]
                },
                identificatorField: {
                    type: "string",
                    default: ""
                },
                dumpInterval: {
                    type: "object",
                    properties: {
                        timeWindow: {
                            type: "integer",
                            default: 300,
                            minimum: 0
                        },
                        align: {
                            type: "string",
                            default: "yes",
                            enum: ["yes", "no"]
                        }
                    },
                    required: ["timeWindow", "align"]
                },
                index: {
                    type: "object",
                    properties: {
                        enable: {
                            type: "string",
                            default: "no",
                            enum: ["yes", "no"]
                        },
                        autosize: {
                            type: "string",
                            default: "yes",
                            enum: ["yes", "no"]
                        },
                        estimatedItemCount: {
                            type: "integer",
                            default: 100000,
                            minimum: 0
                        },
                        falsePositiveProbability: {
                            type: "number",
                            default: 0.01
                        }
                    },
                    required: ["enable", "autosize"]
                }
            },
            required: ["storagePath", "compress", "dumpInterval", "index"]
        }
    },
    required: ["name", "plugin", "params"]
};

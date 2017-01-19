import requests
import socket
import threading
import thread
import logging
import time
# import paho.mqtt.client as mqtt
import client as mqtt
import json
import urllib2

GIOT_ULTopic_prefix = "GIOT-GW/UL/"
GIOT_DLTopic_prefix = "GIOT-GW/DL/"
LAN_MAC = "1C497B498D80"
LoRa_Wan_MAC = "00001c497b48db92"
Target_node_MAC = "000000000500001e"


mcs_data_format = {
    "datapoints": [
        {
            "dataChnId": "Temperature",
            "values": {
                "value": "20.00"
            }
        },
        {
            "dataChnId": "Humidity",
            "values": {
                "value": "30.00"
            }
        }
    ]
}

downlink_data = [{
    "macAddr": "000000000500001e",
    "data": "0000",
    "id": "998877abcd0123",
    "extra": {
        "port": 2,
        "txpara": 2
    }
}]

# change this to the values from MCS web console
DEVICE_INFO = {
    'device_id': 'DBKHFNIw',
    'device_key': '8fszdReA51m0vRjq'
}

# change 'INFO' to 'WARNING' to filter info messages
logging.basicConfig(level='INFO')

heartBeatTask = None
dlmsg = 0
device_status = 0

def establishCommandChannel():
    # Query command server's IP & port
    connectionAPI = 'https://api.mediatek.com/mcs/v2/devices/%(device_id)s/connections.csv'
    r = requests.get(connectionAPI % DEVICE_INFO,
                 headers = {'deviceKey' : DEVICE_INFO['device_key'],
                            'Content-Type' : 'text/csv'})
    logging.info("Command Channel IP,port=" + r.text)
    (ip, port) = r.text.split(',')

    # Connect to command server
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.settimeout(None)

    # Heartbeat for command server to keep the channel alive
    def sendHeartBeat(commandChannel):
        keepAliveMessage = '%(device_id)s,%(device_key)s,0' % DEVICE_INFO
        commandChannel.sendall(keepAliveMessage)
        logging.info("beat:%s" % keepAliveMessage)
        # check the value - it's either 0 or 1

    def heartBeat(commandChannel):
        sendHeartBeat(commandChannel)
        # Re-start the timer periodically
        global heartBeatTask
        heartBeatTask = threading.Timer(40, heartBeat, [commandChannel]).start()

    heartBeat(s)
    return s

def waitAndExecuteCommand(commandChannel):
    global dlmsg
    global device_status
    while True:
        command = commandChannel.recv(1024)
        logging.info("recv:" + command)
        # command can be a response of heart beat or an update of the LED_control,
        # so we split by ',' and drop device id and device key and check length
        fields = command.split(',')[2:]

        # if len(fields) > 1:
        #     timeStamp, dataChannelId, commandString = fields
        #     if dataChannelId == 'lora_led':
        #         # check the value - it's either 0 or 1
        #         commandValue = int(commandString)
        #         logging.info("led :%d" % commandValue)
        #         setLED(commandValue)
        #     elif dataChannelId == 'lora_fan':
        #         # check the value - it's either 0 or 1
        #         commandValue = int(commandString)
        #         logging.info("led :%d" % commandValue)
        #         setFAN(commandValue)

        if len(fields) > 1:
            timeStamp, dataChannelId, commandString = fields
            if dataChannelId == 'lora_led':
                # check the value - it's either 0 or 1
                # fan direct to binary 01
                # (device_status & 2) for keeping original status
                device_status = int(commandString)| (device_status & 2)
                logging.info("led :%d" % device_status)
                dlmsg = 1
                downlink_data[0]['data'] = str(device_status) + '000'
                # print downlink_data
            elif dataChannelId == 'lora_fan':
                # check the value - it's either 0 or 1
                # fan direct to binary 10
                # (device_status & 1) for keeping original status
                device_status = (int(commandString) << 1) | (device_status & 1)
                logging.info("led :%d" % device_status)
                dlmsg = 1
                downlink_data[0]['data'] = str(device_status) + '000'

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(GIOT_ULTopic_prefix + LAN_MAC)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global dlmsg
    print dlmsg

    json_extractor = json.loads(msg.payload)

    if dlmsg == 1 and (json_extractor[0]['macAddr'] == Target_node_MAC):
        downlink_data[0]['macAddr'] = Target_node_MAC
        downlink_data[0]['id'] = str(int(time.time()))
        client.publish(GIOT_DLTopic_prefix + LoRa_Wan_MAC, payload=json.dumps(downlink_data), qos=0, retain=False)
        dlmsg = 0
    # print(msg.topic+" "+str(msg.payload))

    # print(json_extractor[0]['channel'])
    # print(json_extractor[0]['macAddr'])
    # print(json_extractor[0]['data'].decode("hex"))

    if json_extractor[0]['macAddr'] == Target_node_MAC:
        string_value = json_extractor[0]['data'].decode("hex")
        #print(string_value[1:6])
        #print(string_value[6:11])
        mcs_data_format['datapoints'][0]['values']['value'] = string_value[1:6]
        mcs_data_format['datapoints'][1]['values']['value'] = string_value[6:11]
        #print(mcs_data_format)
        req = urllib2.Request('http://api.mediatek.com/mcs/v2/devices/' + DEVICE_INFO['device_id'] + '/datapoints')
        req.add_header('deviceKey', DEVICE_INFO['device_key'])
        req.add_header('Content-Type', 'application/json')

        response = urllib2.urlopen(req, json.dumps(mcs_data_format))
        print(response)

client = mqtt.Client(client_id="0f858ec97dfc4d208bb07c3919446562", protocol=mqtt.MQTTv31)
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set("lazyengineers", password="lazyengineers")
client.connect("104.199.215.165", port=1883, keepalive=60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.

if __name__ == '__main__':
    channel = establishCommandChannel()
    thread.start_new_thread(waitAndExecuteCommand, (channel,))
    client.loop_forever()
    while (True):
        pass
    # t = threading.Thread(target=waitAndExecuteCommand, args = (channel,))
    # t.daemon = True
    # t.start()
    # client.loop_forever()

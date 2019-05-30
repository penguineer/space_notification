#!/usr/bin/python3
import json
import time
import paho.mqtt.client as mqtt
import argparse
import os

parser = argparse.ArgumentParser(description='create SpaceAPI JSON based on MQTT data')
parser.add_argument('--server', type=str, default='mqtt.n39.eu', help='address of the MQTT server')
parser.add_argument('--template', type=argparse.FileType('r'), default='template.json', help='template of the JSON')
parser.add_argument('--out', type=argparse.FileType('w'), default='www/spaceapi.json', help='output file location')
parser.add_argument('--open-image', type=str, default='www/open.png', help='image for open state')
parser.add_argument('--closed-image', type=str, default='www/closed.png', help='image for closed state')
parser.add_argument('--symlink-location', type=str, default='www/state.png',
                    help='where the symlink to the state image should be created')

args = parser.parse_args()
spaceapi_dict = json.loads(args.template.read())


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print('Connected with result code ' + str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe('/Netz39/Things/Door/Events')
    client.subscribe('/Netz39/Things/StatusSwitch/Lever/State')


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(
        'Got message! Topic=[%s] Payload=[%s]'
        % (msg.topic, str(msg.payload.decode('ASCII')))
    )
    current_time = int(time.time())

    if msg.topic == 'Netz39/Things/Door/Events':
        print('detected door event (yay!)')
        spaceapi_dict['ext_door']['lastchange'] = current_time

        if msg.payload == b'door open':
            print('\tdoor open')
            spaceapi_dict['ext_door']['open'] = True
        elif msg.payload == b'door closed':
            print('\tdoor closed')
            spaceapi_dict['ext_door']['open'] = False
        elif msg.payload == b'door locked':
            print('\tdoor locked')
            spaceapi_dict['ext_door']['locked'] = True
        elif msg.payload == b'door unlocked':
            print('\tdoor unlocked')
            spaceapi_dict['ext_door']['locked'] = False

    elif msg.topic == 'Netz39/Things/StatusSwitch/Lever/State':
        print('detected lever event (yay?)')
        spaceapi_dict['state']['lastchange'] = current_time

        if msg.payload == b'open':
            print('\tlever open')
            spaceapi_dict['state']['open'] = True
        elif msg.payload == b'closed':
            print('\tlever closed')
            spaceapi_dict['state']['open'] = False

    # build strings to publish
    json_str = json.dumps(spaceapi_dict, indent=2)
    isOpen_str = 'true' if spaceapi_dict['state']['open'] else 'false'
    timestamp_str = str(current_time)

    args.out.seek(0)
    args.out.truncate(0)
    args.out.write(json_str)
    args.out.flush()

    # update symlink
    pic = args.open_image if spaceapi_dict['state']['open'] else args.closed_image
    os.remove(args.symlink_location)
    os.symlink(os.path.abspath(pic), os.path.abspath(args.symlink_location))

    # publish to SpaceAPI topics
    client.publish(topic='/Netz39/SpaceAPI/json', payload=json_str, qos=2, retain=True)
    client.publish(topic='/Netz39/SpaceAPI/isOpen', payload=isOpen_str, qos=2, retain=True)
    client.publish(topic='/Netz39/SpaceAPI/lastchange', payload=timestamp_str, qos=2, retain=True)


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(args.server, 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()


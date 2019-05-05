#!/usr/bin/python3
import json
import time
import paho.mqtt.client as mqtt

spaceapi_dict = {
    'api': '0.13',
    'space': 'Netz39',
    'logo': 'http://www.netz39.de/wiki/_media/resources:public_relations:logo:netz39_logo_2013-07-11.png',
    'url': 'http://www.netz39.de',
    'location': {
        'address': 'Leibnizstr. 32, 39104 Magdeburg, Germany',
        'lat': 52.119561,
        'lon': 11.629398,
    },
    'state': {
        'open': True,
        'lastchange': int(time.time()),
        'trigger_person': '',
        'message': '',
        'icon': {
            'open': 'http://www.netz39.de/open.png',
            'closed': 'http://www.netz39.de/closed.png',
        },
    },
    'contact': {
        'email': 'kontakt@netz39.de',
        'twitter': '@Netz39',
        'ml': 'list@netz39.de',
        'irc': 'irc://freenode/#netz39',
    },
    'issue_report_channels': ['email', 'twitter', 'ml'],
    'ext_door': {'locked': True, 'open': True, 'lastchange': int(time.time())},
}


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

    # write json file
    with open('../spaceapi.json', 'w') as file:
        file.write(json_str)

    # publish to SpaceAPI topics
    client.publish(topic='/Netz39/SpaceAPI/json',payload=json_str, qos=2, retain=True)
    client.publish(topic='/Netz39/SpaceAPI/isOpen', payload=isOpen_str, qos=2, retain=True)
    client.publish(topic='/Netz39/SpaceAPI/lastchange', payload=timestamp_str, qos=2, retain=True)


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect('helium', 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()

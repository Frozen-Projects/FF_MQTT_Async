# FF_MQTT_Async
This is an Eclipse Paho C library based Async MQTT client (subscriber and publisher) plugin for Unreal Engine 5.

## THIRD PARTY LIBRARY
https://github.com/eclipse/paho.mqtt.c

## DEPENDENCIES
https://github.com/FF-Plugins-Active/FF_MQTT_Sync

## USAGE HINTS
* I assume that you already know how to connect an MQTT broker.
* You can see a sample blueprint in ``Content/BPs`` folder. Just change connection informations based on your broker.
* You need to put on of these protocols before URL ``mqtt://``, ``mqtts://``, ``ws://``, ``wss://``, ``ssl://``
* You need to put port after the URL.
* Create your delegate events **before** using ``MQTT Async - Init``
* Connect your ``publish``, ``subscribe`` and ``unsubscribe`` functions **after** ``OnConnect`` delegate. Because ``MQTT Async - Init`` function's return value will only tell you that connection request has been successfully sent or not. It won't tell you connection has been created or not. ``OnConnect`` delegate will tell you that. So, you have do your data operations, after that delegate triggers.

## DOCUMENTS
https://blueprintue.com/blueprint/7i2hjgct/

## ROADMAP
* Android Platform support

## LICENSE
Our custom license allow you to use this plugin in your commercial projects freely, but you can't sell this plugin as a plugin with or without any modification on any marketplace.
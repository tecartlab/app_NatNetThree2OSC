NatNetThree2OSC
===================================


NatNetThree2OSC is a small app designed to convert the Optitrack NatNet 3.0 protocoll to Open Sound Control (OSC). It provides the following features:

+ connects ...
+ Sends an OSC Packet from NatNet Packet.
+ Receive OSC packets via UDP.

Download
--------



License
-------

NatNetThree2OSC is licensed under the MIT license.

See License.txt

Using The Application
-----------------

Usage: NatNetThree2OSC  <NatNetLocal IP (127.0.0.1)> <NatNetServer IP (127.0.0.1)> <OscSenIP (127.0.0.1)> <OscSendPort (54321)> <OscListeningPort (55555)> <verbose [0/1]>

upon stream the following messages are sent:

/frame/start <frameNumber>
/rigidbody \<rigidbodyID> tracked \<0/1>
/rigidbody <rigidbodyID> position <x> <y> <z>
/rigidbody <rigidbodyID> quat <x> <y> <z> <w>
/skeleton/bone <skleletonName> <boneID> position <x> <y> <z>
/skeleton/bone <skleletonName> <boneID> quat <x> <y> <z> <w>
/skeleton/joint <skleletonName> <boneID> quat <x> <y> <z> <w>
/frame/end <frameNumber>

sending commands to the <OscListeningPort> will pass commands to Motive:

the following commands are implemented:

/commands refetch

will return all rigidbodies and skeletons currently streaming

/markerset/id <markersetName>
/rigidbody/id <rigidbodyName> <rigidbodyID>
/skeleton/id <skleletonName> <SkeletonID>
/skeleton/id/bone <skleletonName> <boneID> <boneName>
/forceplate/id <serial>
/forceplate/id/channel <serial> <channelID> <channelName>

Building
---------

This code is based on the NatNet SDK from optitrack (http://optitrack.com/downloads/developer-tools.html) and includes the dll from SharpOSC (https://github.com/ValdemarOrn/SharpOSC).

Open the NatNetSamples inside the /Samples - folder and build NatNetThree2OSC Project

Contribute
----------

I would love to get some feedback. Use the Issue tracker on Github to send bug reports and feature requests, or just if you have something to say about the project. If you have code changes that you would like to have integrated into the main repository, send me a pull request or a patch. I will try my best to integrate them and make sure NatNetThree2OSC improves and matures.

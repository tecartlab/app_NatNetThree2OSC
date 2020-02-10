NatNetThree2OSC 5.0
===================================


NatNetThree2OSC is a small app designed to convert the Optitrack NatNet 3.0 protocoll to Open Sound Control (OSC). It provides the following features:

+ connects ...
+ Sends an OSC Packet from NatNet Packet.
+ Receive OSC packets via UDP.

Download
--------

https://github.com/tecartlab/app_NetNatThree2OSC/releases

License
-------

NatNetThree2OSC is licensed under the MIT license.

See License.txt

Using The Application
-----------------

Usage: NatNetThree2OSC  
* **--localIP**          Required. IP address of this machine.
* **--motiveIP**          Required. IP address of the machine Motive is running on.
* **--oscSendIP**         Required. IP address of the machine OSC data is sent to.
* **--mulitCastIP**       (Default: 239.255.42.99) Multicast IP Motive is sending on.
* **--motiveDataPort**    (Default: 1511) Motives data port
* **--motiveCmdPort**     (Default: 1510) Motives command port
* **--oscSendPort**       Required. listening port of the machine OSC data is sent to.
* **--oscCtrlPort**       (Default: 65111) listening port of this service to trigger Motive.
* **--oscMode**           (Default: max) OSC format (max, isadora, touch)
* **--yup2zup**           (Default: false) transform y-up to z-up
* **--verbose**          (Default: false) verbose mode
* **--help**              Display this help screen.
* **--version**           Display version information.

upon stream the following messages are sent depending on the OSC Mode

+ /frame/start \<frameNumber>

MAX/MSP: OSC MODE = max

+ /marker \<markerID> position \<x> \<y> \<z>

+ /rigidbody \<rigidbodyID> tracked \<0/1>
+ /rigidbody \<rigidbodyID> position \<x> \<y> \<z>
+ /rigidbody \<rigidbodyID> quat \<qx> \<qy> \<qz> \<qw>

+ /skeleton/bone \<skleletonName> \<boneID> position \<x> \<y> \<z>
+ /skeleton/bone \<skleletonName> \<boneID> quat \<qx> \<qy> \<qz> \<qw>
+ /skeleton/joint \<skleletonName> \<boneID> quat \<qx> \<qy> \<qz> \<qw>

ISADORA: OSC MODE = isadora

+ /marker/\<markerID>/position \<x> \<y> \<z>

+ /rigidbody/\<rigidbodyID>/tracked \<0/1>
+ /rigidbody/\<rigidbodyID>/position \<x> \<y> \<z>
+ /rigidbody/\<rigidbodyID>/quat \<qx> \<qy> \<qz> \<qw>

+ /skeleton/\<skleletonName>/bone/\<boneID>/position \<x> \<y> \<z>
+ /skeleton/\<skleletonName>/bone/\<boneID>/quat \<qx> \<qy> \<qz> \q<w>
+ /skeleton/\<skleletonName>/joint/\<boneID>/quat \<qx> \<qy> \<qz> \<qw>

TouchDesigner: OSC MODE = touch

+ /marker/\<markerID>/position \<x> \<y> \<z>

+ /rigidbody/\<rigidbodyID>/tracked \<0/1>
+ /rigidbody/\<rigidbodyID>/transformation \<x> \<y> \<z> \<qx> \<qy> \<qz> \<qw>

+ /skeleton/\<skleletonName>/bone/\<boneID>/transformation \<x> \<y> \<z> \<qx> \<qy> \<qz> \<qw>
+ /skeleton/\<skleletonName>/joint/\<boneID>/quat \<x> \<y> \<z> \<w>

IF you want to have multiple modes, set the oscmode like "max,isadora" or "isadora,touch" and make sure no space is between the values

At the end of the frame the frameend message is sent

+ /frame/end \<frameNumber>

sending commands to the <OscListeningPort> will pass commands to Motive:

the following commands are implemented:

+ /commands refetch

will return all rigidbodies and skeletons currently streaming

+ /rigidbody/id \<rigidbodyName> \<rigidbodyID>
+ /skeleton/id \<skleletonName> \<SkeletonID>
+ /skeleton/id/bone \<skleletonName> \<boneID> \<boneName>
+ /forceplate/id \<serial>
+ /forceplate/id/channel \<serial> \<channelID> \<channelName>

Building
---------

This code is based on the NatNet SDK from optitrack (http://optitrack.com/downloads/developer-tools.html) and includes the dll from SharpOSC (https://github.com/ValdemarOrn/SharpOSC).

Open the NatNetSamples inside the /Samples - folder and build NatNetThree2OSC Project

### Dependency

[CommandLine Parser Library](https://github.com/commandlineparser/commandline). How install the package got [here](https://github.com/commandlineparser/commandline/wiki/Getting-Started).

Contribute
----------

I would love to get some feedback. Use the Issue tracker on Github to send bug reports and feature requests, or just if you have something to say about the project. If you have code changes that you would like to have integrated into the main repository, send me a pull request or a patch. I will try my best to integrate them and make sure NatNetThree2OSC improves and matures.

NatNetThree2OSC 8.8.1
===================================


NatNetThree2OSC is a small app designed to convert the Optitrack NatNet 3.1 protocoll to Open Sound Control (OSC). It provides the following features:

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
* **--localIP**           Required. IP address of this machine.
* **--motiveIP**          Required. IP address of the machine Motive is running on.
* **--oscSendIP**         Required. IP-address or URL of the machine OSC data is sent to.
* **--oscSendPort**       Required. receiving port of the machine OSC data is sent to.
* **--oscCtrlPort**       (Default: 65111) local listening port to refetch descriptions.
* **--oscMode**           (Default: max) OSC format (max, isadora, touch, sparck, ambi)
* **--mulitCastIP**       (Default: 239.255.42.99) Multicast IP Motive is sending on.
* **--motiveDataPort**    (Default: 1511) Motives data port
* **--motiveCmdPort**     (Default: 1510) Motives command port
* **--frameModulo**       (Default: 1) Frame reduction: Send every n-th frame
* **--dataStreamInfo**    (Default: 0=off) sends each specified [ms] streaminfo message to the console.
* **--sendSkeletons**     (Default: false) send skeleton data
* **--sendMarkerInfo**    (Default: false) send marker info (position data)
* **--yup2zup**           (Default: false) transform y-up to z-up
* **--leftHanded**        (Default: false) transform right handed to left handed coordinate system
* **--matrix**            (Default: false) calculate and send the transformation matrix
* **--invMatrix**         (Default: false) calculate and send the inverse transformation matrix
* **--bundled**           (Default: false) send the frame message inside an osc bundle
* **--autoReconnect**     (Default: false) reconnect to motive when no data is received
* **--verbose**           (Default: false) verbose mode
* **--help**              Display this help screen.
* **--version**           Display version information.

For all the boolean flags, if you want to set them true you need simply add its name: --flagName. If you want to keep it false, don't add it to the command line.

### Streaming

when streaming, the following messages are sent at the beginning of each frame

\<timestamp> = \<milliSecondsSinceMotiveStarted(float)>

+ /frame/start \<frameNumber>
+ /frame/timestamp \<timestamp>
+ /frame/timecode \<smtp> \<subframe>

if OSC MODE = sparck

+ /f/s \<frameNumber>
+ /f/t \<timestamp>

upon streaming, the following messages are sent depending on the OSC Mode

**(!) - will only be sent if the CLI flags are set.**

#### MAX/MSP: OSC MODE = max

+ (!) /marker \<markerID> position \<x> \<y> \<z>
+ /rigidbody \<rigidbodyID> tracked \<0/1>
+ /rigidbody \<rigidbodyID> position \<x> \<y> \<z>
+ /rigidbody \<rigidbodyID> quat \<qx> \<qy> \<qz> \<qw>
+ (!) /rigidbody \<rigidbodyID> matrix \<m11> \<m12> \<m13> \<m14> \<m21> ... \<m44>
+ (!) /rigidbody \<rigidbodyID> invmatrix \<m11> \<m12> \<m13> \<m14> \<m21> ... \<m44>
+ (!) /skeleton/bone \<skleletonName> \<boneID> position \<x> \<y> \<z>
+ (!) /skeleton/bone \<skleletonName> \<boneID> quat \<qx> \<qy> \<qz> \<qw>
+ (!) /skeleton/joint \<skleletonName> \<boneID> quat \<qx> \<qy> \<qz> \<qw>

#### ISADORA: OSC MODE = isadora

+ (!) /marker/\<markerID>/position \<x> \<y> \<z>
+ /rigidbody/\<rigidbodyID>/tracked \<0/1>
+ /rigidbody/\<rigidbodyID>/position \<x> \<y> \<z>
+ /rigidbody/\<rigidbodyID>/quat \<qx> \<qy> \<qz> \<qw>
+ (!) /rigidbody/\<rigidbodyID>/matrix \<m11> \<m12> \<m13> \<m14> \<m21> ... \<m44>
+ (!) /rigidbody/\<rigidbodyID>/invmatrix \<m11> \<m12> \<m13> \<m14> \<m21> ... \<m44>
+ (!) /skeleton/\<skleletonName>/bone/\<boneID>/position \<x> \<y> \<z>
+ (!) /skeleton/\<skleletonName>/bone/\<boneID>/quat \<qx> \<qy> \<qz> \q<w>
+ (!) /skeleton/\<skleletonName>/joint/\<boneID>/quat \<qx> \<qy> \<qz> \<qw>

#### TouchDesigner: OSC MODE = touch

+ (!) /marker/\<markerID>/position \<x> \<y> \<z>
+ /rigidbody/\<rigidbodyID>/tracked \<0/1>
+ /rigidbody/\<rigidbodyID>/transformation \<x> \<y> \<z> \<qx> \<qy> \<qz> \<qw>
+ (!) /rigidbody/\<rigidbodyID>/matrix \<m11> \<m12> \<m13> \<m14> \<m21> ... \<m44>
+ (!) /rigidbody/\<rigidbodyID>/invmatrix \<m11> \<m12> \<m13> \<m14> \<m21> ... \<m44>
+ (!) /skeleton/\<skleletonName>/bone/\<boneID>/transformation \<x> \<y> \<z> \<qx> \<qy> \<qz> \<qw>
+ (!) /skeleton/\<skleletonName>/joint/\<boneID>/quat \<x> \<y> \<z> \<w>

#### Ambisonics: OSC MODE = ambi

+ /icst/ambi/source/xyz \<rigidbodyName> \<x> \<y> \<z>

in this mode **no** frame messages are sent.

#### SPARCK: OSC MODE = sparck

the addresses for sparck mode are structured in a way to process them quickly in sparck and are therefore not very human readable.

<datatype> specifies what kind of data follows
0 = tracked
1 = maker
2 = rigidbody
10 = skeleton

+ /rb \<rigidbodyID> <datatype = 0> \<0/1>
+ (!) /rb \<rigidbodyID> <datatype = 1> \<markerID> \<x> \<y> \<z>
+ /rb \<rigidbodyID> <datatype = 2> \<timestamp> \<x> \<y> \<z> \<qx> \<qy> \<qz> \<qw>
+ (!) /skel \<skleletonID> <datatype = 10> \<boneID> \<timestamp> \<x> \<y> \<z> \<qx> \<qy> \<qz> \<qw>

**(!) - will only be sent if the CLI flags are set.**

IF you want to have multiple modes, set the oscmode like "max,isadora" or "isadora,touch" and make sure no space is between the values

At the end of the frame the frameend message is sent

+ /frame/end \<frameNumber>

if OSC MODE = sparck

+ /f/e \<frameNumber>

### Remote control

sending commands to the \<OscListeningPort> will pass commands to Motive:

the following commands are implemented:

+ /script/oscModeSparck (0..1) will start/stop streaming max type messages
+ /script/oscModeMax (0..1) will start/stop streaming max type messages
+ /script/oscModeIsadora (0..1) will start/stop streaming isadora type messages
+ /script/oscModeTouch (0..1) will start/stop streaming touchdesigner type messages
+ /script/sendSkeletons (0..1) will start/stop streaming skeleton data
+ /script/sendMarkerInfo (0..1) will start/stop streaming marker data
+ /script/verbose (0..1) will start/stop outputing verbose messages
+ /script/autoReconnect (0..1) will start/stop auto reconnecting
+ /script/zUpAxis (0..1) 1 = will transform to zUp orientation
+ /script/leftHanded (0..1) 1 = will transform to left handed orientation
+ /script/bundled (0..1) will start/stop bundle the osc messages
+ /script/calcMatrix (0..1) 1 = will start/stop calculate the transformation matrix
+ /script/calcInvMatrix (0..1) 1 = will start/stop calculate the inverse transformation matrix

--

+ /motive/command refetch

will return all command options and all rigidbodies and skeletons currently streaming

* /motive/update/start
+ /motive/rigidbody/id \<rigidbodyName> \<rigidbodyID>
+ /motive/skeleton/id \<skleletonName> \<SkeletonID>
+ /motive/skeleton/id/bone \<skleletonName> \<boneID> \<boneName>
+ /motive/forceplate/id \<serial>
+ /motive/forceplate/id/channel \<serial> \<channelID> \<channelName>
+ /motive/update/end

Building
---------

This code is based on the NatNet SDK from optitrack (http://optitrack.com/downloads/developer-tools.html) and includes the dll from SharpOSC (https://github.com/ValdemarOrn/SharpOSC).

Open the NatNetSamples inside the /Samples - folder and build NatNetThree2OSC Project

### Dependency

[CommandLine Parser Library](https://github.com/commandlineparser/commandline). How install the package got [here](https://github.com/commandlineparser/commandline/wiki/Getting-Started).

[SharpOSC](https://github.com/tecartlab/SharpOSC/releases/tag/v0.1.2.0). This is a fork from the official repo with a new class that allows "bidirectional" udp connectîons.

Contribute
----------

I would love to get some feedback. Use the Issue tracker on Github to send bug reports and feature requests, or just if you have something to say about the project. If you have code changes that you would like to have integrated into the main repository, send me a pull request or a patch. I will try my best to integrate them and make sure NatNetThree2OSC improves and matures.

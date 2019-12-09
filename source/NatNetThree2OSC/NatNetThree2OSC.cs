//=============================================================================
// Copyright © 2017 NaturalPoint, Inc. All Rights Reserved.
// 
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall NaturalPoint, Inc. or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//=============================================================================


using System;
using System.Collections;
using System.Collections.Generic;

using NatNetML;


/* SampleClientML.cs
 * 
 * This program is a sample console application which uses the managed NatNet assembly (NatNetML.dll) for receiving NatNet data
 * from a tracking server (e.g. Motive) and outputting them in every 200 mocap frames. This is provided mainly for demonstration purposes,
 * and thus, the program is designed at its simpliest approach. The program connects to a server application at a localhost IP address
 * (127.0.0.1) using Multicast connection protocol.
 *  
 * You may integrate this program into your applications if needed. This program is not designed to take account for latency/frame build up
 * when tracking a high number of assets. For more robust and comprehensive use of the NatNet assembly, refer to the provided WinFormSample project.
 * 
 *  Note: The NatNet .NET assembly is derived from the native NatNetLib.dll library, so make sure the NatNetLib.dll is linked to your application
 *        along with the NatNetML.dll file.  
 * 
 *  List of Output Data:
 *  ====================
 *      - Markers Data : Prints out total number of markers reconstructed in the scene.
 *      - Rigid Body Data : Prints out position and orientation data
 *      - Skeleton Data : Prints out only the position of the hip segment
 *      - Force Plate Data : Prints out only the first subsample data per each mocap frame
 * 
 */


namespace NatNetThree2OSC
{
    public class NatNetThree2OSC
    {
        /*  [NatNet] Network connection configuration    */
        private static NatNetML.NatNetClientML mNatNet;    // The client instance
        private static string mStrLocalIP = "127.0.0.1";   // Local IP address (string)
        private static string mStrServerIP = "127.0.0.1";  // Server IP address (string)
        private static string mStrOscSendIP = "127.0.0.1";   // Local OSC IP address (string)
        private static int mStrOscSendPort = 54321;   // Local OSC Port (int)
        private static int mStrOscListeningPort = 55555;   // Local OSC Port (int)
        private static bool mVerbose = false;    // verbose data (bool)
        private static string mOscMode = "max";    // osc mode sets the different structures of osc messages 
        private static bool mOscModeMax = true;
        private static bool mOscModeIsa = false;
        private static bool mOscModeTouch = false;
        private static int mUpAxis = 0;
        private static NatNetML.ConnectionType mConnectionType = ConnectionType.Multicast; // Multicast or Unicast mode


        /*  List for saving each of datadescriptors */
        private static List<NatNetML.DataDescriptor> mDataDescriptor = new List<NatNetML.DataDescriptor>(); 

        /*  Lists and Hashtables for saving data descriptions   */
        private static Hashtable mHtSkelRBs = new Hashtable();
        private static List<RigidBody> mRigidBodies = new List<RigidBody>();
        private static List<Skeleton> mSkeletons = new List<Skeleton>();
        private static List<ForcePlate> mForcePlates = new List<ForcePlate>();

        /*  boolean value for detecting change in asset */
        private static bool mAssetChanged = false;

        // create an OSC Udp sender
        private static SharpOSC.UDPSender OSCsender;

        static void Main(string[] args)
        {
            if (args.Length >= 3)
            {
                mStrLocalIP = args[0];
                mStrServerIP = args[1];
                mStrOscSendIP = args[2];
            } else
            {
                Console.WriteLine("Less than 3 arguments. Ignoring input...");
            }
            int j;
            bool jj;
            if (args.Length >= 4)
            {
                if (Int32.TryParse(args[3], out j))
                    mStrOscSendPort = j;
                else
                    Console.WriteLine("OscSendPort value is no number ({0:N3}). Ignoring input...", args[3]);
            }
            if (args.Length >= 5)
            {
                if (Int32.TryParse(args[4], out j))
                    mStrOscListeningPort = j;
                else
                    Console.WriteLine("OscListeningPort value is no number ({0:N3}). Ignoring input...", args[4]);
            }
            if (args.Length >= 6)
            {
                mOscMode = args[5];
                mOscModeMax = (mOscMode.ToLower().Contains("max")) ? true : false;
                mOscModeIsa = (mOscMode.ToLower().Contains("isadora")) ? true : false;
                mOscModeTouch = (mOscMode.ToLower().Contains("touch")) ? true : false;
                if (!mOscMode.ToLower().Contains("touch") && !mOscMode.ToLower().Contains("isadora") && !mOscMode.ToLower().Contains("max"))
                    Console.WriteLine("Value is not valid (max, isadora, touch) ({0:N3}). Ignoring input...", args[5]);
            }
            if (args.Length >= 7)
            {
                if (Int32.TryParse(args[6], out j))
                    mUpAxis = j;
                else
                    Console.WriteLine("upAxis value is no number ({0:N3}). Ignoring input...", args[6]);
            }
            if (args.Length == 8)
            {
                if (Boolean.TryParse(args[7], out jj))
                    mVerbose = jj;
                else
                    Console.WriteLine("Verbose value is no boolean (true/false) ({0:N3}). Ignoring input...", args[7]);
            }
            Console.WriteLine("\n---- NatNetThree2OSC v. 4.0  ----");
            Console.WriteLine("\n----   20190519 by maybites  ----");

            Console.WriteLine("\nNatNetThree2OSC \n\t<NatNetLocal IP \t({0:N3})> \n\t<NatNetServer IP \t({1:N3})> \n\t<OscSendIP \t\t({2:N3})> \n\t<OscSendPort \t\t({3})> \n\t<OscListeningPort \t({4})> \n\t<oscMode \t\t[{5}]> \n\t<upAxis \t\t[{6}]> \n\t<verbose \t\t[{7}]>\n", mStrLocalIP, mStrServerIP, mStrOscSendIP, mStrOscSendPort, mStrOscListeningPort, mOscMode, mUpAxis, mVerbose);

            // intantiate an OSC udp sender
            OSCsender = new SharpOSC.UDPSender(mStrOscSendIP, mStrOscSendPort);

            Console.WriteLine("NatNetThree2OSC managed client application starting...\n");
            /*  [NatNet] Initialize client object and connect to the server  */
            connectToServer();                          // Initialize a NatNetClient object and connect to a server.

            Console.WriteLine("============================ SERVER DESCRIPTOR ================================\n");
            /*  [NatNet] Confirming Server Connection. Instantiate the server descriptor object and obtain the server description. */
            bool connectionConfirmed = fetchServerDescriptor();    // To confirm connection, request server description data

            if (connectionConfirmed)                         // Once the connection is confirmed.
            {
                Console.WriteLine("============================= DATA DESCRIPTOR =================================\n");
                Console.WriteLine("Now Fetching the Data Descriptor.\n");
                fetchDataDescriptor();                  //Fetch and parse data descriptor

                Console.WriteLine("============================= FRAME OF DATA ===================================\n");
                Console.WriteLine("Now Fetching the Frame Data\n");

                /*  [NatNet] Assigning a event handler function for fetching frame data each time a frame is received   */
                mNatNet.OnFrameReady += new NatNetML.FrameReadyEventHandler(fetchFrameData);
               
                Console.WriteLine("Success: Data Port Connected \n");

                Console.WriteLine("======================== STREAMING IN (PRESS ESC TO EXIT) =====================\n");
            }

            // The cabllback function for receiveing OSC messages
            SharpOSC.HandleOscPacket callback = delegate (SharpOSC.OscPacket packet)
            {
                var messageReceived = (SharpOSC.OscMessage)packet;
                if (messageReceived != null && messageReceived.Address.Equals(value: "/command"))
                {
                    if(messageReceived.Arguments.Count > 0 && messageReceived.Arguments.IndexOf("refetch") != -1)
                    {
                        Console.WriteLine("Received Refetching Command.");
                        mAssetChanged = true;
                    }
                }
            };

            var listener = new SharpOSC.UDPListener(mStrOscListeningPort, callback);

            while (!(Console.KeyAvailable && Console.ReadKey().Key == ConsoleKey.Escape))
            {
                // Continuously listening for Frame data
                // Enter ESC to exit

                // Exception handler for updated assets list.
                if (mAssetChanged == true)
                {
                    Console.WriteLine("\n===============================================================================\n");
                    Console.WriteLine("Change in the list of the assets. Refetching the descriptions");

                    /*  Clear out existing lists */
                    mDataDescriptor.Clear();
                    mHtSkelRBs.Clear();
                    mRigidBodies.Clear();
                    mSkeletons.Clear();
                    mForcePlates.Clear();

                    /* [NatNet] Re-fetch the updated list of descriptors  */
                    fetchDataDescriptor();
                    Console.WriteLine("===============================================================================\n");
                    mAssetChanged = false;
                }
            }
            /*  [NatNet] Disabling data handling function   */
            mNatNet.OnFrameReady -= fetchFrameData;

            /*  Clearing Saved Descriptions */
            mRigidBodies.Clear();
            mSkeletons.Clear();
            mHtSkelRBs.Clear();
            mForcePlates.Clear();
            mNatNet.Disconnect();
            listener.Close();
        }

         /// <summary>
        /// [NatNet] parseFrameData will be called when a frame of Mocap
        /// data has is received from the server application.
        ///
        /// Note: This callback is on the network service thread, so it is
        /// important to return from this function quickly as possible 
        /// to prevent incoming frames of data from buffering up on the
        /// network socket.
        ///
        /// Note: "data" is a reference structure to the current frame of data.
        /// NatNet re-uses this same instance for each incoming frame, so it should
        /// not be kept (the values contained in "data" will become replaced after
        /// this callback function has exited).
        /// </summary>
        /// <param name="data">The actual frame of mocap data</param>
        /// <param name="client">The NatNet client instance</param>
        static void fetchFrameData(NatNetML.FrameOfMocapData data, NatNetML.NatNetClientML client)
        {

            /*  Exception handler for cases where assets are added or removed.
                Data description is re-obtained in the main function so that contents
                in the frame handler is kept minimal. */
            if (( data.bTrackingModelsChanged == true || data.nRigidBodies != mRigidBodies.Count || data.nSkeletons != mSkeletons.Count || data.nForcePlates != mForcePlates.Count))
            {
                mAssetChanged = true;
            }
            else if(data.iFrame % 1 == 0)
            {
                /*  Processing and ouputting frame data every 200th frame.
                    This conditional statement is included in order to simplify the program output */

                var message = new SharpOSC.OscMessage("/frame/start", data.iFrame);
                OSCsender.Send(message);

                /*
                if (data.bRecording == false)
                    Console.WriteLine("Frame #{0} Received:", data.iFrame);
                else if (data.bRecording == true)
                    Console.WriteLine("[Recording] Frame #{0} Received:", data.iFrame);
                */

                 processFrameData(data);

                message = new SharpOSC.OscMessage("/frame/end", data.iFrame);
                OSCsender.Send(message);
            }
        }

        static void processFrameData(NatNetML.FrameOfMocapData data)
        {

            var message = new SharpOSC.OscMessage("/marker");
            /*  Parsing marker Frame Data   */

            for (int j = 0; j < data.nMarkers; j++)
            {
                NatNetML.Marker rbData = data.LabeledMarkers[j];    // Received marker descriptions
                
                float pxt, pyt, pzt = 0.0f;
                if (mUpAxis == 1)
                {
                    pxt = rbData.x;
                    pyt = -rbData.z;
                    pzt = rbData.y;
                }
                else
                {
                    pxt = rbData.x;
                    pyt = rbData.y;
                    pzt = rbData.z;
                }

                if (mOscModeMax)
                {
                    message = new SharpOSC.OscMessage("/marker", rbData.ID, "position", pxt, pyt, pzt);
                    OSCsender.Send(message);
                }
                if (mOscModeIsa || mOscModeTouch)
                {
                    message = new SharpOSC.OscMessage("/marker" + rbData.ID + "/position", pxt, pyt, pzt);
                    OSCsender.Send(message);
                }
            }


            /*  Parsing Rigid Body Frame Data   */
            for (int i = 0; i < mRigidBodies.Count; i++)
            {
                int rbID = mRigidBodies[i].ID;              // Fetching rigid body IDs from the saved descriptions

                for (int j = 0; j < data.nRigidBodies; j++)
                {
                    if(rbID == data.RigidBodies[j].ID)      // When rigid body ID of the descriptions matches rigid body ID of the frame data.
                    {
                        NatNetML.RigidBody rb = mRigidBodies[i];                // Saved rigid body descriptions
                        NatNetML.RigidBodyData rbData = data.RigidBodies[j];    // Received rigid body descriptions

                        float pxt, pyt, pzt, qxt, qyt, qzt, qwt = 0.0f;
                        if (mUpAxis == 1)
                        {
                            pxt = rbData.x;
                            pyt = -rbData.z;
                            pzt = rbData.y;
                            qxt = rbData.qx;
                            qyt = -rbData.qz;
                            qzt = rbData.qy;
                            qwt = rbData.qw;
                        }
                        else
                        {
                            pxt = rbData.x;
                            pyt = rbData.y;
                            pzt = rbData.z;
                            qxt = rbData.qx;
                            qyt = rbData.qy;
                            qzt = rbData.qz;
                            qwt = rbData.qw;
                        }

                        if (rbData.Tracked == true)
                        {
                            if (mOscModeMax)
                            {
                                message = new SharpOSC.OscMessage("/rigidbody", rb.ID, "tracked", 1);
                                OSCsender.Send(message);
                                message = new SharpOSC.OscMessage("/rigidbody", rb.ID, "position", pxt, pyt, pzt);
                                OSCsender.Send(message);
                                message = new SharpOSC.OscMessage("/rigidbody", rb.ID, "quat", qxt, qyt, qzt, qwt);
                                OSCsender.Send(message);
                            }
                            if (mOscModeIsa)
                            {
                                message = new SharpOSC.OscMessage("/rigidbody/" + rb.ID + "/tracked", 1);
                                OSCsender.Send(message);
                                message = new SharpOSC.OscMessage("/rigidbody/" + rb.ID + "/position", pxt, pyt, pzt);
                                OSCsender.Send(message);
                                message = new SharpOSC.OscMessage("/rigidbody/" + rb.ID + "/quat", qxt, qyt, qzt, qwt);
                                OSCsender.Send(message);
                            }
                            if (mOscModeTouch)
                            {
                                message = new SharpOSC.OscMessage("/rigidbody/" + rb.ID + "/tracked", 1);
                                OSCsender.Send(message);
                                message = new SharpOSC.OscMessage("/rigidbody/" + rb.ID + "/transformation", pxt, pyt, pzt, qxt, qyt, qzt, qwt);
                                OSCsender.Send(message);
                            }

                            /*
                            Console.WriteLine("\tRigidBody ({0}):", rb.Name);
                            Console.WriteLine("\t\tpos ({0:N3}, {1:N3}, {2:N3})", rbData.x, rbData.y, rbData.z);

                            // Rigid Body Euler Orientation
                            float[] quat = new float[4] { rbData.qx, rbData.qy, rbData.qz, rbData.qw };
                            float[] eulers = new float[3];

                            eulers = NatNetClientML.QuatToEuler(quat, NATEulerOrder.NAT_XYZr); //Converting quat orientation into XYZ Euler representation.
                            double xrot = RadiansToDegrees(eulers[0]);
                            double yrot = RadiansToDegrees(eulers[1]);
                            double zrot = RadiansToDegrees(eulers[2]);

                            Console.WriteLine("\t\tori ({0:N3}, {1:N3}, {2:N3})", xrot, yrot, zrot);
                            */
                        }
                        else
                        {
                            if (mOscModeMax)
                            {
                                message = new SharpOSC.OscMessage("/rigidbody", rb.ID, "tracked", 0);
                                OSCsender.Send(message);
                            }
                            if (mOscModeIsa || mOscModeTouch)
                            {
                                message = new SharpOSC.OscMessage("/rigidbody/" + rb.ID + "/tracked", 0);
                                OSCsender.Send(message);
                            }
                            // HERE AN INFO MESSAGE can be sent that this rigidbody is occluded...(set red...)

                            //Console.WriteLine("\t{0} is not tracked in frame {1}", rb.Name, data.iFrame);
                        }
                    }
                }
            }

            /* Parsing Skeleton Frame Data  */
            for (int i = 0; i < mSkeletons.Count; i++)      // Fetching skeleton IDs from the saved descriptions
            {
                int sklID = mSkeletons[i].ID;

                for (int j = 0; j < data.nSkeletons; j++)
                {
                    if (sklID == data.Skeletons[j].ID)      // When skeleton ID of the description matches skeleton ID of the frame data.
                    {
                        NatNetML.Skeleton skl = mSkeletons[i];              // Saved skeleton descriptions
                        NatNetML.SkeletonData sklData = data.Skeletons[j];  // Received skeleton frame data


                        //Console.WriteLine("\tSkeleton ({0}):", skl.Name);
                        //Console.WriteLine("\t\tSegment count: {0}", sklData.nRigidBodies);

                        //  Now, for each of the skeleton segments 
                        for (int k = 0; k < sklData.nRigidBodies; k++)
                        {

                            NatNetML.RigidBodyData boneData = sklData.RigidBodies[k];

                            float pxt, pyt, pzt, qxt, qyt, qzt, qwt = 0.0f;
                            if (mUpAxis == 1)
                            {
                                pxt = boneData.x;
                                pyt = -boneData.z;
                                pzt = boneData.y;
                                qxt = boneData.qx;
                                qyt = -boneData.qz;
                                qzt = boneData.qy;
                                qwt = boneData.qw;
                            }
                            else
                            {
                                pxt = boneData.x;
                                pyt = boneData.y;
                                pzt = boneData.z;
                                qxt = boneData.qx;
                                qyt = boneData.qy;
                                qzt = boneData.qz;
                                qwt = boneData.qw;
                            }

                            //  Decoding skeleton bone ID   
                            int skeletonID = HighWord(boneData.ID);
                            int rigidBodyID = LowWord(boneData.ID);
                            int uniqueID = skeletonID * 1000 + rigidBodyID;
                            int key = uniqueID.GetHashCode();

                            NatNetML.RigidBody bone = (RigidBody)mHtSkelRBs[key];   //Fetching saved skeleton bone descriptions

                            if (bone != null) // during a refetch the bone descriptions might be removed for a moment
                            {
                                if (mOscModeMax)
                                {
                                    message = new SharpOSC.OscMessage("/skeleton/bone", skl.Name, bone.ID, "position", pxt, pyt, pzt);
                                    OSCsender.Send(message);
                                    message = new SharpOSC.OscMessage("/skeleton/bone", skl.Name, bone.ID, "quat", qxt, qyt, qzt, qwt);
                                    OSCsender.Send(message);
                                    message = new SharpOSC.OscMessage("/skeleton/joint", skl.Name, bone.ID, "quat", qxt, qyt, qzt, qwt);
                                    OSCsender.Send(message);
                                }
                                if (mOscModeIsa)
                                {
                                    message = new SharpOSC.OscMessage("/skeleton/" + skl.Name + "/bone/" + bone.ID + "/position", pxt, pyt, pzt);
                                    OSCsender.Send(message);
                                    message = new SharpOSC.OscMessage("/skeleton/" + skl.Name + "/bone/" + bone.ID + "/quat", qxt, qyt, qzt, qwt);
                                    OSCsender.Send(message);
                                    message = new SharpOSC.OscMessage("/skeleton/" + skl.Name + "/joint/" + bone.ID + "/quat", qxt, qyt, qzt, qwt);
                                    OSCsender.Send(message);
                                }
                                if (mOscModeTouch)
                                {
                                    message = new SharpOSC.OscMessage("/skeleton/" + skl.Name + "/bone/" + bone.ID + "/transformation", pxt, pyt, pzt, qxt, qyt, qzt, qwt);
                                    OSCsender.Send(message);
                                    message = new SharpOSC.OscMessage("/skeleton/" + skl.Name + "/joint/" + bone.ID + "/quat", qxt, qyt, qzt, qwt);
                                    OSCsender.Send(message);
                                }
                            }
                        }
                    }
                }
            }
            
            /*  Parsing Force Plate Frame Data  */
            for (int i = 0; i < mForcePlates.Count; i++)
            {
                int fpID = mForcePlates[i].ID;                  // Fetching force plate IDs from the saved descriptions

                for (int j = 0; j < data.nForcePlates; j++)
                {
                    if (fpID == data.ForcePlates[j].ID)         // When force plate ID of the descriptions matches force plate ID of the frame data.
                    {
                        NatNetML.ForcePlate fp = mForcePlates[i];                // Saved force plate descriptions
                        NatNetML.ForcePlateData fpData = data.ForcePlates[i];    // Received forceplate frame data

                        Console.WriteLine("\tForce Plate ({0}):", fp.Serial);

                        // Here we will be printing out only the first force plate "subsample" (index 0) that was collected with the mocap frame.
                        for (int k = 0; k < fpData.nChannels; k++)
                        {
                            Console.WriteLine("\t\tChannel {0}: {1}", fp.ChannelNames[k], fpData.ChannelData[k].Values[0]);
                        }
                    }
                }
            }
            //Console.WriteLine("\n");
        }

        static void connectToServer()
        {
            /*  [NatNet] Instantiate the client object  */
            mNatNet = new NatNetML.NatNetClientML();

            /*  [NatNet] Checking verions of the NatNet SDK library  */
            int[] verNatNet = new int[4];           // Saving NatNet SDK version number
            verNatNet = mNatNet.NatNetVersion();
            Console.WriteLine("NatNet SDK Version: {0}.{1}.{2}.{3}", verNatNet[0], verNatNet[1], verNatNet[2], verNatNet[3]);

            /*  [NatNet] Connecting to the Server    */
            Console.WriteLine("\nConnecting...\n\tLocal IP address: {0}\n\tServer IP Address: {1}\n\n", mStrLocalIP, mStrServerIP);

            NatNetClientML.ConnectParams connectParams = new NatNetClientML.ConnectParams();
            connectParams.ConnectionType = mConnectionType;
            connectParams.ServerAddress = mStrServerIP;
            connectParams.LocalAddress = mStrLocalIP;
            mNatNet.Connect( connectParams );
        }

        static bool fetchServerDescriptor()
        {
            NatNetML.ServerDescription m_ServerDescriptor = new NatNetML.ServerDescription();
            int errorCode = mNatNet.GetServerDescription(m_ServerDescriptor);

            if (errorCode == 0)
            {
                Console.WriteLine("Success: Connected to the server\n");
                parseSeverDescriptor(m_ServerDescriptor);
                return true;
            }
            else
            {
                Console.WriteLine("Error: Failed to connect. Check the connection settings.");
                Console.WriteLine("Program terminated (Enter ESC to exit)");
                return false;
            }
        }

        static void parseSeverDescriptor(NatNetML.ServerDescription server)
        {
            Console.WriteLine("Server Info:");
            Console.WriteLine("\tHost: {0}", server.HostComputerName);
            Console.WriteLine("\tApplication Name: {0}", server.HostApp);
            Console.WriteLine("\tApplication Version: {0}.{1}.{2}.{3}", server.HostAppVersion[0], server.HostAppVersion[1], server.HostAppVersion[2], server.HostAppVersion[3]);
            Console.WriteLine("\tNatNet Version: {0}.{1}.{2}.{3}\n", server.NatNetVersion[0], server.NatNetVersion[1], server.NatNetVersion[2], server.NatNetVersion[3]);
        }

        static void fetchDataDescriptor()
        {
            /*  [NatNet] Fetch Data Descriptions. Instantiate objects for saving data descriptions and frame data    */        
            bool result = mNatNet.GetDataDescriptions(out mDataDescriptor);
            if (result)
            {
                Console.WriteLine("Success: Data Descriptions obtained from the server.");
                parseDataDescriptor(mDataDescriptor);
            }
            else
            {
                Console.WriteLine("Error: Could not get the Data Descriptions");
            }
            Console.WriteLine("\n");
        }

        static void parseDataDescriptor(List<NatNetML.DataDescriptor> description)
        {
            //  [NatNet] Request a description of the Active Model List from the server. 
            //  This sample will list only names of the data sets, but you can access 
            int numDataSet = description.Count;
            Console.WriteLine("Total {0} data sets in the capture:", numDataSet);
            
            for (int i = 0; i < numDataSet; ++i)
            {
                int dataSetType = description[i].type;
                // Parse Data Descriptions for each data sets and save them in the delcared lists and hashtables for later uses.
                switch (dataSetType)
                {
                    case ((int) NatNetML.DataDescriptorType.eMarkerSetData):
                        NatNetML.MarkerSet mkset = (NatNetML.MarkerSet)description[i];
                        Console.WriteLine("\tMarkerSet ({0})", mkset.Name);
                        var message = new SharpOSC.OscMessage("/markerset/id", mkset.Name);
                        OSCsender.Send(message);

                        break;

                    case ((int) NatNetML.DataDescriptorType.eRigidbodyData):
                        NatNetML.RigidBody rb = (NatNetML.RigidBody)description[i];
                        Console.WriteLine("\tRigidBody ({0})", rb.Name);

                        message = new SharpOSC.OscMessage("/rigidbody/id", rb.Name, rb.ID);
                        OSCsender.Send(message);

                        // Saving Rigid Body Descriptions
                        mRigidBodies.Add(rb);
                        break;


                    case ((int) NatNetML.DataDescriptorType.eSkeletonData):
                        NatNetML.Skeleton skl = (NatNetML.Skeleton)description[i];
                        Console.WriteLine("\tSkeleton ({0}), Bones:", skl.Name);

                        message = new SharpOSC.OscMessage("/skeleton/id", skl.Name, skl.ID);
                        OSCsender.Send(message);

                        //Saving Skeleton Descriptions
                        mSkeletons.Add(skl);

                        // Saving Individual Bone Descriptions
                        for (int j = 0; j < skl.nRigidBodies; j++)
                        {
                            message = new SharpOSC.OscMessage("/skeleton/id/bone", skl.Name, skl.RigidBodies[j].ID, skl.RigidBodies[j].Name);
                            OSCsender.Send(message);

                            Console.WriteLine("\t\t{0}. {1}", skl.RigidBodies[j].ID, skl.RigidBodies[j].Name);
                            int uniqueID = skl.ID * 1000 + skl.RigidBodies[j].ID;
                            int key = uniqueID.GetHashCode();
                            mHtSkelRBs.Add(key, skl.RigidBodies[j]); //Saving the bone segments onto the hashtable
                        }
                        break;


                    case ((int) NatNetML.DataDescriptorType.eForcePlateData):
                        NatNetML.ForcePlate fp = (NatNetML.ForcePlate)description[i];
                        Console.WriteLine("\tForcePlate ({0})", fp.Serial);

                        message = new SharpOSC.OscMessage("/forceplate/id", fp.Serial);
                        OSCsender.Send(message);

                        // Saving Force Plate Channel Names
                        mForcePlates.Add(fp);
               
                        for (int j = 0; j < fp.ChannelCount; j++)
                        {
                            message = new SharpOSC.OscMessage("/forceplate/id/channel", fp.Serial, j + 1, fp.ChannelNames[j]);
                            OSCsender.Send(message);

                            Console.WriteLine("\t\tChannel {0}: {1}", j + 1, fp.ChannelNames[j]);
                        }
                        break;

                    default:
                        // When a Data Set does not match any of the descriptions provided by the SDK.
                        Console.WriteLine("\tError: Invalid Data Set");
                        break;
                }
            }
        }

        static double RadiansToDegrees(double dRads)
        {
            return dRads * (180.0f / Math.PI);
        }
        
        static int LowWord(int number)
        {
            return number & 0xFFFF;
        }

        static int HighWord(int number)
        {
            return ((number >> 16) & 0xFFFF);
        }
    } // End. ManagedClient class
} // End. NatNetML Namespace

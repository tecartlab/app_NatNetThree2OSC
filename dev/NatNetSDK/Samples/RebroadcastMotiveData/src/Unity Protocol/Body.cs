using UnityEngine;
using System.Collections;
using System.Xml;

/* 
Copyright © 2016 NaturalPoint Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

// Attach Body.cs to an empty Game Object and it will parse and create visual
// game objects based on bone data.  Body.cs is meant to be a simple example 
// of how to parse and display skeletal data in Unity.

// In order to work properly, this class is expecting that you also have instantiated
// another game object and attached the Slip Stream script to it.  Alternatively
// they could be attached to the same object.

public class Body : MonoBehaviour {
	
	public GameObject SlipStreamObject;
	
	// Use this for initialization
	void Start () 
	{
		SlipStreamObject.GetComponent<SlipStream>().PacketNotification += new PacketReceivedHandler(OnPacketReceived);
	}
	
	// packet received
	void OnPacketReceived(object sender, string Packet)
	{
		XmlDocument xmlDoc= new XmlDocument();
		xmlDoc.LoadXml(Packet);

        //== skeletons ==--

		XmlNodeList boneList = xmlDoc.GetElementsByTagName("Bone");

		for(int index=0; index<boneList.Count; index++)
		{
		
			int id = System.Convert.ToInt32(boneList[index].Attributes["ID"].InnerText);
			
			float x = (float) System.Convert.ToDouble(boneList[index].Attributes["x"].InnerText);
			float y = (float) System.Convert.ToDouble(boneList[index].Attributes["y"].InnerText);
			float z = (float) System.Convert.ToDouble(boneList[index].Attributes["z"].InnerText);
			
			float qx = (float) System.Convert.ToDouble(boneList[index].Attributes["qx"].InnerText);
			float qy = (float) System.Convert.ToDouble(boneList[index].Attributes["qy"].InnerText);
			float qz = (float) System.Convert.ToDouble(boneList[index].Attributes["qz"].InnerText);
			float qw = (float) System.Convert.ToDouble(boneList[index].Attributes["qw"].InnerText);
			
			//== coordinate system conversion (right to left handed) ==--
			
			z = -z;
			qz = -qz;
			qw = -qw;
			
			//== bone pose ==--
			
			Vector3    position    = new Vector3(x,y,z);
			Quaternion orientation = new Quaternion(qx,qy,qz,qw);
			
			//== locate or create bone object ==--
			
			string objectName = "Bone"+id.ToString();
				
			GameObject bone;
			
			bone = GameObject.Find(objectName);
			
			if(bone==null)
			{
				bone = GameObject.CreatePrimitive(PrimitiveType.Cube);
				Vector3 scale = new Vector3(0.1f,0.1f,0.1f);
				bone.transform.localScale = scale;
				bone.name = objectName;
			}		
			
			//== set bone's pose ==--
			
			bone.transform.position = position;
			bone.transform.rotation = orientation;
		}

        //== rigid bodies ==--

        XmlNodeList rbList = xmlDoc.GetElementsByTagName("RigidBody");

        for (int index = 0; index < rbList.Count; index++)
        {

            int id = System.Convert.ToInt32(rbList[index].Attributes["ID"].InnerText);

            float x = (float)System.Convert.ToDouble(rbList[index].Attributes["x"].InnerText);
            float y = (float)System.Convert.ToDouble(rbList[index].Attributes["y"].InnerText);
            float z = (float)System.Convert.ToDouble(rbList[index].Attributes["z"].InnerText);

            float qx = (float)System.Convert.ToDouble(rbList[index].Attributes["qx"].InnerText);
            float qy = (float)System.Convert.ToDouble(rbList[index].Attributes["qy"].InnerText);
            float qz = (float)System.Convert.ToDouble(rbList[index].Attributes["qz"].InnerText);
            float qw = (float)System.Convert.ToDouble(rbList[index].Attributes["qw"].InnerText);

            //== coordinate system conversion (right to left handed) ==--

            z = -z;
            qz = -qz;
            qw = -qw;

            //== bone pose ==--

            Vector3 position = new Vector3(x, y, z);
            Quaternion orientation = new Quaternion(qx, qy, qz, qw);

            //== locate or create bone object ==--

            string objectName = "RigidBody" + id.ToString();

            GameObject bone;

            bone = GameObject.Find(objectName);

            if (bone == null)
            {
                bone = GameObject.CreatePrimitive(PrimitiveType.Cube);
                Vector3 scale = new Vector3(0.1f, 0.1f, 0.1f);
                bone.transform.localScale = scale;
                bone.name = objectName;
            }

            //== set bone's pose ==--

            bone.transform.position = position;
            bone.transform.rotation = orientation;
        }
	}
	
	// Update is called once per frame
	void Update () 
	{
	
	}
}

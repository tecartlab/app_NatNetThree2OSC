using UnityEngine;
using System.Collections;
using System.Collections.Generic;
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

public class LivePoseAnimator : MonoBehaviour
{
    private Avatar mocapAvatar;
    public Avatar destinationAvatar;

    public HumanPoseHandler SourcePoseHandler;
    public HumanPoseHandler HumanPoseHandler;

    public HumanPose HumanPose;

    public GameObject SlipStreamObject;

    public bool ShowMocapData = false;
    private bool currentShowMocapData = false;

    void Start()
    {
        //== set up the pose handler ==--
        HumanPoseHandler = new HumanPoseHandler(destinationAvatar, this.transform);
        SourcePoseHandler = null;

        //== hook to incoming UDP real-time mocap data ==--
        SlipStreamObject.GetComponent<SlipStream>().PacketNotification += new PacketReceivedHandler(OnPacketReceived);
        PrepareBoneDictionary();
        PrepareBoneToSkeleton();
    }

    void Update()
    {
        //== if there is new data or settings changed, apply data and retarget ==--
        
        if (mNew || ShowMocapData!=currentShowMocapData || currentActor!=Actor)
        {
            if( mPacket==null )
            {
                return;
            }

            currentShowMocapData = ShowMocapData;

            if( currentActor!=Actor )
            {
                currentActor = Actor;
                PrepareBoneDictionary();
                PrepareBoneToSkeleton();
            }

            mNew = false;
            
            //== the following just pushes the incoming bone information to the appropriate
            //== game objects in the source skeleton

            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(mPacket);

            //== check for definition ==--
            
            XmlNodeList definitions = xmlDoc.GetElementsByTagName("SkeletonDescriptions");

            if( definitions.Count>0 )
            {
                ParseSkeletonDefinitions( xmlDoc );
                return;
            }

            if (SourcePoseHandler == null)
            {
                return;
            }
            
            //== skeletons ==--

            XmlNodeList boneList = xmlDoc.GetElementsByTagName("Bone");

            for (int index = 0; index < boneList.Count; index++)
            {
                string boneName = boneList[index].Attributes["Name"].InnerText;
                
                float x = (float)System.Convert.ToDouble(boneList[index].Attributes["x"].InnerText);
                float y = (float)System.Convert.ToDouble(boneList[index].Attributes["y"].InnerText);
                float z = (float)System.Convert.ToDouble(boneList[index].Attributes["z"].InnerText);

                float qx = (float)System.Convert.ToDouble(boneList[index].Attributes["qx"].InnerText);
                float qy = (float)System.Convert.ToDouble(boneList[index].Attributes["qy"].InnerText);
                float qz = (float)System.Convert.ToDouble(boneList[index].Attributes["qz"].InnerText);
                float qw = (float)System.Convert.ToDouble(boneList[index].Attributes["qw"].InnerText);

                //== coordinate system conversion (right to left handed) ==--

                x = -x;
                qx = -qx;
                qw = -qw;

                //== bone pose ==--

                Vector3 position = new Vector3(x, y, z);
                Quaternion orientation = new Quaternion(qx, qy, qz, qw);

                //== locate or create bone object ==--

                string objectName = boneName;

                /*if (mBoneDictionary.ContainsKey(objectName))
                {
                    objectName = "BVH:" + mBoneDictionary[objectName];
                }*/

                GameObject bone;

                bone = GameObject.Find(objectName);

                if (bone != null)
                {
                    bone.transform.localPosition = position;
                    bone.transform.localRotation = orientation;

                    //== create a cube visually so you can see the raw mocap data ==--

                    if( objectName.Contains( Actor ) )
                    {
                        Transform visualTransform = bone.transform.FindChild(objectName + "Visual");

                        if (ShowMocapData && visualTransform == null)
                        {
                            GameObject visual = GameObject.CreatePrimitive(PrimitiveType.Cube);
                            Vector3 scale = new Vector3(0.1f, 0.1f, 0.1f);
                            visual.transform.localScale = scale;
                            visual.name = objectName + "Visual";
                            visual.transform.parent = bone.transform;
                            visual.transform.localPosition = new Vector3(0, 0, 0);
                            visual.transform.localRotation = Quaternion.Euler(0, 0, 0);
                        }

                        if (ShowMocapData == false && visualTransform != null)
                        {
                            GameObject.Destroy(visualTransform.gameObject);
                        }
                    }
                }
            }

            //== rigid bodies ==-- 

            //== skip rigid bodies ==--

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

                x = -x;
                qx = -qx;
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

            //== fetch the muscle values using the human pose muscle model ==--
            SourcePoseHandler.GetHumanPose(ref HumanPose);

            //== retarget to the destination avatar ==--
            HumanPoseHandler.SetHumanPose(ref HumanPose);
        }
    }

    private void ParseSkeletonDefinitions( XmlDocument xmlDoc )
    {
        //== skeletons ==--

        XmlNodeList skeletonList = xmlDoc.GetElementsByTagName("SkeletonDescription");

        for (int index = 0; index < skeletonList.Count; index++)
        {
            ParseSkeletonDefinition( skeletonList.Item( index ) );
        } 
    }

    private void ParseSkeletonDefinition( XmlNode skeleton )
    {
        //== create skeleton object ==--

        string actorName = skeleton.Attributes["Name"].InnerText;

        if( actorName!=Actor )
        {
            //== we're looking for a specific actor here ==--
            return;
        }

        GameObject actorObject = GameObject.Find(actorName);

        if (actorObject != null)
        {
            //== actor already in scene, early out...
            return;
        }

        actorObject = new GameObject();

        Vector3 actorScale = new Vector3(1,1,1);
        actorObject.transform.localScale = actorScale;
        actorObject.name = actorName;

        //== Hierarchy ==--

        Dictionary<int, string> hierarchy = new Dictionary<int, string>();
    
        //== populate hierarchy tree first ==--

        for (int i = 0; i < skeleton.ChildNodes.Count; i++)
        {
            XmlNode bone = skeleton.ChildNodes.Item( i );

            string boneName = bone.Attributes["Name"].InnerText;

            int    ID       = System.Convert.ToInt32(bone.Attributes["ID"].InnerText);

            hierarchy.Add( ID, boneName );
        }

        for (int i = 0; i < skeleton.ChildNodes.Count; i++)
        {
            XmlNode bone = skeleton.ChildNodes.Item(i);

            string boneName = bone.Attributes["Name"].InnerText;

            int    parentID = System.Convert.ToInt32(bone.Attributes["ParentID"].InnerText);
            float  x        = (float)System.Convert.ToDouble(bone.Attributes["x"].InnerText);
            float  y        = (float)System.Convert.ToDouble(bone.Attributes["y"].InnerText);
            float  z        = (float)System.Convert.ToDouble(bone.Attributes["z"].InnerText);

            //== now we know the name, hierarchy, and location of each bone ==--

            string objectName = boneName;
             
            GameObject parentObject = null;

            if( parentID==0 )
            {
                //== parent object to top level ==--
                parentObject = actorObject;
            }
            else
            {
                if (hierarchy.ContainsKey(parentID))
                {
                    string parentName = hierarchy[parentID];

                    parentObject = GameObject.Find( parentName );
                }
            }

            GameObject boneObject;

            boneObject = GameObject.Find(objectName);

            if (boneObject == null)
            {
                boneObject = new GameObject();

                if (parentObject != null)
                {
                    boneObject.transform.parent = parentObject.transform;
                }
                Vector3 scale = new Vector3( 1,1,1 );
                boneObject.transform.localScale = scale;
                boneObject.name = objectName;

                Vector3 position = new Vector3(x, y, z);
                boneObject.transform.localPosition = position;

                Transform visualTransform = boneObject.transform.FindChild(objectName + "Visual");

                if (visualTransform==null && ShowMocapData)
                {
                    GameObject visual = GameObject.CreatePrimitive(PrimitiveType.Cube);
                    Vector3 scalev = new Vector3(0.1f, 0.1f, 0.1f);
                    visual.transform.localScale = scalev;
                    visual.name = objectName + "Visual";
                    visual.transform.parent = boneObject.transform;
                    visual.transform.localPosition = new Vector3(0, 0, 0);
                    visual.transform.localRotation = Quaternion.Euler(0, 0, 0);
                }
            }
        }
        
        //== now lets create the avatar ==--

        //== create the human description ==--
        HumanDescription desc = new HumanDescription();

        //=== here we go ==--

        //== go through all the bones and determine mapping ==--

        HumanBone[] humanBones = new HumanBone[ skeleton.ChildNodes.Count ];
            
        string[] humanName = HumanTrait.BoneName;

        int j = 0;
        int ii = 0;
        while (ii < humanName.Length) 
        {
            if (mBoneToSkeleton.ContainsKey(humanName[ii]))
            {
                HumanBone humanBone = new HumanBone();
                humanBone.humanName = humanName[ii];
                humanBone.boneName = mBoneToSkeleton[humanName[ii]];
                humanBone.limit.useDefaultValues = true;
                humanBones[j++] = humanBone;
            }
            ii++;
        }

        SkeletonBone[] skeletonBones = new SkeletonBone[skeleton.ChildNodes.Count+1];

        SkeletonBone sBone = new SkeletonBone();
        sBone.name = actorName;
        sBone.position = new Vector3(0, 0, 0);
        sBone.rotation = Quaternion.identity;
        sBone.scale = new Vector3(1, 1, 1);
        skeletonBones[0] = sBone;

        for (int i = 0; i < skeleton.ChildNodes.Count; i++)
        {
            XmlNode bone = skeleton.ChildNodes.Item(i);

            string boneName = bone.Attributes["Name"].InnerText;

            float x = (float)System.Convert.ToDouble(bone.Attributes["x"].InnerText);
            float y = (float)System.Convert.ToDouble(bone.Attributes["y"].InnerText);
            float z = (float)System.Convert.ToDouble(bone.Attributes["z"].InnerText);

            //== populate skeleton info ==--

            SkeletonBone skeletonBone = new SkeletonBone();
            skeletonBone.name = boneName;

            skeletonBone.position = new Vector3(x, y, z);
            skeletonBone.rotation = Quaternion.identity;
            skeletonBone.scale = new Vector3( 1,1,1 );
            skeletonBones[i+1] = skeletonBone;
        }

        //set the bone arrays right
        desc.human = humanBones;
        desc.skeleton = skeletonBones;

        //set the default values for the rest of the human descriptor parameters
        desc.upperArmTwist = 0.5f;
        desc.lowerArmTwist = 0.5f;
        desc.upperLegTwist = 0.5f;
        desc.lowerLegTwist = 0.5f;
        desc.armStretch = 0.05f;
        desc.legStretch = 0.05f;
        desc.feetSpacing = 0.0f;
        desc.hasTranslationDoF = false;

        mocapAvatar = AvatarBuilder.BuildHumanAvatar(GameObject.Find(actorName), desc);
        SourcePoseHandler = new HumanPoseHandler(mocapAvatar, GameObject.Find( actorName ).transform);
    }

    private string mPacket;
    private bool mNew = false;
    //== incoming real-time pose data ==--
    public void OnPacketReceived(object sender, string Packet)
    {
        mPacket = Packet;
        mNew = true;
    }

    public void OnSamplePacket(string packet)
    {
        //== hook for "Sample Mocap Data Script" ==--

        mPacket = packet.Replace("Trent", Actor);
        mNew = true;
    }


    //== bone mapping look-up table ==--

    public string Actor = "Trent";
    private string currentActor = "";
    private Dictionary<string, string> mBoneDictionary = new Dictionary<string, string>();
    public void PrepareBoneDictionary()
    {

        mBoneDictionary.Clear();
        mBoneDictionary.Add(Actor + "_Hip", "Hips");
        mBoneDictionary.Add(Actor + "_Ab", "Spine");
        mBoneDictionary.Add(Actor + "_Chest", "Spine1");
        mBoneDictionary.Add(Actor + "_Neck", "Neck");
        mBoneDictionary.Add(Actor + "_Head", "Head");
        mBoneDictionary.Add(Actor + "_LShoulder", "LeftShoulder");
        mBoneDictionary.Add(Actor + "_LUArm", "LeftArm");
        mBoneDictionary.Add(Actor + "_LFArm", "LeftForeArm");
        mBoneDictionary.Add(Actor + "_LHand", "LeftHand");
        mBoneDictionary.Add(Actor + "_RShoulder", "RightShoulder");
        mBoneDictionary.Add(Actor + "_RUArm", "RightArm");
        mBoneDictionary.Add(Actor + "_RFArm", "RightForeArm");
        mBoneDictionary.Add(Actor + "_RHand", "RightHand");
        mBoneDictionary.Add(Actor + "_LThigh", "LeftUpLeg");
        mBoneDictionary.Add(Actor + "_LShin", "LeftLeg");
        mBoneDictionary.Add(Actor + "_LFoot", "LeftFoot");
        mBoneDictionary.Add(Actor + "_RThigh", "RightUpLeg");
        mBoneDictionary.Add(Actor + "_RShin", "RightLeg");
        mBoneDictionary.Add(Actor + "_RFoot", "RightFoot");
        mBoneDictionary.Add(Actor + "_LToe", "LeftToeBase");
        mBoneDictionary.Add(Actor + "_RToe", "RightToeBase");
    }

    private Dictionary<string, string> mBoneToSkeleton = new Dictionary<string, string>();
    public void PrepareBoneToSkeleton()
    {
        mBoneToSkeleton.Clear();
        mBoneToSkeleton.Add("Hips", Actor + "_Hip");        
        mBoneToSkeleton.Add("Spine", Actor + "_Ab");        
        mBoneToSkeleton.Add("Chest", Actor + "_Chest");
        mBoneToSkeleton.Add("Neck", Actor + "_Neck");    
        mBoneToSkeleton.Add("Head", Actor + "_Head");      
        mBoneToSkeleton.Add("LeftShoulder",Actor + "_LShoulder"); 
        mBoneToSkeleton.Add("LeftUpperArm",Actor + "_LUArm");     
        mBoneToSkeleton.Add("LeftLowerArm",Actor + "_LFArm");     
        mBoneToSkeleton.Add("LeftHand",Actor + "_LHand");     
        mBoneToSkeleton.Add("RightShoulder",Actor + "_RShoulder"); 
        mBoneToSkeleton.Add("RightUpperArm",Actor + "_RUArm");     
        mBoneToSkeleton.Add("RightLowerArm",Actor + "_RFArm");     
        mBoneToSkeleton.Add("RightHand",Actor + "_RHand");     
        mBoneToSkeleton.Add("LeftUpperLeg",Actor + "_LThigh");    
        mBoneToSkeleton.Add("LeftLowerLeg",Actor + "_LShin");     
        mBoneToSkeleton.Add("LeftFoot",Actor + "_LFoot");     
        mBoneToSkeleton.Add("RightUpperLeg",Actor + "_RThigh");    
        mBoneToSkeleton.Add("RightLowerLeg",Actor + "_RShin");     
        mBoneToSkeleton.Add("RightFoot",Actor + "_RFoot");     
        mBoneToSkeleton.Add("LeftToeBase",Actor + "_LToe");      
        mBoneToSkeleton.Add("RightToeBase",Actor + "_RToe");      
    }
}

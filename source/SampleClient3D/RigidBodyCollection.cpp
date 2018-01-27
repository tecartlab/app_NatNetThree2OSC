#include "RigidBodyCollection.h"

//////////////////////////////////////////////////////////////////////////
// RigidBodyCollection implementation
//////////////////////////////////////////////////////////////////////////


RigidBodyCollection::RigidBodyCollection()
  :mNumRigidBodies(0)
{
  ;
}

void RigidBodyCollection::AppendRigidBodyData(sRigidBodyData const * const rigidBodyData, size_t numRigidBodies)
{
    for (size_t i = 0; i < numRigidBodies; ++i)
    {
      const sRigidBodyData& rb = rigidBodyData[i];
      mXYZCoord[i + mNumRigidBodies] = std::make_tuple(rb.x, rb.y, rb.z);

      mXYZWQuats[i + mNumRigidBodies] = std::make_tuple(rb.qx, rb.qy, rb.qz, rb.qw);
      mIds[i + mNumRigidBodies] = rb.ID;
    }
    mNumRigidBodies += numRigidBodies;
}

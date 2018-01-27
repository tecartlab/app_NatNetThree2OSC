#include "MarkerPositionCollection.h"

//////////////////////////////////////////////////////////////////////////
// MarkerPositionCollection implementation
//////////////////////////////////////////////////////////////////////////

MarkerPositionCollection::MarkerPositionCollection()
  :mMarkerPositionCount(0)
{
  ;
}

void MarkerPositionCollection::AppendMarkerPositions(float markerData[][3], size_t numMarkers)
{
  for (size_t i = 0; i < numMarkers; ++i)
  {
    mMarkerPositions[i + mMarkerPositionCount] = std::make_tuple(markerData[i][0], markerData[i][1], markerData[i][2]);
  }

  mMarkerPositionCount += numMarkers;
}

void MarkerPositionCollection::AppendLabledMarkers(sMarker markers[], size_t numMarkers)
{
  for (size_t i = 0; i < numMarkers; ++i)
  {
    mLabledMarkers[i + mLabledMarkerCount].ID   = markers[i].ID;
    mLabledMarkers[i + mLabledMarkerCount].x    = markers[i].x;
    mLabledMarkers[i + mLabledMarkerCount].y    = markers[i].y;
    mLabledMarkers[i + mLabledMarkerCount].z    = markers[i].z;
    mLabledMarkers[i + mLabledMarkerCount].size = markers[i].size;
  }

  mLabledMarkerCount += numMarkers;
}
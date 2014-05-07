#ifndef __DASL_VREP_HPP__
#define __DASL_VREP_HPP__

#include <map>
#include <vector>

extern "C" {
#include "extApi.h"
}

/*********************************************************************/
//the main class to interact with the VREP simulator
/*********************************************************************/
class DaslVrep
{
private:
  //the client id
  simxInt clientId;

  //X and Y dimensions
  simxInt xdim,ydim;

  //the floor center coordinate in VREP
  simxFloat floorCenter[3];

  //the min and max X and Y coordinates of the floor in VREP
  simxFloat minx,maxx,miny,maxy;

  //map from nodes created to their target objects
  std::map<simxInt,simxInt> node2Targets;

  //map from nodes to waypoint locations
  typedef std::map<simxInt,std::vector<simxFloat> > TargetMap;
  TargetMap node2Waypoint;

  //map from nodes to the position of their targets
  TargetMap node2TargetPos;

  //debug flag
  bool debug;

  //return true if the argument node is close enough to its target
  bool nodeAtTarget(simxInt nodeId);

  //return true if the argument node's target is close enought to its
  //waypoint
  bool targetAtWaypoint(simxInt nodeId);

public:
  //constructors
  DaslVrep() {}
  DaslVrep(simxInt _xdim,simxInt _ydim);

  simxInt connect(simxChar *ipAddr,simxInt port);
  void disconnect();
  void setDebug(const bool d);
  simxInt createNode();
  simxInt destroyNode(const simxInt nodeId);
  simxInt getNumObjects();
  simxInt getPingTime();
  simxInt placeNodeAt(simxInt nodeId,simxFloat x,simxFloat y,simxFloat z);
  simxInt moveNodeTo(simxInt nodeId,simxFloat x,simxFloat y,simxFloat z);
  simxInt startSim();
  simxInt pauseSim();
  simxInt stopSim();
};

#endif //__DASL_VREP_HPP__

/*********************************************************************/
//end of file
/*********************************************************************/

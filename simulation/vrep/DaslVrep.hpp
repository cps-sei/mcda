#ifndef __DASL_VREP_HPP__
#define __DASL_VREP_HPP__

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

  //map from nodes to target locations
  typedef std::map<simxInt,std::vector<simxFloat> > TargetMap;
  TargetMap targetMap;

  //default constructor is private
  DaslVrep() {}

public:
  //constructors
  DaslVrep(simxInt _xdim,simxInt _ydim);

  simxInt connect(simxChar *ipAddr,simxInt port);
  void disconnect();
  simxInt createNode();
  simxInt destroyNode(const simxInt nodeId);
  simxInt getNumObjects();
  simxInt getPingTime();
  simxInt placeNodeAt(simxInt nodeId,simxFloat x,simxFloat y,simxFloat z);
  simxInt startSim();
  simxInt pauseSim();
  simxInt stopSim();
};

#endif //__DASL_VREP_HPP__

/*********************************************************************/
//end of file
/*********************************************************************/

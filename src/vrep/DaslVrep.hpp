#ifndef __DASL_VREP_HPP__
#define __DASL_VREP_HPP__

#include <map>
#include <vector>
#include "daig/MCDA_Export.h"

extern "C" {
#include "extApi.h"
}

/*********************************************************************/
//the main class to interact with the VREP simulator. this is an
//abstract base class. it must be extended for each type of model.
/*********************************************************************/
class MCDA_Export DaslVrep
{
protected:
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

  //create node given a model file
  simxInt createNode(const std::string &modelFile);

public:
  //constructors
  DaslVrep() {}
  DaslVrep(simxInt _xdim,simxInt _ydim);

  simxInt connect(simxChar *ipAddr,simxInt port);
  void disconnect();
  void setDebug(const bool d);
  virtual simxInt createNode() = 0;
  simxInt destroyNode(const simxInt nodeId);
  simxInt getNumObjects();
  simxInt getPingTime();
  virtual simxInt placeNodeAt(simxInt nodeId,simxFloat x,simxFloat y,simxFloat z);
  virtual simxInt moveNodeTo(simxInt nodeId,simxFloat x,simxFloat y,simxFloat z) = 0;
  virtual simxInt getTargetHandle(simxInt nodeId) = 0;
  simxInt startSim();
  simxInt pauseSim();
  simxInt stopSim();
};

/*********************************************************************/
//the subclass of DaslVrep corresponding to a quadrirotor model.
/*********************************************************************/
class MCDA_Export QuadriRotor : public DaslVrep
{
public:
  QuadriRotor() {}
  QuadriRotor(simxInt _xdim,simxInt _ydim) : DaslVrep(_xdim,_ydim) {}
  simxInt createNode();
  simxInt getTargetHandle(simxInt nodeId);
  simxInt placeNodeAt(simxInt nodeId,simxFloat x,simxFloat y,simxFloat z);
  simxInt moveNodeTo(simxInt nodeId,simxFloat x,simxFloat y,simxFloat z);
};

/*********************************************************************/
//the subclass of DaslVrep corresponding to an ant model.
/*********************************************************************/
class MCDA_Export TrackerAnt : public DaslVrep
{
private:
  const static double antZ;
public:
  TrackerAnt() {}
  TrackerAnt(simxInt _xdim,simxInt _ydim) : DaslVrep(_xdim,_ydim) {}
  simxInt createNode();
  simxInt getTargetHandle(simxInt nodeId);
  simxInt placeNodeAt(simxInt nodeId,simxFloat x,simxFloat y,simxFloat z);
  simxInt moveNodeTo(simxInt nodeId,simxFloat x,simxFloat y,simxFloat z);
};

#endif //__DASL_VREP_HPP__

/*********************************************************************/
//end of file
/*********************************************************************/

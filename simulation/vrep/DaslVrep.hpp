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

public:
  //constructors
  DaslVrep();

  simxInt connect(simxChar *ipAddr,simxInt port);
  void disconnect();
  void setXDim(simxInt _x);
  void setYDim(simxInt _y);
  simxInt createNode();
  simxInt destroyNode(const simxInt nodeId);
  simxInt getNumObjects();
  simxInt getPingTime();
  simxInt placeNodeAt(simxInt nodeId,simxFloat x,simxFloat y,simxFloat z);
};

#endif //__DASL_VREP_HPP__

/*********************************************************************/
//end of file
/*********************************************************************/

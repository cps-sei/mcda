#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "madara/utility/Utility.h"

#include "DaslVrep.hpp"

int main(int argc,char **argv)
{
  if(argc == 1) {
    std::cout << "ERROR: Usage: " << argv[0] << " port-number\n";
    return 1;
  }
  
  simxInt port = atoi(argv[1]);

  std::cout << "opening connection to VREP\n";
  QuadriRotor the_dv(10,10);
  DaslVrep &dv = the_dv;

  if(dv.connect((simxChar*)"127.0.0.1",port) == -1) {
    std::cerr << "ERROR: could not connect to VREP ...\n";
    return 1;
  }
  std::cout << "connected to VREP successfully ...\n";
  std::cout << "scene has " << dv.getNumObjects() << " objects\n";

  std::cout << "ping time is " << dv.getPingTime() << " ms\n";

  Madara::Utility::sleep(2);

  if(port == 19900) {
    simxInt node1 = dv.createNode();
    std::cout << "created robot 1\n";
    std::cout << "scene now has " << dv.getNumObjects() << " objects\n";
    dv.placeNodeAt(node1,0,0,1);

    //dv.startSim(); sleep(5); dv.pauseSim(); sleep(5);
    //dv.startSim(); sleep(5); dv.stopSim();

    dv.startSim();
    while(dv.moveNodeTo(node1,0,10,1));
    while(dv.moveNodeTo(node1,10,10,1));
    while(dv.moveNodeTo(node1,10,0,1));
    while(dv.moveNodeTo(node1,0,0,1));

    dv.stopSim();
    dv.destroyNode(node1);
  } else {
    simxInt node1 = dv.createNode();
    std::cout << "created robot 1\n";
    std::cout << "scene now has " << dv.getNumObjects() << " objects\n";
    dv.placeNodeAt(node1,10,10,1);

    //dv.destroyNode(node1);
  }

  dv.disconnect();
  return 0;
}

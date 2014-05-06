#include <unistd.h>
#include <iostream>

#include "DaslVrep.hpp"

int main()
{
  std::cout << "opening connection to VREP\n";
  DaslVrep dv;
  if(dv.connect() == -1) {
    std::cerr << "ERROR: could not connect to VREP ...\n";
    return 1;
  }
  std::cout << "connected to VREP successfully ...\n";
  std::cout << "scene has " << dv.getNumObjects() << " objects\n";

  std::cout << "ping time is " << dv.getPingTime() << " ms\n";

  sleep(2);
  simxInt model1 = dv.createNode();
  std::cout << "created robot 1\n";
  std::cout << "scene now has " << dv.getNumObjects() << " objects\n";
  dv.moveNode(model1,0,0,0.5);

  dv.destroyNode(model1);

  // sleep(2);

  // simxInt model2 = loadModel(clientId);
  // std::cout << "created robot 2\n";
  // std::cout << "scene now has " << getNumObjects(clientId) << " objects\n";
  // moveModel(clientId,model2,.2,.2,0.0001);

  // sleep(2);
  // simxStartSimulation(clientId,simx_opmode_oneshot_wait);
  // std::cout << "simulation started ...\n";

  // sleep(5);
  // simxPauseSimulation(clientId,simx_opmode_oneshot_wait);
  // std::cout << "simulation paused ...\n";

  // sleep(2);
  // simxStartSimulation(clientId,simx_opmode_oneshot_wait);
  // std::cout << "simulation started ...\n";

  // sleep(5);
  // simxStopSimulation(clientId,simx_opmode_oneshot_wait);
  // std::cout << "simulation stopped ...\n";

  // //sleep(5);
  // deleteModel(clientId,model1);
  // std::cout << "deleted robot1\n";

  // //sleep(5);
  // deleteModel(clientId,model2);
  // std::cout << "deleted robot2\n";

  dv.disconnect();
  return 0;
}

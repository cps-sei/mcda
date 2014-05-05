#include <unistd.h>
#include <iostream>

extern "C" {
#include "extApi.h"
}

//the VREP port number
const int portNb = 19998;

//return the number of objects in the scene
simxInt getNumObjects(simxInt clientId)
{  
  simxInt objCount = 0,*objHandles=0;
  if(simxGetObjects(clientId,sim_object_shape_type,&objCount,&objHandles,simx_opmode_oneshot_wait) != simx_error_noerror) {
    std::cerr << "ERROR : count not get objects ...\n";
    return -1;
  }
  return objCount;
}

//create a hexapod model and return handle.
simxInt loadModel(simxInt clientId)
{
  simxInt baseHandle;
  if(simxLoadModel(clientId,"/home/chaki/stuff/vrep/V-REP_PRO_EDU_V3_1_0_64_Linux/models/robots/mobile/hexapod1.ttm",0,&baseHandle,simx_opmode_oneshot_wait) != simx_error_noerror) {
    std::cerr << "ERROR : count not load model ...\n";
  } else std::cout << "model loaded successfully\n";
  return baseHandle;
}

//delete a hexapod model specified by handle
simxInt deleteModel(simxInt clientId,simxInt handle)
{
  //delete all children
  for(;;) {
    simxInt childHandle = 0;
    simxGetObjectChild(clientId,handle,0,&childHandle,simx_opmode_oneshot_wait);
    if(childHandle == -1) break;
    simxRemoveObject(clientId,childHandle,simx_opmode_oneshot_wait);
  }

  return simxRemoveObject(clientId,handle,simx_opmode_oneshot_wait);
}

//move a model to a new position
simxInt moveModel(simxInt clientId,simxInt model,simxFloat x,simxFloat y,simxFloat z)
{
  simxFloat coord[3] = { x, y, z };
  return simxSetObjectPosition(clientId,model,-1,coord,simx_opmode_oneshot_wait);
}

int main()
{
  std::cout << "opening connection to VREP\n";
  int clientId = simxStart((simxChar*)"127.0.0.1",portNb,true,true,2000,5);

  if(clientId == -1) {
    std::cerr << "ERROR: could not connect to VREP ...\n";
    return 1;
  }

  std::cout << "connected to VREP successfully ...\n";
  std::cout << "scene has " << getNumObjects(clientId) << " objects\n";

  simxInt pingTime = -1;
  simxGetPingTime(clientId,&pingTime);
  std::cout << "ping time is " << pingTime << " ms\n";

  sleep(2);
  simxInt model1 = loadModel(clientId);
  std::cout << "created robot 1\n";
  std::cout << "scene now has " << getNumObjects(clientId) << " objects\n";

  sleep(2);

  simxInt model2 = loadModel(clientId);
  std::cout << "created robot 2\n";
  std::cout << "scene now has " << getNumObjects(clientId) << " objects\n";
  moveModel(clientId,model2,0.5,0.5,0);

  sleep(2);
  simxStartSimulation(clientId,simx_opmode_oneshot_wait);
  std::cout << "simulation started ...\n";

  sleep(5);
  simxPauseSimulation(clientId,simx_opmode_oneshot_wait);
  std::cout << "simulation paused ...\n";

  sleep(2);
  simxStartSimulation(clientId,simx_opmode_oneshot_wait);
  std::cout << "simulation started ...\n";

  sleep(5);
  simxStopSimulation(clientId,simx_opmode_oneshot_wait);
  std::cout << "simulation stopped ...\n";

  sleep(5);
  deleteModel(clientId,model1);
  std::cout << "deleted robot1\n";

  sleep(5);
  deleteModel(clientId,model2);
  std::cout << "deleted robot2\n";

  return 0;
}

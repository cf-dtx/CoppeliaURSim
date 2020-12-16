#pragma once

#include "RobotInterface.h"

#include <vector>

#ifdef _WIN32
#define SIM_DLLEXPORT extern "C" __declspec(dllexport)
#else
#define SIM_DLLEXPORT extern "C"
#endif

// The 3 required entry points of the CoppelisSim plugin:
SIM_DLLEXPORT unsigned char simStart(void *reservedPointer, int reservedInt);
SIM_DLLEXPORT void simEnd();
SIM_DLLEXPORT void *simMessage(int message, int *auxiliaryData, void *customData, int *replyData);

/*!
 * Guaranteeing the Coppelia standard API functions are always called from the VRep main thread.
 *
 * Instead of directly calling this functions from communication threads, the information is
 * passed to queues that are push/poped from the communication or VRep thread.
 *
 * To clear the plugin files, it is created objects that encapsulate information relative to
 * robots, end-effectors or surgical trajectories.
 */

bool simExtGetRobotHandles(std::shared_ptr<Robot> ptr_robot);
bool simExtGetJointLimits(std::shared_ptr<Robot> ptr_robot);

bool simExtGetJointPositions(const std::vector<int> &jhandles, std::vector<double> &pos);
bool simExtGetJointVelocities(const std::vector<int> &jhandles, std::vector<double> &vel);
bool simExtSetJointPositions(const std::vector<int> &jhandles, const std::vector<double> &dpos);

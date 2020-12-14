#include "RobotInterface.h"

#include <algorithm>
#include <iostream>

using namespace std;

RobotInterface::RobotInterface()
{
    ur10.name = "UR10e";
    ur10.dofs = 6;
}

void RobotInterface::InitRobot()
{
    //Get Robot Handles
    //simExtGetRobotHandles(ur10);
    //Get Joint Limits - only get limits for 1 robot since they are identical (real and hollow)
    //simExtGetJointLimits(ur10);
}

void RobotInterface::GetJointPositions(std::vector<double> &pos)
{
    //simExtGetJointPositions(ur10.jhandles, pos);
}

void RobotInterface::GetJointVelocities(std::vector<double> &vel)
{
    //simExtGetJointVelocities(ur10.jhandles, vel);
}

void RobotInterface::SetJointPositions(const std::vector<double> &dpos)
{
    //simExtSetJointPositions(ur10.jhandles, dpos, false);
}

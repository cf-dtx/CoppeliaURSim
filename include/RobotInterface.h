#ifndef SIM_EXT_RTDE_ROBOT_INTERFACE_H
#define SIM_EXT_RTDE_ROBOT_INTERFACE_H

#include <map>
#include <string>
#include <vector>

//Robot
struct Robot {
    int handle;
    std::string name;
    int dofs;
    //bool hollow;
    std::vector<int> jhandles;
    std::vector< std::pair<double, double> > jplim;
    std::vector<double> jvlim;
};

class RobotInterface {
public:
    RobotInterface();

    void InitRobot();

    void GetJointPositions(std::vector<double> &pos);
    void GetJointVelocities(std::vector<double> &vel);
    void SetJointPositions(const std::vector<double> &dpos);

public:
    Robot ur10;
};

#endif // SIM_EXT_RTDE_ROBOT_INTERFACE_H

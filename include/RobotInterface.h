#ifndef SIM_EXT_RTDE_ROBOT_INTERFACE_H
#define SIM_EXT_RTDE_ROBOT_INTERFACE_H

#include <map>
#include <queue>
#include <string>
#include <vector>
#include <memory>

#include "basic_concurrent_queue.h"
#include "vector_concurrent_queue.h"

//Robot
struct Robot
{
  int handle;
  std::string name;
  int dofs;
  //bool hollow;
  std::vector<int> jhandles;
  std::vector<std::pair<double, double>> jplim;
  std::vector<double> jvlim;
};

struct RTDEInterface
{
  size_t buffer_size = 5;
  VectorConcurrentQueue<double> current_joint_positions;
  VectorConcurrentQueue<bool> current_io;
  BasicConcurrentQueue<int> target_io;
};

class RobotInterface
{
public:
  RobotInterface();

  void InitRobot();

  void GetJointPositions(std::vector<double> &pos);
  void GetJointVelocities(std::vector<double> &vel);
  void SetJointPositions(const std::vector<double> &dpos);

public:
  std::shared_ptr<Robot> m_ur10;
  std::shared_ptr<RTDEInterface> m_rtde;
};

#endif // SIM_EXT_RTDE_ROBOT_INTERFACE_H

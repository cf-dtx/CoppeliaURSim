#ifndef RTDE_CLIENT_H
#define RTDE_CLIENT_H

// General purpose
#include <vector>
#include <string>
#include <memory>
#include <atomic>
#include <map>
#include <mutex>

// Connection and Network
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

// Connection to UR using RTDE inferface
#include <ur_rtde/rtde.h>
#include <ur_rtde/rtde_control_interface.h>
#include <ur_rtde/rtde_receive_interface.h>
#include <ur_rtde/rtde_io_interface.h>

//#include "vector_concurrent_queue.h"

class RTDEclient
{
public:
  RTDEclient();

private:
  std::string getIP();

public:
  bool getJoints(std::vector<double> &q);
  bool getDigitalIO(uint8_t io_id);
  void setDigitalIO(uint8_t io_id, bool signal);

private:
  std::string m_ip;
  std::unique_ptr<ur_rtde::RTDEReceiveInterface> m_ircv;
  // std::unique_ptr<ur_rtde::RTDEControlInterface> m_ictrl;
  std::unique_ptr<ur_rtde::RTDEIOInterface> m_iio;
};

#endif //RTDE_CLIENT_H

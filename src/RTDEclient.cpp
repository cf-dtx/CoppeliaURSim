#include "RTDEclient.h"

using namespace ur_rtde;
using namespace std;

RTDEclient::RTDEclient(std::shared_ptr<RTDEInterface> irtde)
{
  m_rtde = irtde;
  m_period = 0.025; // 25ms
}

RTDEclient::~RTDEclient()
{
  RTDEdisconnect();
}

bool RTDEclient::RTDEconnect()
{
  // Obtain current wireless IP
  m_ip = getIP();
  // Initiate clients for the Receive, Control and IO interfaces
  m_ircv = std::make_unique<RTDEReceiveInterface>(m_ip);
  // m_ictrl = std::make_unique<RTDEControlInterface>(m_ip);
  m_iio = std::make_unique<RTDEIOInterface>(m_ip);

  // launch thread to handle communication with UR_RTDE
  m_alive = true;
  m_thr = std::async(std::launch::async, &RTDEclient::run, this);

  return m_ircv->isConnected();
}

bool RTDEclient::RTDEdisconnect()
{
  m_alive = false;

  m_iio.reset();
  m_ircv.reset();

  return true;
}

string RTDEclient::getIP()
{
  // Get IP
  int fd;
  struct ifreq ifr;
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  /* I want to get an IPv4 IP address */
  ifr.ifr_addr.sa_family = AF_INET;
  /* I want IP address attached to "wlp5s0" */
  // TODO: script to obtain the wireless interface name automatically
  strncpy(ifr.ifr_name, "wlp4s0", IFNAMSIZ - 1);
  //strncpy(ifr.ifr_name, "enp37s0", IFNAMSIZ-1);
  ioctl(fd, SIOCGIFADDR, &ifr);
  close(fd);

  return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

void RTDEclient::run()
{
  while (m_alive)
  {
    if (m_ircv->isConnected())
    {
      // Update current joint positions
      std::vector<double> q = m_ircv->getActualQ();
      m_rtde->current_joint_positions.push_back(q);
      while (m_rtde->current_joint_positions.size() > m_rtde->buffer_size)
        m_rtde->current_joint_positions.pop_front();

      // Update current ios
      std::vector<bool> io(8);
      for (size_t i = 0; i < 8; ++i)
      {
        io[i] = m_ircv->getDigitalOutState(i);
      }
      m_rtde->current_io.push_back(io);
      while (m_rtde->current_io.size() > m_rtde->buffer_size)
        m_rtde->current_io.pop_front();

      // Set target ios
      while (!m_rtde->target_io.empty())
      {
        m_iio->setStandardDigitalOut(m_rtde->target_io.back(), true);
        m_rtde->target_io.pop_back();
      }
    }
    else
    {
      m_alive = false;
    }
  }
}

bool RTDEclient::getJoints(std::vector<double> &q)
{
  q = m_rtde->current_joint_positions.back();

  return true;
}

bool RTDEclient::getDigitalIO(uint8_t io_id)
{
  return m_rtde->current_io.back()[io_id];
}

void RTDEclient::setDigitalIO(uint8_t io_id, bool signal)
{
  // TODO: At the moment we can only set HI value on IO
  m_rtde->target_io.push_back(io_id);
  // m_iio->setStandardDigitalOut(io_id, signal);
}

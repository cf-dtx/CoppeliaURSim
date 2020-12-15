#include "RTDEclient.h"

using namespace ur_rtde;
using namespace std;

RTDEclient::RTDEclient()
{
  // Obtain current wireless IP
  m_ip = getIP();

  // Initiate clients for the Receive, Control and IO interfaces
  m_ircv = std::make_unique<RTDEReceiveInterface>(m_ip);
  // m_ictrl = std::make_unique<RTDEControlInterface>(m_ip);
  m_iio = std::make_unique<RTDEIOInterface>(m_ip);
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

bool RTDEclient::getJoints(std::vector<double> &q)
{
  if (m_ircv->isConnected())
  {
    q.clear();

    q = m_ircv->getActualQ();

    return true;
  }
  return false;
}

bool RTDEclient::getDigitalIO(uint8_t io_id)
{
  return m_ircv->getDigitalOutState(io_id);
}

void RTDEclient::setDigitalIO(uint8_t io_id, bool signal)
{
  m_iio->setStandardDigitalOut(io_id, signal);
}

#ifndef __CONGESTIONCONTROL_H__
#define __CONGESTIONCONTROL_H__

#include <stdint.h>
#include <thread>
#include <mutex>
#include "json.hpp"
using json = nlohmann::json;

#define INIT_SSTHRESH 16


enum congestState_c { CONGEST_OPEN, CONGEST_RECOVERY };

class congestionControl_c {
public:
  uint16_t getCwnd() { return (uint16_t) cwnd + numDupack; }

  bool dupack(uint32_t ack);
  bool successTransfer(uint32_t ack);
  void RTO();
  void checkRecovery(uint32_t ack);


  void startWriteToCwndLog() {
    writeToCwndLog = std::thread(&congestionControl_c::writeToCwndLogThread, this);
  }
  void dumpCwndLog() {
    cwndLog_mutex.lock();
    FILE *cwndLogFile = fopen("cwndLog.json","w");
    fprintf(cwndLogFile, "%s", cwndLog.dump().c_str());
    fclose(cwndLogFile);
    cwndLog_mutex.unlock();
  }





private:
  congestState_c congestState = CONGEST_OPEN;
  uint16_t ssthresh = INIT_SSTHRESH;
  float cwnd = 1;

  uint32_t numDupack = 0;
  uint32_t recoveryPoint;
  bool alradyRetrans = false;
  //TODO: is inflisht really correct?

  void writeToCwndLogThread();
  std::thread writeToCwndLog;
  std::mutex cwndLog_mutex;
  json cwndLog;








};

#endif

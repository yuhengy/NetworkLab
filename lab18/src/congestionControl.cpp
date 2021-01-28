#include "congestionControl.h"
#include <unistd.h>
#include <sys/time.h>
#include <list>




bool congestionControl_c::dupack(uint32_t ack)
{
  numDupack++;
  if (congestState == CONGEST_OPEN) {
    if (numDupack == 3) {
      congestState = CONGEST_RECOVERY;
      recoveryPoint = ack;
      ssthresh = cwnd / 2;
      cwnd = cwnd / 2;
    }
  }

  if (congestState == CONGEST_RECOVERY && !alradyRetrans) {
    alradyRetrans = true;
    return true;
  }
  else {
    return false;
  }
}


bool congestionControl_c::successTransfer(uint32_t ack)
{
  numDupack = 0;
  if (cwnd < ssthresh) {
    cwnd += 1;
  }
  else {
    cwnd += 1 / cwnd;
  }

  if ((congestState == CONGEST_RECOVERY) && (ack < recoveryPoint)) {
    return true;
  }
  else {
    alradyRetrans = false;
    return false;
  }
}


void congestionControl_c::RTO()
{
  printf("RTO!!!\n");
  congestState = CONGEST_OPEN;
  ssthresh = cwnd / 2;
  cwnd = 1;
  numDupack = 0;
}


void congestionControl_c::checkRecovery(uint32_t ack)
{
  if (congestState == CONGEST_RECOVERY && ack > recoveryPoint) {
    congestState = CONGEST_OPEN;
    numDupack = 0;
  }
}






//-------------------------------------------------------
void congestionControl_c::writeToCwndLogThread()
{
  float time_use;
  struct timeval start, now;
  gettimeofday(&start,NULL);

  std::list<float> enptyList1;
  std::list<uint16_t> enptyList2;
  cwndLog["timeStamp"] = enptyList1;
  cwndLog["cwnd"] = enptyList2;

  while (true) {
    usleep(100);

    gettimeofday(&now,NULL);
    time_use=(now.tv_sec-start.tv_sec)*1000000+(now.tv_usec-start.tv_usec);
    cwndLog_mutex.lock();
    cwndLog["timeStamp"].push_back(time_use);
    cwndLog["cwnd"].push_back((uint16_t)cwnd);
    cwndLog_mutex.unlock();

  }
}



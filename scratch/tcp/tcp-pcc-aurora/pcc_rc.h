
#ifndef _PCC_RC_H_
#define _PCC_RC_H_

#include "monitoring-interval.h"

class PccRateController {
  public:
    PccRateController() {};
    virtual ~PccRateController() {};
    virtual ns3::DataRate GetNextSendingRate(ns3::DataRate current_rate, ns3::Time cur_time) = 0;
    virtual void MonitorIntervalFinished(const ns3::MonitorInterval& mi) = 0;
    virtual void Reset() {std::cout << "DEFAULT RESET CALLED" << std::endl; };
};

#endif
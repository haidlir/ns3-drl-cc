#ifndef _PCC_CUSTOM_RC_H_
#define _PCC_CUSTOM_RC_H_

#include "pcc_rc.h"

class PccCustomRateController : public PccRateController {
    public:
        PccCustomRateController(double call_freq);
        ~PccCustomRateController() {};

        ns3::DataRate GetNextSendingRate(ns3::DataRate current_rate, ns3::Time cur_time);
        void MonitorIntervalFinished(const ns3::MonitorInterval& mi);

        void Reset();
};

#endif
#ifndef _PCC_CUSTOM_RC_H_
#define _PCC_CUSTOM_RC_H_

#include "pcc_rc.h"
#include "ns3/opengym-module.h"

#include <vector>
#include <deque>

namespace ns3 {
    
class States
{
    public:
        States();
        void AddState(float num);
        std::vector<float> ToVector();
        void UpdateReward(float reward);
        float GetLastReward() {return m_last_reward;}
    private:
        uint32_t m_size;
        std::deque<float> m_queue;
        float m_last_reward {0.0};
};

class PccCustomRateController : public PccRateController, public OpenGymEnv
{
    public:
        PccCustomRateController(double call_freq);
        ~PccCustomRateController() {};

        ns3::DataRate GetNextSendingRate(ns3::DataRate current_rate, ns3::Time cur_time);
        void MonitorIntervalFinished(const ns3::MonitorInterval& mi);

        void Reset();

        // OpenGym interface
        Ptr<OpenGymSpace> GetActionSpace();
        bool GetGameOver();
        float GetReward();
        std::string GetExtraInfo();
        bool ExecuteActions(Ptr<OpenGymDataContainer> action);

        Ptr<OpenGymSpace> GetObservationSpace();
        Ptr<OpenGymDataContainer> GetObservation();

    private:
        double m_alpha_sending_rate;
        States m_states;
};

}

#endif
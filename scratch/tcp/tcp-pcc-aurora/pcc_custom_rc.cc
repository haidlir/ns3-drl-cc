
#include "pcc_custom_rc.h"
#include <time.h>
#include <random>

PccCustomRateController::PccCustomRateController(double call_freq)
{
    // std::cout << "Starting Custom Rate Controller" << std::endl;
}

void PccCustomRateController::Reset()
{
    // std::cout << "Starting Reset" << std::endl;
}

void PccCustomRateController::MonitorIntervalFinished(const ns3::MonitorInterval& mi)
{
    // std::cout << "stats: ";
    // std::cout << mi.GetBytesSent() << " | ";
    // std::cout << mi.GetBytesAcked() << " | ";
    // std::cout << mi.GetBytesLost() << " | ";
    // std::cout << mi.GetSendStartTime() << " | ";
    // std::cout << mi.GetSendEndTime() << " | ";
    // std::cout << mi.GetRecvStartTime() << " | ";
    // std::cout << mi.GetRecvEndTime() << " | ";
    // std::cout << mi.GetFirstAckLatency() << " | ";
    // std::cout << mi.GetLastAckLatency() << " | ";
    // std::cout << mi.GetAveragePacketSize() << " | ";
    // std::cout << mi.GetUtility() << "\n";

    // if (!has_time_offset) {
    //     time_offset_usec = mi.GetSendStartTime();
    //     has_time_offset = true;
    // }
    // GiveSample(
    //     mi.GetBytesSent(),
    //     mi.GetBytesAcked(),
    //     mi.GetBytesLost(),
    //     (mi.GetSendStartTime() - time_offset_usec) / (double)USEC_PER_SEC,
    //     (mi.GetSendEndTime() - time_offset_usec) / (double)USEC_PER_SEC,
    //     (mi.GetRecvStartTime() - time_offset_usec) / (double)USEC_PER_SEC,
    //     (mi.GetRecvEndTime() - time_offset_usec) / (double)USEC_PER_SEC,
    //     mi.GetFirstAckLatency() / (double)USEC_PER_SEC,
    //     mi.GetLastAckLatency() / (double)USEC_PER_SEC,
    //     mi.GetAveragePacketSize(),
    //     mi.GetUtility()
    // );
}

ns3::DataRate PccCustomRateController::GetNextSendingRate(ns3::DataRate current_rate, ns3::Time cur_time) {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    
    double mean = 0;
    double std_var = 0.25;
    std::normal_distribution<> d{mean, std_var};
    // Burn In
    // for (int n = 0; n != 1000; ++n)
    //     d(gen);
    double ratio = d(gen);
    // std::cout << "ratio: " << ratio << std::endl;
    auto new_sending_rate = current_rate * (1 + ratio);
    return new_sending_rate;
}
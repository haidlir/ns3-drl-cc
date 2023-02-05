#ifndef THIRD_PARTY_PCC_QUIC_PCC_MONITOR_INTERVAL_H_
#define THIRD_PARTY_PCC_QUIC_PCC_MONITOR_INTERVAL_H_

#include <utility>
#include <deque>
#include <vector>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cmath>

#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/sequence-number.h"

// #ifndef TCPCONGESTIONOPS_CUSTOM_H
// #define TCPCONGESTIONOPS_CUSTOM_H

namespace ns3
{

// PacketRttSample, stores the packet number and its corresponding RTT
class PacketRttSample {
  public:
    PacketRttSample();
    PacketRttSample(SequenceNumber32 packet_number, Time rtt);
    ~PacketRttSample() {}

    // Packet number of the sampled packet.
    SequenceNumber32 packet_number;
    // RTT corresponding to the sampled packet.
    Time rtt;
};

// MonitorInterval, as the queue's entry struct, stores the information
// of a PCC monitor interval (MonitorInterval) that can be used to
// - pinpoint a acked/lost packet to the corresponding MonitorInterval,
// - calculate the MonitorInterval's utility value.
class MonitorInterval
{
  //friend class MonitorIntervalMetric;
  public:
    MonitorInterval(DataRate sending_rate, Time end_time);

    ~MonitorInterval() {}

    void OnPacketSent(Time cur_time, SequenceNumber32 packet_num, uint32_t packet_size);
    void OnPacketAcked(Time cur_time, SequenceNumber32 packet_num, uint32_t packet_size, Time rtt);
    void OnPacketLost(Time cur_time, SequenceNumber32 packet_num, uint32_t packet_size);

    bool AllPacketsSent(Time cur_time) const;
    bool AllPacketsAccountedFor(Time cur_time);

    DataRate GetTargetSendingRate() const;
    Time GetStartTime() const;

    void SetUtility(double utility);
    
    double GetObsThroughput() const;
    double GetObsSendingRate() const;
    double GetObsSendDur() const;
    double GetObsRecvDur() const;
    double GetObsRtt() const;
    double GetObsRttInflation() const;
    double GetObsLossRate() const;
    double GetObsUtility() const;

    int GetId() const { return id; }

    uint32_t GetBytesSent() const { return bytes_sent; } 
    uint32_t GetBytesAcked() const { return bytes_acked; } 
    uint32_t GetBytesLost() const { return bytes_lost; } 

    Time GetSendStartTime() const { return first_packet_sent_time; }
    Time GetSendEndTime() const { return last_packet_sent_time; }
    Time GetRecvStartTime() const { return first_packet_ack_time; }
    Time GetRecvEndTime() const { return last_packet_ack_time; }

    double GetFirstAckLatency() const;
    double GetLastAckLatency() const;

    uint32_t GetAveragePacketSize() const { return bytes_sent / n_packets_sent; }
    double GetUtility() const { return utility; }

    // Additional NS3 for testing
    Time GetEndTime() const { return end_time; }
    void SetEndTime(Time end_time);

  private:
    int next_id{0};

    bool ContainsPacket(SequenceNumber32 packet_num);

    int id;

    // Sending rate.
    DataRate target_sending_rate;
    // The end time for this monitor interval in microseconds.
    Time end_time;

    // Sent time of the first packet.
    Time first_packet_sent_time;
    // Sent time of the last packet.
    Time last_packet_sent_time;

    // Sent time of the first packet.
    Time first_packet_ack_time;
    // Sent time of the last packet.
    Time last_packet_ack_time;

    // PacketNumber of the first sent packet.
    SequenceNumber32 first_packet_number;
    // PacketNumber of the last sent packet.
    SequenceNumber32 last_packet_number;
    // PacketNumber of the last packet whose status is known (acked/lost).
    SequenceNumber32 last_packet_number_accounted_for;

    // Number of bytes which are sent in total.
    uint32_t bytes_sent;
    // Number of bytes which have been acked.
    uint32_t bytes_acked;
    // Number of bytes which are considered as lost.
    uint32_t bytes_lost;

    // Utility value of this MonitorInterval, which is calculated
    // when all sent packets are accounted for.
    double utility;

    // The number of packets in this monitor interval.
    uint32_t n_packets_sent;

    // The number of packets whose return status is known.
    uint32_t n_packets_accounted_for;

    // A sample of the RTT for each packet.
    std::vector<PacketRttSample> packet_rtt_samples;
};

// PccMonitorIntervalQueue contains a queue of MonitorIntervals.
// New MonitorIntervals are added to the tail of the queue.
// Existing MonitorIntervals are removed from the queue when all
// 'useful' intervals' utilities are available.
class PccMonitorIntervalQueue {
  public:
    explicit PccMonitorIntervalQueue();
    PccMonitorIntervalQueue(const PccMonitorIntervalQueue&) = delete;
    PccMonitorIntervalQueue& operator=(const PccMonitorIntervalQueue&) = delete;
    PccMonitorIntervalQueue(PccMonitorIntervalQueue&&) = delete;
    PccMonitorIntervalQueue& operator=(PccMonitorIntervalQueue&&) = delete;
    ~PccMonitorIntervalQueue();

    // Creates a new MonitorInterval and add it to the tail of the
    // monitor interval queue, provided the necessary variables
    // for MonitorInterval initialization.
    void Push(MonitorInterval mi);

    // Called when a packet belonging to current monitor interval is sent.
    void OnPacketSent(Time sent_time,
                      SequenceNumber32 packet_number,
                      uint32_t bytes);

    // Called when a packet belonging to current monitor interval is acked.
    void OnPacketAcked(Time sent_time,
                      SequenceNumber32 packet_number,
                      uint32_t bytes_acked,
                      Time rtt);

    // Called when a packet belonging to current monitor interval is lost.
    void OnPacketLost(Time sent_time,
                      SequenceNumber32 packet_number,
                      uint32_t bytes_lost);

    // Called when packets are acked or considered as lost.
    // void OnCongestionEvent(const AckedPacketVector& acked_packets,
    //                       const LostPacketVector& lost_packets,
    //                       int64_t rtt_us,
    //                       Time event_time);

    // Returns the most recent MonitorInterval in the tail of the queue
    const MonitorInterval& Current() const;
    bool HasFinishedInterval(Time cur_time);
    MonitorInterval Pop();
    
    bool Empty() const;
    size_t Size() const;

  private:
    std::deque<MonitorInterval> m_monitor_intervals;
};

double CalculateUtility(MonitorInterval const &cur_mi);
    
} // namespace ns3

#endif
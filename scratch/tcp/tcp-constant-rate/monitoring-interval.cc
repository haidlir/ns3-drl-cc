#include "monitoring-interval.h"

#include <iostream>

namespace
{

// Number of probing MonitorIntervals necessary for Probing.
//const size_t kRoundsPerProbing = 4;
// Tolerance of loss rate by utility function.
const float kLossTolerance = 0.05f;
// Coefficeint of the loss rate term in utility function.
const float kLossCoefficient = -1000.0f;
// Coefficient of RTT term in utility function.
const float kRTTCoefficient = -200.0f;
// Coefficienty of the latency term in the utility function.
const float kLatencyCoefficient = 1;
// Alpha factor in the utility function.
const float kAlpha = 1;
// An exponent in the utility function.
const float kExponent = 0.9;
// An exponent in the utility function.
const size_t kMegabit = 1024 * 1024;

}  // namespace

namespace ns3
{

PacketRttSample::PacketRttSample()
    : packet_number(0), rtt(0)
{
}

PacketRttSample::PacketRttSample(SequenceNumber32 packet_number, Time rtt)
    : packet_number(packet_number), rtt(rtt)
{
}

PccMonitorIntervalQueue::PccMonitorIntervalQueue() {}

PccMonitorIntervalQueue::~PccMonitorIntervalQueue() {}

void PccMonitorIntervalQueue::Push(MonitorInterval mi) {
    monitor_intervals_.emplace_back(mi);
}

MonitorInterval PccMonitorIntervalQueue::Pop() {
    MonitorInterval mi = monitor_intervals_.front();
    monitor_intervals_.pop_front();
    return mi;
}

bool PccMonitorIntervalQueue::HasFinishedInterval(Time cur_time) {
    if (monitor_intervals_.empty()) {
        return false;
    }
    return monitor_intervals_.front().AllPacketsAccountedFor(cur_time);
}

void PccMonitorIntervalQueue::OnPacketSent(Time sent_time,
                                           SequenceNumber32 packet_number,
                                           uint32_t bytes) {
  if (monitor_intervals_.empty()) {
    return;
  }

  MonitorInterval& interval = monitor_intervals_.back();
  interval.OnPacketSent(sent_time, packet_number, bytes);
}

// void PccMonitorIntervalQueue::OnCongestionEvent(
//     const AckedPacketVector& acked_packets,
//     const LostPacketVector& lost_packets,
//     int64_t rtt_us,
//     Time event_time) {

//   if (monitor_intervals_.empty()) {
//     // Skip all the received packets if we have no monitor intervals.
//     return;
//   }

//   for (MonitorInterval& interval : monitor_intervals_) {
//     if (interval.AllPacketsAccountedFor(event_time)) {
//       // Skips intervals that have available utilities.
//       continue;
//     }

//     for (const AckedPacket& acked_packet : acked_packets) {
//       interval.OnPacketAcked(event_time, acked_packet.packet_number, acked_packet.bytes_acked, rtt_us);
//     }

//     for (const LostPacket& lost_packet : lost_packets) {
//       interval.OnPacketLost(event_time, lost_packet.packet_number, lost_packet.bytes_lost);
//     }
//   }
// }

const MonitorInterval& PccMonitorIntervalQueue::Current() const {
  return monitor_intervals_.back();
}

bool PccMonitorIntervalQueue::Empty() const {
  return monitor_intervals_.empty();
}

size_t PccMonitorIntervalQueue::Size() const {
  return monitor_intervals_.size();
}

MonitorInterval::MonitorInterval(DataRate sending_rate, Time end_time) {
    this->target_sending_rate = sending_rate;
    this->end_time = end_time;
    bytes_sent = 0;
    bytes_acked = 0;
    bytes_lost = 0;
    n_packets_sent = 0;
    n_packets_accounted_for = 0;
    first_packet_ack_time = Time();
    last_packet_ack_time = Time();
    // id = next_id;
    // ++next_id;
}

void MonitorInterval::OnPacketSent(Time cur_time, SequenceNumber32 packet_number, uint32_t packet_size) {
    if (n_packets_sent == 0) {
        first_packet_sent_time = cur_time;
        first_packet_number = packet_number;
        last_packet_number_accounted_for = first_packet_number - 1;
        //std::cerr << "MI " << id << " started with " << packet_number << ", dur " << (end_time - cur_time) << std::endl; 
    }
    //std::cerr << "MI " << id << " got packet " << packet_number << std::endl;
    //std::cerr << "\tSent: " << packet_number << ", dur " << (end_time - cur_time) << std::endl; 
    //std::cerr << "\tTime: " << cur_time << std::endl; 
    last_packet_sent_time = cur_time;
    last_packet_number = packet_number;
    ++n_packets_sent;
    bytes_sent += packet_size;
    // std::cout << "Mi::OnPacketSent " << "SeqNum: " << packet_number <<  " ";
    // std::cout << "PacketSize: " << packet_size <<  std::endl;
}

void MonitorInterval::OnPacketAcked(Time cur_time, SequenceNumber32 packet_number, uint32_t packet_size, Time rtt) {
    // Below code is disabled, since the TCP's SeqNum mechanism is different to QUIC's PNs.
    // if (ContainsPacket(packet_number) && packet_number > last_packet_number_accounted_for) {
    if (ContainsPacket(packet_number)) {
        int skipped = (packet_number - last_packet_number_accounted_for) - 1;
        bytes_acked += packet_size;
        n_packets_accounted_for += skipped + 1;
        packet_rtt_samples.push_back(PacketRttSample(packet_number, rtt));
        last_packet_number_accounted_for = packet_number;
        last_packet_ack_time = cur_time;
        //std::cerr << "MI " << id << " got ack " << packet_number << std::endl;
        //std::cerr << "\tAck time: " << cur_time << std::endl; 
    } else if (packet_number > last_packet_number) {
        n_packets_accounted_for = n_packets_sent;
        last_packet_number_accounted_for = last_packet_number;
    }
    if (packet_number >= first_packet_number && first_packet_ack_time.IsZero()) {
        first_packet_ack_time = cur_time;
        //std::cerr << "MI " << id << " first ack " << packet_number << std::endl;
        //std::cerr << "\tAck time: " << cur_time << std::endl; 
    }
    if (packet_number >= last_packet_number && last_packet_ack_time.IsZero()) {
        last_packet_ack_time = cur_time;
        //std::cerr << "MI " << id << " last ack " << packet_number << std::endl;
        //std::cerr << "\tAck time: " << cur_time << std::endl; 
    }
    if (AllPacketsAccountedFor(cur_time)) {
        //std::cout << "MI " << id << " [" << first_packet_number << ", " << last_packet_number << "] finished at packet " << packet_number << std::endl; 
    }
    static int count = 1;
    count++;
    // std::cout << "Mi::OnPacketAcked " << "SeqNum: " << packet_number <<  " ";
    // std::cout << "PacketSize: " << packet_size <<  std::endl;
}

void MonitorInterval::OnPacketLost(Time cur_time, SequenceNumber32 packet_number, uint32_t packet_size) {
    // Below code is disabled, since the TCP's SeqNum mechanism is different to QUIC's PNs.
    // if (ContainsPacket(packet_number) && packet_number > last_packet_number_accounted_for) {
    if (ContainsPacket(packet_number)) {
        int skipped = (packet_number - last_packet_number_accounted_for) - 1;
        bytes_lost += packet_size;
        n_packets_accounted_for += skipped + 1;
        last_packet_number_accounted_for = packet_number;
    } else if (packet_number > last_packet_number) {
        n_packets_accounted_for = n_packets_sent;
        last_packet_number_accounted_for = last_packet_number;
    }
    if (packet_number >= first_packet_number && first_packet_ack_time.IsZero()) {
        first_packet_ack_time = cur_time;
    }
    if (packet_number >= last_packet_number && last_packet_ack_time.IsZero()) {
        last_packet_ack_time = cur_time;
    }
    if (AllPacketsAccountedFor(cur_time)) {
        //std::cout << "MI [" << first_packet_number << ", " << last_packet_number << "] finished at packet " << packet_number << std::endl; 
    }
}

bool MonitorInterval::AllPacketsSent(Time cur_time) const {
    //std::cout << "Checking if all packets sent: " << cur_time << " >= " << end_time << std::endl;
    return (cur_time >= end_time);
}

bool MonitorInterval::AllPacketsAccountedFor(Time cur_time) {
    return AllPacketsSent(cur_time) && (n_packets_accounted_for == n_packets_sent);
}

Time MonitorInterval::GetStartTime() const {
    return first_packet_sent_time;
}

DataRate MonitorInterval::GetTargetSendingRate() const {
    return target_sending_rate;
}

float MonitorInterval::GetObsThroughput() const {
    float dur = GetObsRecvDur();
    //std::cout << "Num packets: " << n_packets_sent << std::endl;
    //std::cout << "Dur: " << dur << " Acked: " << bytes_acked << std::endl;
    if (dur == 0) {
        return 0;
    }
    return 8 * bytes_acked / (dur / 1000000.0);
}

float MonitorInterval::GetObsSendingRate() const {
    float dur = GetObsSendDur();
    if (dur == 0) {
        return 0;
    }
    return 8 * bytes_sent / (dur / 1000000.0);
}

float MonitorInterval::GetObsSendDur() const {
    return (last_packet_sent_time.GetMicroSeconds() - first_packet_sent_time.GetMicroSeconds());
}

float MonitorInterval::GetObsRecvDur() const {
    //std::cerr << "MI " << id << "\n";
    //std::cerr << "\tfirst ack " << first_packet_ack_time << "\n\tlast ack " << last_packet_ack_time << "\n";
    return (last_packet_ack_time.GetMicroSeconds() - first_packet_ack_time.GetMicroSeconds());
}

float MonitorInterval::GetObsRtt() const {
    if (packet_rtt_samples.empty()) {
        return 0;
    }
    double rtt_sum = 0.0;
    for (const PacketRttSample& sample : packet_rtt_samples) {
        rtt_sum += sample.rtt.GetSeconds();
    }
    return rtt_sum / packet_rtt_samples.size();
}

float MonitorInterval::GetObsRttInflation() const {
    if (packet_rtt_samples.size() < 2) {
        return 0;
    }
    float send_dur = GetObsSendDur();
    if (send_dur == 0) {
        return 0;
    }
    float first_half_rtt_sum = 0;
    float second_half_rtt_sum = 0;
    int half_count = packet_rtt_samples.size() / 2;
    for (int i = 0; i < 2 * half_count; ++i) {
        if (i < half_count) {
            first_half_rtt_sum += packet_rtt_samples[i].rtt.GetMicroSeconds();
        } else {
            second_half_rtt_sum += packet_rtt_samples[i].rtt.GetMicroSeconds();
        }
    }
    float rtt_inflation = (second_half_rtt_sum - first_half_rtt_sum) / (half_count * send_dur);
    return rtt_inflation;
}

float MonitorInterval::GetObsLossRate() const {
    return 1.0 - (bytes_acked / (float)bytes_sent);
}

void MonitorInterval::SetUtility(float new_utility) {
    utility = new_utility;
}

float MonitorInterval::GetObsUtility() const {
    return utility;
}

uint64_t MonitorInterval::GetFirstAckLatency() const {
    if (packet_rtt_samples.size() > 0) {
        return packet_rtt_samples.front().rtt.GetSeconds();
    }
    return 0;
}

uint64_t MonitorInterval::GetLastAckLatency() const {
    if (packet_rtt_samples.size() > 0) {
        return packet_rtt_samples.back().rtt.GetSeconds();
    }
    return 0;
}

bool MonitorInterval::ContainsPacket(SequenceNumber32 packet_number) {
    return (packet_number >= first_packet_number && packet_number <= last_packet_number);
}


float CalculateUtility(MonitorInterval const &cur_mi) {

  float throughput = cur_mi.GetObsThroughput();
  float sending_rate_mbps = cur_mi.GetObsSendingRate() * 1e-6;
  float rtt_inflation = cur_mi.GetObsRttInflation(); 
  float avg_rtt = cur_mi.GetObsRtt();
  float loss_rate = cur_mi.GetObsLossRate();
  
  float rtt_contribution = 900 * rtt_inflation;
  float loss_contribution = 11.35 * loss_rate;
  float sending_factor = kAlpha * pow(sending_rate_mbps, kExponent);
  loss_contribution *= -1.0 * sending_rate_mbps;
  rtt_contribution *= -1.0 * sending_rate_mbps;
  
  float utility = sending_factor + loss_contribution + rtt_contribution;
    
  return utility;
}

} // namespace ns3
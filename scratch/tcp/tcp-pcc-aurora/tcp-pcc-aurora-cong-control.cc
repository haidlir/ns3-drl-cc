/*
 * Copyright (c) 2015 Natale Patriciello <natale.patriciello@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define NS_LOG_APPEND_CONTEXT                                                                      \
    {                                                                                              \
        std::clog << Simulator::Now().GetSeconds() << " ";                                         \
    }

#include "tcp-pcc-aurora-cong-control.h"
#include "pcc_custom_rc.h"

#include "ns3/log.h"

#include <cmath>

NS_LOG_COMPONENT_DEFINE("TcpPccAurora");

namespace {
// Default TCPMSS used in UDT only.
const uint32_t kDefaultTCPMSS = 1400;
// Number of bits per byte.
const uint32_t kBitsPerByte = 8;
// Duration of monitor intervals as a proportion of RTT.
// const double kMonitorIntervalDuration = 2.0;
const double kMonitorIntervalDuration = 0.5;
// Minimum number of packets in a monitor interval.
const uint32_t kMinimumPacketsPerInterval = 5;
}  // namespace

namespace ns3
{

RttEstimatorJK::RttEstimatorJK()
{
}

RttEstimatorJK::~RttEstimatorJK()
{
}

void RttEstimatorJK::AddNewSample(Time rtt)
{
    if (m_estRtt.IsZero())
    {
        m_estRtt = rtt;
        m_varRtt = Time(m_estRtt.GetDouble() * 0.5);
    }
    else
    {
        m_estRtt = Time(m_estRtt.GetDouble() * (1.0-m_alpha)
                   + rtt.GetDouble() * m_alpha);
        m_varRtt = Time(m_varRtt.GetDouble() * (1.0-m_beta)
                   + std::abs((m_estRtt - rtt).GetDouble()) * m_beta);
    }
    // std::cout << "rtt new sample | ewma | var: " << rtt.GetMilliSeconds() << " | " << m_estRtt.GetMilliSeconds() << " | " << m_varRtt.GetMilliSeconds() << std::endl;
}

// Constant Rate

NS_OBJECT_ENSURE_REGISTERED(TcpPccAurora);

TypeId
TcpPccAurora::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TcpPccAurora")
                            .SetParent<TcpCongestionOpsCustom>()
                            .SetGroupName("Internet")
                            .AddConstructor<TcpPccAurora>();
    return tid;
}

TcpPccAurora::TcpPccAurora()
    : TcpCongestionOpsCustom()
{
    NS_LOG_FUNCTION(this);
    // std::cout << "Starting sending rate = " << m_sending_rate << std::endl;
    double call_freq = 1.0 / kMonitorIntervalDuration;
    m_rate_controller = new PccCustomRateController(call_freq);
}

TcpPccAurora::TcpPccAurora(const TcpPccAurora& sock)
    : TcpCongestionOpsCustom(sock)
{
    NS_LOG_FUNCTION(this);
    // std::cout << "Starting sending rate = " << m_sending_rate << std::endl;
    double call_freq = 1.0 / kMonitorIntervalDuration;
    m_rate_controller = new PccCustomRateController(call_freq);
}

TcpPccAurora::~TcpPccAurora()
{
}

void
TcpPccAurora::CongestionStateSet(Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCongState_t newState)
{
    NS_LOG_FUNCTION(this << tcb << newState);
    if (newState == TcpSocketState::CA_OPEN)
    {
        NS_LOG_DEBUG("CongestionStateSet triggered to CA_OPEN :: " << newState);
        m_wait_mode = false;
        // tcb->m_ssThresh = tcb->m_initialSsThresh;
        UpdatePacingRate(tcb);
        m_rtt_estimator.AddNewSample(tcb->m_lastRtt);
    }
    else if (newState == TcpSocketState::CA_LOSS)
    {
        NS_LOG_DEBUG("CongestionStateSet triggered to CA_LOSS :: " << newState);
        m_wait_mode = true;
    }
    else if (newState == TcpSocketState::CA_RECOVERY)
    {
        NS_LOG_DEBUG("CongestionStateSet triggered to CA_RECOVERY :: " << newState);
        // tcb->m_cWnd = tcb->m_bytesInFlight.Get() + std::max(tcb->m_lastAckedSackedBytes, tcb->m_segmentSize);
    }
}

void
TcpPccAurora::UpdatePacingRate(Ptr<TcpSocketState> tcb)
{
    NS_LOG_FUNCTION(this << tcb);

    if (!tcb->m_pacing)
    {
        NS_LOG_WARN("TcpPccAurora must use pacing");
        tcb->m_pacing = true;
    }

    Time rtt;
    if (tcb->m_minRtt != Time::Max())
    {
        rtt = MilliSeconds(std::max<long int>(tcb->m_minRtt.GetMilliSeconds(), 1));
    }
    else
    {
        rtt = MilliSeconds(1);
    }
    // DataRate nominalBandwidth(tcb->m_cWnd * 8 / rtt.GetSeconds());
    // tcb->m_pacingRate = DataRate(m_pacingGain * nominalBandwidth.GetBitRate());
    NS_LOG_INFO("Sending Rate " << m_sending_rate);
    tcb->m_pacingRate = m_sending_rate * 1.1;
    tcb->m_maxPacingRate = m_sending_rate * 1.1;
    tcb->m_pacingSsRatio = 1.1;
    uint32_t est_cWnd = m_sending_rate.GetBitRate()  * rtt.GetSeconds() / 8 * 1.1;
    tcb->m_cWnd = est_cWnd;
    tcb->m_ssThresh = est_cWnd;
}

std::string
TcpPccAurora::GetName() const
{
    return "TcpPccAurora";
}

uint32_t
TcpPccAurora::GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight)
{
    NS_LOG_FUNCTION(this << tcb << bytesInFlight);
    return tcb->m_ssThresh;
}

bool
TcpPccAurora::HasCongControl() const
{
    return true;
}

void
TcpPccAurora::CongControl(Ptr<TcpSocketState> tcb,
                              const TcpRateOps::TcpRateConnection& rc,
                              const TcpRateOps::TcpRateSample& rs)
{
    NS_LOG_FUNCTION(this << tcb);
    if (m_wait_mode) return;

    auto event_time = Simulator::Now();
    SequenceNumber32 segmentsAcked = tcb -> m_lastAckedSeq;
    uint32_t sz = tcb-> m_lastAckedSackedBytes;
    Time rtt = tcb->m_lastRtt;
    m_rtt_estimator.AddNewSample(rtt);
    Time rtt_estimate = GetCurrentRttEstimate(event_time);
    m_interval_queue.OnPacketAcked(event_time, segmentsAcked, sz, rtt_estimate);
    CheckMonitoringInterval(event_time);
    
    // Debugging
    // static bool done_print = false;
    // if (!done_print && event_time.Compare(mi.GetEndTime()) >= 0)
    // {
    //     done_print = true;
    //     std::cout << "OnPacketSent " << mi.GetBytesSent();
    //     std::cout << " OnPacketacked " << mi.GetBytesAcked();
    //     std::cout << " OnPacketLost " << mi.GetBytesLost();
    //     std::cout << " Utility " << CalculateUtility(mi) << "\n";
    //     m_sending_rate = DataRate(m_sending_rate.GetBitRate() * 0.5);
    // }
}

void
TcpPccAurora::ProcessECN()
{
    std::cout << "Here is ProcessECN\n";
}

void
TcpPccAurora::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked, const Time &rtt)
{
}

void
TcpPccAurora::OnPacketSent(Ptr<TcpSocketState> tcb, SequenceNumber32 seq, uint32_t sz)
{
    if (m_wait_mode) return;
    auto sent_time = Simulator::Now();
    if (ShouldCreateNewMonitorInterval(sent_time))
    {
        // Set the monitor duration to N of smoothed rtt.
        Time rtt_estimate = GetCurrentRttEstimate(sent_time);
        m_sending_rate = UpdateSendingRate(sent_time);
        Time monitor_duration = ComputeMonitorDuration(m_sending_rate, rtt_estimate);
        // std::cout << "monitor duration: " << monitor_duration.GetMilliSeconds() << std::endl;
        //std::cerr << "Create MI:" << std::endl;
        //std::cerr << "\tTime: " << sent_time << std::endl;
        //std::cerr << "\tPacket Number: " << packet_number << std::endl;
        //std::cerr << "\tDuration: " << monitor_duration << std::endl;
        m_interval_queue.Push(MonitorInterval(m_sending_rate, sent_time + monitor_duration));
        UpdatePacingRate(tcb);
    }
    m_interval_queue.OnPacketSent(sent_time, seq, sz);
}

void
TcpPccAurora::OnPacketLost(SequenceNumber32 seq, uint32_t sz)
{
    auto event_time = Simulator::Now();
    m_interval_queue.OnPacketLost(event_time, seq, sz);
    if (m_wait_mode) return;
    CheckMonitoringInterval(event_time);
}

void
TcpPccAurora::CheckMonitoringInterval(Time event_time)
{
    while (m_interval_queue.HasFinishedInterval(event_time))
    {
        MonitorInterval mi = m_interval_queue.Pop();
        //std::cerr << "MI Finished with: " << mi.n_packets_sent << ", loss " << mi.GetObsLossRate() << std::endl;
        mi.SetUtility(CalculateUtility(mi));
        m_rate_control_lock.lock();
        m_rate_controller->MonitorIntervalFinished(mi);
        m_rate_control_lock.unlock();
    }
}

bool TcpPccAurora::ShouldCreateNewMonitorInterval(Time sent_time)
{
    return m_interval_queue.Empty() ||
        m_interval_queue.Current().AllPacketsSent(sent_time);
}

Time TcpPccAurora::GetCurrentRttEstimate(Time sent_time)
{
    return m_rtt_estimator.GetJkRtt();
}

DataRate TcpPccAurora::UpdateSendingRate(Time event_time)
{
    m_rate_control_lock.lock();
    m_sending_rate = m_rate_controller->GetNextSendingRate(m_sending_rate, event_time);
    m_rate_control_lock.unlock();
    //std::cout << "PCC: rate = " << m_sending_rate << std::endl;
    return m_sending_rate;
}

Time TcpPccAurora::ComputeMonitorDuration(
    DataRate sending_rate, 
    Time rtt) {
  
  return
      Time(std::max(kMonitorIntervalDuration * rtt.GetDouble(), 
               kMinimumPacketsPerInterval * kBitsPerByte * 
               kDefaultTCPMSS / sending_rate.GetBitRate() *
               Time("1s").GetDouble()));
}

Ptr<TcpCongestionOpsCustom>
TcpPccAurora::Fork()
{
    return CopyObject<TcpPccAurora>(this);
}

} // namespace ns3

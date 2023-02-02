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

#include "tcp-constant-rate-cong-control.h"

#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("TcpConstantRate");

namespace ns3
{

// Constant Rate

NS_OBJECT_ENSURE_REGISTERED(TcpConstantRate);

TypeId
TcpConstantRate::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TcpConstantRate")
                            .SetParent<TcpCongestionOpsCustom>()
                            .SetGroupName("Internet")
                            .AddConstructor<TcpConstantRate>();
    return tid;
}

TcpConstantRate::TcpConstantRate()
    : TcpCongestionOpsCustom(), mi(MonitorInterval(sending_rate, Time("5s")))
{
    NS_LOG_FUNCTION(this);
}

TcpConstantRate::TcpConstantRate(const TcpConstantRate& sock)
    : TcpCongestionOpsCustom(sock), mi(MonitorInterval(sending_rate, Time("5s")))
{
    NS_LOG_FUNCTION(this);
}

TcpConstantRate::~TcpConstantRate()
{
}

void
TcpConstantRate::CongestionStateSet(Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCongState_t newState)
{
    NS_LOG_FUNCTION(this << tcb << newState);
    if (newState == TcpSocketState::CA_OPEN)
    {
        NS_LOG_DEBUG("CongestionStateSet triggered to CA_OPEN :: " << newState);
        // tcb->m_ssThresh = tcb->m_initialSsThresh;
        UpdatePacingRate(tcb);
    }
    else if (newState == TcpSocketState::CA_LOSS)
    {
        NS_LOG_DEBUG("CongestionStateSet triggered to CA_LOSS :: " << newState);
    }
    else if (newState == TcpSocketState::CA_RECOVERY)
    {
        NS_LOG_DEBUG("CongestionStateSet triggered to CA_RECOVERY :: " << newState);
        // tcb->m_cWnd = tcb->m_bytesInFlight.Get() + std::max(tcb->m_lastAckedSackedBytes, tcb->m_segmentSize);
    }
}

void
TcpConstantRate::UpdatePacingRate(Ptr<TcpSocketState> tcb)
{
    NS_LOG_FUNCTION(this << tcb);

    if (!tcb->m_pacing)
    {
        NS_LOG_WARN("TcpConstantRate must use pacing");
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
    NS_LOG_INFO("Sending Rate " << sending_rate);
    tcb->m_pacingRate = sending_rate * 1.1;
    tcb->m_maxPacingRate = sending_rate * 1.1;
    tcb->m_pacingSsRatio = 1;
    uint32_t est_cWnd = sending_rate.GetBitRate()  * rtt.GetSeconds() / 8 * 1.1;
    tcb->m_cWnd = est_cWnd;
    tcb->m_ssThresh = est_cWnd;
}

std::string
TcpConstantRate::GetName() const
{
    return "TcpConstantRate";
}

uint32_t
TcpConstantRate::GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight)
{
    NS_LOG_FUNCTION(this << tcb << bytesInFlight);
    return tcb->m_ssThresh;
}

bool
TcpConstantRate::HasCongControl() const
{
    return true;
}

void
TcpConstantRate::CongControl(Ptr<TcpSocketState> tcb,
                              const TcpRateOps::TcpRateConnection& rc,
                              const TcpRateOps::TcpRateSample& rs)
{
    NS_LOG_FUNCTION(this << tcb);

    auto current_time = Simulator::Now();
    SequenceNumber32 segmentsAcked = tcb -> m_lastAckedSeq;
    uint32_t sz = tcb-> m_lastAckedSackedBytes;
    Time rtt = tcb->m_lastRtt;
    mi.OnPacketAcked(current_time, segmentsAcked, sz, rtt);
    
    // Debugging
    static bool done_print = false;
    if (!done_print && current_time.Compare(mi.GetEndTime()) >= 0)
    {
        done_print = true;
        std::cout << "OnPacketSent " << mi.GetBytesSent();
        std::cout << " OnPacketacked " << mi.GetBytesAcked();
        std::cout << " OnPacketLost " << mi.GetBytesLost();
        std::cout << " Utility " << CalculateUtility(mi) << "\n";
        sending_rate = DataRate(sending_rate.GetBitRate() * 0.5);
    }
}

void
TcpConstantRate::ProcessECN()
{
    std::cout << "Here is ProcessECN\n";
}

void
TcpConstantRate::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked, const Time &rtt)
{
}

void
TcpConstantRate::OnPacketSent(Ptr<TcpSocketState> tcb, SequenceNumber32 seq, uint32_t sz)
{
    auto current_time = Simulator::Now();
    // Update socket sending rate once
    static bool done_update = false;
    if (!done_update && current_time.Compare(mi.GetEndTime()) >= 0)
    {
        UpdatePacingRate(tcb);
        done_update = true;
    }
    mi.OnPacketSent(current_time, seq, sz);
}

void
TcpConstantRate::OnPacketLost(SequenceNumber32 seq, uint32_t sz)
{
    auto current_time = Simulator::Now();
    mi.OnPacketLost(current_time, seq, sz);
    // std::cout << "Here is PacketLoss " << seq << " "<< sz << "\n";
}

Ptr<TcpCongestionOpsCustom>
TcpConstantRate::Fork()
{
    return CopyObject<TcpConstantRate>(this);
}

} // namespace ns3

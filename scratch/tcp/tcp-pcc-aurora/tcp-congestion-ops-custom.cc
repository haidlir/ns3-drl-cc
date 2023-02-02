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
#include "tcp-congestion-ops-custom.h"

#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TcpCongestionOpsCustom");

NS_OBJECT_ENSURE_REGISTERED(TcpCongestionOpsCustom);

TypeId
TcpCongestionOpsCustom::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::TcpCongestionOpsCustom").SetParent<Object>().SetGroupName("Internet");
    return tid;
}

TcpCongestionOpsCustom::TcpCongestionOpsCustom()
    : Object()
{
}

TcpCongestionOpsCustom::TcpCongestionOpsCustom(const TcpCongestionOpsCustom& other)
    : Object(other)
{
}

TcpCongestionOpsCustom::~TcpCongestionOpsCustom()
{
}

void
TcpCongestionOpsCustom::IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
    NS_LOG_FUNCTION(this << tcb << segmentsAcked);
}

void
TcpCongestionOpsCustom::PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked, const Time& rtt)
{
    NS_LOG_FUNCTION(this << tcb << segmentsAcked << rtt);
}

void
TcpCongestionOpsCustom::CongestionStateSet(Ptr<TcpSocketState> tcb,
                                     const TcpSocketState::TcpCongState_t newState)
{
    NS_LOG_FUNCTION(this << tcb << newState);
}

void
TcpCongestionOpsCustom::CwndEvent(Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCAEvent_t event)
{
    NS_LOG_FUNCTION(this << tcb << event);
}

bool
TcpCongestionOpsCustom::HasCongControl() const
{
    return false;
}

void
TcpCongestionOpsCustom::CongControl(Ptr<TcpSocketState> tcb,
                              const TcpRateOps::TcpRateConnection& /* rc */,
                              const TcpRateOps::TcpRateSample& /* rs */)
{
    NS_LOG_FUNCTION(this << tcb);
}

void
TcpCongestionOpsCustom::ProcessECN()
{
    NS_LOG_FUNCTION(this);
}

void
TcpCongestionOpsCustom::OnPacketSent(Ptr<TcpSocketState> tcb, SequenceNumber32 seq, uint32_t sz)
{
    NS_LOG_FUNCTION(this);
}

void
TcpCongestionOpsCustom::OnPacketLost(SequenceNumber32 seq, uint32_t sz)
{
    NS_LOG_FUNCTION(this);
}

} // namespace ns3

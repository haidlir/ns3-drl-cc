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
// #ifndef TCPCONGESTIONOPS_CUSTOM_H
// #define TCPCONGESTIONOPS_CUSTOM_H

#include "ns3/tcp-rate-ops.h"
#include "ns3/tcp-socket-state.h"

namespace ns3
{

/**
 * \ingroup tcp
 * \defgroup congestionOps Congestion Control Algorithms.
 *
 * The various congestion control algorithms, also known as "TCP flavors".
 */

/**
 * \ingroup congestionOps
 *
 * \brief Congestion control abstract class
 *
 * The design is inspired by what Linux v4.0 does (but it has been
 * in place for years). The congestion control is split from the main
 * socket code, and it is a pluggable component. An interface has been defined;
 * variables are maintained in the TcpSocketState class, while subclasses of
 * TcpCongestionOpsCustom operate over an instance of that class.
 *
 * Only three methods have been implemented right now; however, Linux has many others,
 * which can be added later in ns-3.
 *
 * \see IncreaseWindow
 * \see PktsAcked
 */
class TcpCongestionOpsCustom : public Object
{
  public:

    static TypeId GetTypeId();

    TcpCongestionOpsCustom();

    TcpCongestionOpsCustom(const TcpCongestionOpsCustom& other);

    ~TcpCongestionOpsCustom() override;

    virtual std::string GetName() const = 0;

    virtual void Init(Ptr<TcpSocketState> tcb [[maybe_unused]])
    {
    }

    virtual uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight) = 0;

    virtual void IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);

    virtual void PktsAcked(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked, const Time& rtt);

    virtual void CongestionStateSet(Ptr<TcpSocketState> tcb,
                                    const TcpSocketState::TcpCongState_t newState);

    virtual void CwndEvent(Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCAEvent_t event);

    virtual bool HasCongControl() const;

    virtual void CongControl(Ptr<TcpSocketState> tcb,
                             const TcpRateOps::TcpRateConnection& rc,
                             const TcpRateOps::TcpRateSample& rs);
    
    virtual void ProcessECN();
    virtual void OnPacketSent(Ptr<TcpSocketState> tcb, SequenceNumber32 seq, uint32_t sz);
    virtual void OnPacketLost(SequenceNumber32 seq, uint32_t sz);

    virtual Ptr<TcpCongestionOpsCustom> Fork() = 0;
};

} // namespace ns3

// #endif // TCPCONGESTIONOPS_H

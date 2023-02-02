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

#include "tcp-congestion-ops-custom.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-value.h"
#include "ns3/windowed-filter.h"

#include "monitoring-interval.h"

namespace ns3
{

/**
 * \brief The Constant Rate implementation
 *
 * It configures the rate to be constant.
 *
 */
class TcpConstantRate : public TcpCongestionOpsCustom
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    TcpConstantRate();

    /**
     * \brief Copy constructor.
     * \param sock object to copy.
     */
    TcpConstantRate(const TcpConstantRate& sock);

    ~TcpConstantRate() override;

    std::string GetName() const override;

    void CongestionStateSet(Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCongState_t newState) override;
    // void IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) override;
    uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight) override;
    Ptr<TcpCongestionOpsCustom> Fork() override;
    bool HasCongControl() const override;
    void CongControl(Ptr<TcpSocketState> tcb,
                     const TcpRateOps::TcpRateConnection& rc,
                     const TcpRateOps::TcpRateSample& rs) override;
    void ProcessECN();

    void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked, const Time &rtt);
    void OnPacketSent(Ptr<TcpSocketState> tcb, SequenceNumber32 seq, uint32_t sz);
    void OnPacketLost(SequenceNumber32 seq, uint32_t sz);

  protected:

  private:
    DataRate sending_rate {"1024Kbps"}; // Default rate is 512 Kbps
    MonitorInterval mi;

  void UpdatePacingRate(Ptr<TcpSocketState> tcb);
};

} // namespace ns3

// #endif // TCPCONGESTIONOPS_CUSTOM_H

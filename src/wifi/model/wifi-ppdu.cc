/*
 * Copyright (c) 2019 Orange Labs
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
 * Author: Rediet <getachew.redieteab@orange.com>
 */

#include "wifi-ppdu.h"

#include "wifi-psdu.h"

#include "ns3/log.h"
#include "ns3/packet.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("WifiPpdu");

WifiPpdu::WifiPpdu(Ptr<const WifiPsdu> psdu,
                   const WifiTxVector& txVector,
                   uint16_t txCenterFreq,
                   uint64_t uid /* = UINT64_MAX */)
    : m_preamble(txVector.GetPreambleType()),
      m_modulation(txVector.IsValid() ? txVector.GetModulationClass() : WIFI_MOD_CLASS_UNKNOWN),
      m_txCenterFreq(txCenterFreq),
      m_uid(uid),
      m_truncatedTx(false),
      m_txPowerLevel(txVector.GetTxPowerLevel())
{
    NS_LOG_FUNCTION(this << *psdu << txVector << txCenterFreq << uid);
    m_psdus.insert(std::make_pair(SU_STA_ID, psdu));
}

WifiPpdu::WifiPpdu(const WifiConstPsduMap& psdus,
                   const WifiTxVector& txVector,
                   uint16_t txCenterFreq,
                   uint64_t uid)
    : m_preamble(txVector.GetPreambleType()),
      m_modulation(txVector.IsValid() ? txVector.GetMode(psdus.begin()->first).GetModulationClass()
                                      : WIFI_MOD_CLASS_UNKNOWN),
      m_txCenterFreq(txCenterFreq),
      m_uid(uid),
      m_truncatedTx(false),
      m_txPowerLevel(txVector.GetTxPowerLevel()),
      m_txAntennas(txVector.GetNTx())
{
    NS_LOG_FUNCTION(this << psdus << txVector << txCenterFreq << uid);
    m_psdus = psdus;
}

WifiPpdu::~WifiPpdu()
{
    for (auto& psdu : m_psdus)
    {
        psdu.second = nullptr;
    }
    m_psdus.clear();
}

WifiTxVector
WifiPpdu::GetTxVector() const
{
    WifiTxVector txVector = DoGetTxVector();
    txVector.SetTxPowerLevel(m_txPowerLevel);
    txVector.SetNTx(m_txAntennas);
    return txVector;
}

WifiTxVector
WifiPpdu::DoGetTxVector() const
{
    NS_FATAL_ERROR("This method should not be called for the base WifiPpdu class. Use the "
                   "overloaded version in the amendment-specific PPDU subclasses instead!");
    return WifiTxVector(); // should be overloaded
}

Ptr<const WifiPsdu>
WifiPpdu::GetPsdu() const
{
    return m_psdus.begin()->second;
}

bool
WifiPpdu::IsTruncatedTx() const
{
    return m_truncatedTx;
}

void
WifiPpdu::SetTruncatedTx()
{
    NS_LOG_FUNCTION(this);
    m_truncatedTx = true;
}

WifiModulationClass
WifiPpdu::GetModulation() const
{
    return m_modulation;
}

uint16_t
WifiPpdu::GetTransmissionChannelWidth() const
{
    return GetTxVector().GetChannelWidth();
}

bool
WifiPpdu::DoesOverlapChannel(uint16_t minFreq, uint16_t maxFreq) const
{
    NS_LOG_FUNCTION(this << m_txCenterFreq << minFreq << maxFreq);
    uint16_t txChannelWidth = GetTxVector().GetChannelWidth();
    uint16_t minTxFreq = m_txCenterFreq - txChannelWidth / 2;
    uint16_t maxTxFreq = m_txCenterFreq + txChannelWidth / 2;
    /**
     * The PPDU does not overlap the channel in two cases.
     *
     * First non-overlapping case:
     *
     *                                        ┌─────────┐
     *                                PPDU    │ Nominal │
     *                                        │  Band   │
     *                                        └─────────┘
     *                                   minTxFreq   maxTxFreq
     *
     *       minFreq                       maxFreq
     *         ┌──────────────────────────────┐
     *         │           Channel            │
     *         └──────────────────────────────┘
     *
     * Second non-overlapping case:
     *
     *         ┌─────────┐
     * PPDU    │ Nominal │
     *         │  Band   │
     *         └─────────┘
     *    minTxFreq   maxTxFreq
     *
     *                 minFreq                       maxFreq
     *                   ┌──────────────────────────────┐
     *                   │           Channel            │
     *                   └──────────────────────────────┘
     */
    if (minTxFreq >= maxFreq || maxTxFreq <= minFreq)
    {
        return false;
    }
    return true;
}

bool
WifiPpdu::CanBeReceived(uint16_t p20MinFreq, uint16_t p20MaxFreq) const
{
    NS_LOG_FUNCTION(this << p20MinFreq << p20MaxFreq);
    const bool overlap = DoesOverlapChannel(p20MinFreq, p20MaxFreq);
    if (!overlap)
    {
        NS_LOG_INFO("Received PPDU does not overlap with the primary20 channel");
        return false;
    }
    return true;
}

uint64_t
WifiPpdu::GetUid() const
{
    return m_uid;
}

WifiPreamble
WifiPpdu::GetPreamble() const
{
    return m_preamble;
}

WifiPpduType
WifiPpdu::GetType() const
{
    return WIFI_PPDU_TYPE_SU;
}

uint16_t
WifiPpdu::GetStaId() const
{
    return SU_STA_ID;
}

Time
WifiPpdu::GetTxDuration() const
{
    NS_FATAL_ERROR("This method should not be called for the base WifiPpdu class. Use the "
                   "overloaded version in the amendment-specific PPDU subclasses instead!");
    return MicroSeconds(0); // should be overloaded
}

void
WifiPpdu::Print(std::ostream& os) const
{
    os << "[ preamble=" << m_preamble << ", modulation=" << m_modulation
       << ", truncatedTx=" << (m_truncatedTx ? "Y" : "N") << ", UID=" << m_uid << ", "
       << PrintPayload() << "]";
}

std::string
WifiPpdu::PrintPayload() const
{
    std::ostringstream ss;
    ss << "PSDU=" << GetPsdu() << " ";
    return ss.str();
}

Ptr<WifiPpdu>
WifiPpdu::Copy() const
{
    NS_FATAL_ERROR("This method should not be called for the base WifiPpdu class. Use the "
                   "overloaded version in the amendment-specific PPDU subclasses instead!");
    return Create<WifiPpdu>(GetPsdu(), GetTxVector(), m_txCenterFreq);
}

std::ostream&
operator<<(std::ostream& os, const Ptr<const WifiPpdu>& ppdu)
{
    ppdu->Print(os);
    return os;
}

std::ostream&
operator<<(std::ostream& os, const WifiConstPsduMap& psdus)
{
    for (const auto& psdu : psdus)
    {
        os << "PSDU for STA_ID=" << psdu.first << " (" << *psdu.second << ") ";
    }
    return os;
}

} // namespace ns3

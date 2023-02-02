/*
 * Copyright (c) 2010 CTTC
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
 * Authors: Nicola Baldo <nbaldo@cttc.es>
 *          Ghada Badawy <gbadawy@gmail.com>
 */

#include "wifi-tx-vector.h"

#include "wifi-phy-common.h"

#include "ns3/abort.h"

#include <algorithm>
#include <iterator>

namespace ns3
{

WifiTxVector::WifiTxVector()
    : m_preamble(WIFI_PREAMBLE_LONG),
      m_channelWidth(20),
      m_guardInterval(800),
      m_nTx(1),
      m_nss(1),
      m_ness(0),
      m_aggregation(false),
      m_stbc(false),
      m_ldpc(false),
      m_bssColor(0),
      m_length(0),
      m_modeInitialized(false),
      m_inactiveSubchannels(),
      m_ruAllocation()
{
}

WifiTxVector::WifiTxVector(WifiMode mode,
                           uint8_t powerLevel,
                           WifiPreamble preamble,
                           uint16_t guardInterval,
                           uint8_t nTx,
                           uint8_t nss,
                           uint8_t ness,
                           uint16_t channelWidth,
                           bool aggregation,
                           bool stbc,
                           bool ldpc,
                           uint8_t bssColor,
                           uint16_t length)
    : m_mode(mode),
      m_txPowerLevel(powerLevel),
      m_preamble(preamble),
      m_channelWidth(channelWidth),
      m_guardInterval(guardInterval),
      m_nTx(nTx),
      m_nss(nss),
      m_ness(ness),
      m_aggregation(aggregation),
      m_stbc(stbc),
      m_ldpc(ldpc),
      m_bssColor(bssColor),
      m_length(length),
      m_modeInitialized(true),
      m_inactiveSubchannels(),
      m_ruAllocation()
{
}

WifiTxVector::WifiTxVector(const WifiTxVector& txVector)
    : m_mode(txVector.m_mode),
      m_txPowerLevel(txVector.m_txPowerLevel),
      m_preamble(txVector.m_preamble),
      m_channelWidth(txVector.m_channelWidth),
      m_guardInterval(txVector.m_guardInterval),
      m_nTx(txVector.m_nTx),
      m_nss(txVector.m_nss),
      m_ness(txVector.m_ness),
      m_aggregation(txVector.m_aggregation),
      m_stbc(txVector.m_stbc),
      m_ldpc(txVector.m_ldpc),
      m_bssColor(txVector.m_bssColor),
      m_length(txVector.m_length),
      m_modeInitialized(txVector.m_modeInitialized),
      m_inactiveSubchannels(txVector.m_inactiveSubchannels),
      m_sigBMcs(txVector.m_sigBMcs),
      m_ruAllocation(txVector.m_ruAllocation)
{
    m_muUserInfos.clear();
    if (!txVector.m_muUserInfos.empty()) // avoids crashing for loop
    {
        for (auto& info : txVector.m_muUserInfos)
        {
            m_muUserInfos.insert(std::make_pair(info.first, info.second));
        }
    }
}

WifiTxVector::~WifiTxVector()
{
    m_muUserInfos.clear();
}

bool
WifiTxVector::GetModeInitialized() const
{
    return m_modeInitialized;
}

WifiMode
WifiTxVector::GetMode(uint16_t staId) const
{
    if (!m_modeInitialized)
    {
        NS_FATAL_ERROR("WifiTxVector mode must be set before using");
    }
    if (IsMu())
    {
        NS_ABORT_MSG_IF(staId > 2048, "STA-ID should be correctly set for MU (" << staId << ")");
        NS_ASSERT(m_muUserInfos.find(staId) != m_muUserInfos.end());
        return m_muUserInfos.at(staId).mcs;
    }
    return m_mode;
}

WifiModulationClass
WifiTxVector::GetModulationClass() const
{
    NS_ABORT_MSG_IF(!m_modeInitialized, "WifiTxVector mode must be set before using");

    if (IsMu())
    {
        NS_ASSERT(!m_muUserInfos.empty());
        // all the modes belong to the same modulation class
        return m_muUserInfos.begin()->second.mcs.GetModulationClass();
    }
    return m_mode.GetModulationClass();
}

uint8_t
WifiTxVector::GetTxPowerLevel() const
{
    return m_txPowerLevel;
}

WifiPreamble
WifiTxVector::GetPreambleType() const
{
    return m_preamble;
}

uint16_t
WifiTxVector::GetChannelWidth() const
{
    return m_channelWidth;
}

uint16_t
WifiTxVector::GetGuardInterval() const
{
    return m_guardInterval;
}

uint8_t
WifiTxVector::GetNTx() const
{
    return m_nTx;
}

uint8_t
WifiTxVector::GetNss(uint16_t staId) const
{
    if (IsMu())
    {
        NS_ABORT_MSG_IF(staId > 2048, "STA-ID should be correctly set for MU (" << staId << ")");
        NS_ASSERT(m_muUserInfos.find(staId) != m_muUserInfos.end());
        return m_muUserInfos.at(staId).nss;
    }
    return m_nss;
}

uint8_t
WifiTxVector::GetNssMax() const
{
    uint8_t nss = 0;
    if (IsMu())
    {
        for (const auto& info : m_muUserInfos)
        {
            nss = (nss < info.second.nss) ? info.second.nss : nss;
        }
    }
    else
    {
        nss = m_nss;
    }
    return nss;
}

uint8_t
WifiTxVector::GetNess() const
{
    return m_ness;
}

bool
WifiTxVector::IsAggregation() const
{
    return m_aggregation;
}

bool
WifiTxVector::IsStbc() const
{
    return m_stbc;
}

bool
WifiTxVector::IsLdpc() const
{
    return m_ldpc;
}

void
WifiTxVector::SetMode(WifiMode mode)
{
    m_mode = mode;
    m_modeInitialized = true;
}

void
WifiTxVector::SetMode(WifiMode mode, uint16_t staId)
{
    NS_ABORT_MSG_IF(!IsMu(), "Not a MU transmission");
    NS_ABORT_MSG_IF(staId > 2048, "STA-ID should be correctly set for MU");
    m_muUserInfos[staId].mcs = mode;
    m_modeInitialized = true;
}

void
WifiTxVector::SetTxPowerLevel(uint8_t powerlevel)
{
    m_txPowerLevel = powerlevel;
}

void
WifiTxVector::SetPreambleType(WifiPreamble preamble)
{
    m_preamble = preamble;
}

void
WifiTxVector::SetChannelWidth(uint16_t channelWidth)
{
    m_channelWidth = channelWidth;
}

void
WifiTxVector::SetGuardInterval(uint16_t guardInterval)
{
    m_guardInterval = guardInterval;
}

void
WifiTxVector::SetNTx(uint8_t nTx)
{
    m_nTx = nTx;
}

void
WifiTxVector::SetNss(uint8_t nss)
{
    m_nss = nss;
}

void
WifiTxVector::SetNss(uint8_t nss, uint16_t staId)
{
    NS_ABORT_MSG_IF(!IsMu(), "Not a MU transmission");
    NS_ABORT_MSG_IF(staId > 2048, "STA-ID should be correctly set for MU");
    m_muUserInfos[staId].nss = nss;
}

void
WifiTxVector::SetNess(uint8_t ness)
{
    m_ness = ness;
}

void
WifiTxVector::SetAggregation(bool aggregation)
{
    m_aggregation = aggregation;
}

void
WifiTxVector::SetStbc(bool stbc)
{
    m_stbc = stbc;
}

void
WifiTxVector::SetLdpc(bool ldpc)
{
    m_ldpc = ldpc;
}

void
WifiTxVector::SetBssColor(uint8_t color)
{
    m_bssColor = color;
}

uint8_t
WifiTxVector::GetBssColor() const
{
    return m_bssColor;
}

void
WifiTxVector::SetLength(uint16_t length)
{
    m_length = length;
}

uint16_t
WifiTxVector::GetLength() const
{
    return m_length;
}

void
WifiTxVector::SetSigBMode(const WifiMode& mode)
{
    m_sigBMcs = mode;
}

WifiMode
WifiTxVector::GetSigBMode() const
{
    return m_sigBMcs;
}

void
WifiTxVector::SetRuAllocation(const RuAllocation& ruAlloc)
{
    if (IsDlMu() && !m_muUserInfos.empty())
    {
        NS_ASSERT(ruAlloc == DeriveRuAllocation());
    }
    m_ruAllocation = ruAlloc;
}

const RuAllocation&
WifiTxVector::GetRuAllocation() const
{
    if (IsDlMu() && m_ruAllocation.empty())
    {
        m_ruAllocation = DeriveRuAllocation();
    }
    return m_ruAllocation;
}

bool
WifiTxVector::IsValid() const
{
    if (!GetModeInitialized())
    {
        return false;
    }
    std::string modeName = m_mode.GetUniqueName();
    if (m_channelWidth == 20)
    {
        if (m_nss != 3 && m_nss != 6)
        {
            return (modeName != "VhtMcs9");
        }
    }
    else if (m_channelWidth == 80)
    {
        if (m_nss == 3 || m_nss == 7)
        {
            return (modeName != "VhtMcs6");
        }
        else if (m_nss == 6)
        {
            return (modeName != "VhtMcs9");
        }
    }
    else if (m_channelWidth == 160)
    {
        if (m_nss == 3)
        {
            return (modeName != "VhtMcs9");
        }
    }
    return true;
}

bool
WifiTxVector::IsMu() const
{
    return ns3::IsMu(m_preamble);
}

bool
WifiTxVector::IsDlMu() const
{
    return ns3::IsDlMu(m_preamble);
}

bool
WifiTxVector::IsUlMu() const
{
    return ns3::IsUlMu(m_preamble);
}

HeRu::RuSpec
WifiTxVector::GetRu(uint16_t staId) const
{
    NS_ABORT_MSG_IF(!IsMu(), "RU only available for MU");
    NS_ABORT_MSG_IF(staId > 2048, "STA-ID should be correctly set for MU");
    return m_muUserInfos.at(staId).ru;
}

void
WifiTxVector::SetRu(HeRu::RuSpec ru, uint16_t staId)
{
    NS_ABORT_MSG_IF(!IsMu(), "RU only available for MU");
    NS_ABORT_MSG_IF(staId > 2048, "STA-ID should be correctly set for MU");
    m_muUserInfos[staId].ru = ru;
}

HeMuUserInfo
WifiTxVector::GetHeMuUserInfo(uint16_t staId) const
{
    NS_ABORT_MSG_IF(!IsMu(), "HE MU user info only available for MU");
    return m_muUserInfos.at(staId);
}

void
WifiTxVector::SetHeMuUserInfo(uint16_t staId, HeMuUserInfo userInfo)
{
    NS_ABORT_MSG_IF(!IsMu(), "HE MU user info only available for MU");
    NS_ABORT_MSG_IF(staId > 2048, "STA-ID should be correctly set for MU");
    NS_ABORT_MSG_IF(userInfo.mcs.GetModulationClass() < WIFI_MOD_CLASS_HE,
                    "Only HE (or newer) modes authorized for MU");
    m_muUserInfos[staId] = userInfo;
    m_modeInitialized = true;
    m_ruAllocation.clear();
}

const WifiTxVector::HeMuUserInfoMap&
WifiTxVector::GetHeMuUserInfoMap() const
{
    NS_ABORT_MSG_IF(!IsMu(), "HE MU user info map only available for MU");
    return m_muUserInfos;
}

WifiTxVector::HeMuUserInfoMap&
WifiTxVector::GetHeMuUserInfoMap()
{
    NS_ABORT_MSG_IF(!IsMu(), "HE MU user info map only available for MU");
    m_ruAllocation.clear();
    return m_muUserInfos;
}

std::pair<std::size_t, std::size_t>
WifiTxVector::GetNumRusPerHeSigBContentChannel() const
{
    // MU-MIMO is not handled for now, i.e. one station per RU
    auto ruAllocation = GetRuAllocation();
    NS_ASSERT_MSG(!ruAllocation.empty(), "RU allocation is not set");
    if (ruAllocation.size() != m_channelWidth / 20)
    {
        ruAllocation = DeriveRuAllocation();
    }
    NS_ASSERT_MSG(ruAllocation.size() == m_channelWidth / 20,
                  "RU allocation is not consistent with packet bandwidth");

    std::pair<std::size_t /* number of RUs in content channel 1 */,
              std::size_t /* number of RUs in content channel 2 */>
        chSize{0, 0};

    switch (GetChannelWidth())
    {
    case 40:
        chSize.second += HeRu::GetRuSpecs(ruAllocation[1]).size();
        [[fallthrough]];
    case 20:
        chSize.first += HeRu::GetRuSpecs(ruAllocation[0]).size();
        break;
    default:
        for (auto n = 0; n < m_channelWidth / 20;)
        {
            chSize.first += HeRu::GetRuSpecs(ruAllocation[n]).size();
            chSize.second += HeRu::GetRuSpecs(ruAllocation[n + 1]).size();
            if (ruAllocation[n] >= 208)
            {
                // 996 tone RU occupies 80 MHz
                n += 4;
                continue;
            }
            n += 2;
        }
        break;
    }
    return chSize;
}

void
WifiTxVector::SetInactiveSubchannels(const std::vector<bool>& inactiveSubchannels)
{
    NS_ABORT_MSG_IF(m_preamble < WIFI_PREAMBLE_HE_SU,
                    "Only HE (or later) authorized for preamble puncturing");
    NS_ABORT_MSG_IF(
        m_channelWidth < 80,
        "Preamble puncturing only possible for transmission bandwidth of 80 MHz or larger");
    NS_ABORT_MSG_IF(!inactiveSubchannels.empty() &&
                        inactiveSubchannels.size() != (m_channelWidth / 20),
                    "The size of the inactive subchannnels bitmap should be equal to the number of "
                    "20 MHz subchannels");
    m_inactiveSubchannels = inactiveSubchannels;
}

const std::vector<bool>&
WifiTxVector::GetInactiveSubchannels() const
{
    return m_inactiveSubchannels;
}

std::ostream&
operator<<(std::ostream& os, const WifiTxVector& v)
{
    if (!v.IsValid())
    {
        os << "TXVECTOR not valid";
        return os;
    }
    os << "txpwrlvl: " << +v.GetTxPowerLevel() << " preamble: " << v.GetPreambleType()
       << " channel width: " << v.GetChannelWidth() << " GI: " << v.GetGuardInterval()
       << " NTx: " << +v.GetNTx() << " Ness: " << +v.GetNess()
       << " MPDU aggregation: " << v.IsAggregation() << " STBC: " << v.IsStbc()
       << " FEC coding: " << (v.IsLdpc() ? "LDPC" : "BCC");
    if (v.GetPreambleType() >= WIFI_PREAMBLE_HE_SU)
    {
        os << " BSS color: " << +v.GetBssColor();
    }
    if (v.IsUlMu())
    {
        os << " Length: " << v.GetLength();
    }
    if (v.IsMu())
    {
        WifiTxVector::HeMuUserInfoMap userInfoMap = v.GetHeMuUserInfoMap();
        os << " num User Infos: " << userInfoMap.size();
        for (auto& ui : userInfoMap)
        {
            os << ", {STA-ID: " << ui.first << ", " << ui.second.ru << ", MCS: " << ui.second.mcs
               << ", Nss: " << +ui.second.nss << "}";
        }
    }
    else
    {
        os << " mode: " << v.GetMode() << " Nss: " << +v.GetNss();
    }
    const auto& puncturedSubchannels = v.GetInactiveSubchannels();
    if (!puncturedSubchannels.empty())
    {
        os << " Punctured subchannels: ";
        std::copy(puncturedSubchannels.cbegin(),
                  puncturedSubchannels.cend(),
                  std::ostream_iterator<bool>(os, ", "));
    }
    return os;
}

bool
HeMuUserInfo::operator==(const HeMuUserInfo& other) const
{
    return ru == other.ru && mcs.GetMcsValue() == other.mcs.GetMcsValue() && nss == other.nss;
}

bool
HeMuUserInfo::operator!=(const HeMuUserInfo& other) const
{
    return !(*this == other);
}

ContentChannelAllocation
WifiTxVector::GetContentChannelAllocation() const
{
    ContentChannelAllocation channelAlloc{{}};

    if (m_channelWidth > 20)
    {
        channelAlloc.emplace_back();
    }

    for (const auto& [staId, userInfo] : m_muUserInfos)
    {
        auto ruType = userInfo.ru.GetRuType();
        auto ruIdx = userInfo.ru.GetIndex();

        if ((ruType == HeRu::RU_484_TONE) || (ruType == HeRu::RU_996_TONE))
        {
            channelAlloc[0].push_back(staId);
            channelAlloc[1].push_back(staId);
            continue;
        }

        size_t numRus{1};
        if (ruType < HeRu::RU_242_TONE)
        {
            numRus = HeRu::m_heRuSubcarrierGroups.at({20, ruType}).size();
        }

        if (((ruIdx - 1) / numRus) % 2 == 0)
        {
            channelAlloc[0].push_back(staId);
        }
        else
        {
            channelAlloc[1].push_back(staId);
        }
    }

    return channelAlloc;
}

RuAllocation
WifiTxVector::DeriveRuAllocation() const
{
    std::all_of(m_muUserInfos.cbegin(), m_muUserInfos.cend(), [&](const auto& userInfo) {
        return userInfo.second.ru.GetRuType() == m_muUserInfos.cbegin()->second.ru.GetRuType();
    });
    RuAllocation ruAllocations(m_channelWidth / 20, HeRu::EMPTY_242_TONE_RU);
    std::vector<HeRu::RuType> ruTypes{};
    ruTypes.resize(ruAllocations.size());
    for (auto it = m_muUserInfos.begin(); it != m_muUserInfos.end(); ++it)
    {
        const auto ruType = it->second.ru.GetRuType();
        const auto ruBw = HeRu::GetBandwidth(ruType);
        const auto isPrimary80MHz = it->second.ru.GetPrimary80MHz();
        const auto rusPerSubchannel = HeRu::GetRusOfType(ruBw > 20 ? ruBw : 20, ruType);
        auto ruIndex = it->second.ru.GetIndex();
        if ((m_channelWidth >= 80) && (ruIndex > 19))
        {
            // take into account the center 26-tone RU in the primary 80 MHz
            ruIndex--;
        }
        if ((!isPrimary80MHz) && (ruIndex > 19))
        {
            // take into account the center 26-tone RU in the secondary 80 MHz
            ruIndex--;
        }
        if (!isPrimary80MHz && (ruType != HeRu::RU_2x996_TONE))
        {
            NS_ASSERT(m_channelWidth > 80);
            // adjust RU index for the secondary 80 MHz: in that case index is restarting at 1,
            // hence we need to add an offset corresponding to the number of RUs of the same type in
            // the primary 80 MHz
            ruIndex += HeRu::GetRusOfType(80, ruType).size();
        }
        const auto index =
            (ruBw < 20) ? ((ruIndex - 1) / rusPerSubchannel.size()) : ((ruIndex - 1) * (ruBw / 20));
        const auto numSubchannelsForRu = (ruBw < 20) ? 1 : (ruBw / 20);
        NS_ABORT_IF(index >= (m_channelWidth / 20));
        auto ruAlloc = HeRu::GetEqualizedRuAllocation(ruType, false);
        if (ruAllocations.at(index) != HeRu::EMPTY_242_TONE_RU)
        {
            if (ruType == ruTypes.at(index))
            {
                continue;
            }
            if (ruType == HeRu::RU_26_TONE)
            {
                ruAlloc = HeRu::GetEqualizedRuAllocation(ruTypes.at(index), true);
            }
            else if (ruTypes.at(index) == HeRu::RU_26_TONE)
            {
                ruAlloc = HeRu::GetEqualizedRuAllocation(ruType, true);
            }
            else
            {
                NS_ASSERT_MSG(false, "unsupported RU combination");
            }
        }
        for (auto i = 0; i < numSubchannelsForRu; ++i)
        {
            ruTypes.at(index + i) = ruType;
            ruAllocations.at(index + i) = ruAlloc;
        }
    }
    return ruAllocations;
}

} // namespace ns3

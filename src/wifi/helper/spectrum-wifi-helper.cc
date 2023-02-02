/*
 * Copyright (c) 2008 INRIA
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
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include "spectrum-wifi-helper.h"

#include "ns3/error-rate-model.h"
#include "ns3/frame-capture-model.h"
#include "ns3/interference-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-model.h"
#include "ns3/names.h"
#include "ns3/preamble-detection-model.h"
#include "ns3/spectrum-wifi-phy.h"
#include "ns3/wifi-net-device.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SpectrumWifiHelper");

SpectrumWifiPhyHelper::SpectrumWifiPhyHelper(uint8_t nLinks)
    : WifiPhyHelper(nLinks)
{
    NS_ABORT_IF(m_phy.size() != nLinks);
    for (auto& phy : m_phy)
    {
        phy.SetTypeId("ns3::SpectrumWifiPhy");
    }
    m_channels.resize(m_phy.size());
    SetInterferenceHelper("ns3::InterferenceHelper");
    SetErrorRateModel("ns3::TableBasedErrorRateModel");
}

void
SpectrumWifiPhyHelper::SetChannel(Ptr<SpectrumChannel> channel)
{
    for (auto& ch : m_channels)
    {
        ch = channel;
    }
}

void
SpectrumWifiPhyHelper::SetChannel(std::string channelName)
{
    Ptr<SpectrumChannel> channel = Names::Find<SpectrumChannel>(channelName);
    for (auto& ch : m_channels)
    {
        ch = channel;
    }
}

void
SpectrumWifiPhyHelper::SetChannel(uint8_t linkId, Ptr<SpectrumChannel> channel)
{
    m_channels.at(linkId) = channel;
}

void
SpectrumWifiPhyHelper::SetChannel(uint8_t linkId, std::string channelName)
{
    Ptr<SpectrumChannel> channel = Names::Find<SpectrumChannel>(channelName);
    m_channels.at(linkId) = channel;
}

std::vector<Ptr<WifiPhy>>
SpectrumWifiPhyHelper::Create(Ptr<Node> node, Ptr<WifiNetDevice> device) const
{
    std::vector<Ptr<WifiPhy>> ret;

    for (std::size_t i = 0; i < m_phy.size(); i++)
    {
        Ptr<SpectrumWifiPhy> phy = m_phy.at(i).Create<SpectrumWifiPhy>();
        phy->CreateWifiSpectrumPhyInterface(device);
        auto interference = m_interferenceHelper.Create<InterferenceHelper>();
        phy->SetInterferenceHelper(interference);
        Ptr<ErrorRateModel> error = m_errorRateModel.at(i).Create<ErrorRateModel>();
        phy->SetErrorRateModel(error);
        if (m_frameCaptureModel.at(i).IsTypeIdSet())
        {
            auto frameCapture = m_frameCaptureModel.at(i).Create<FrameCaptureModel>();
            phy->SetFrameCaptureModel(frameCapture);
        }
        if (m_preambleDetectionModel.at(i).IsTypeIdSet())
        {
            auto preambleDetection =
                m_preambleDetectionModel.at(i).Create<PreambleDetectionModel>();
            phy->SetPreambleDetectionModel(preambleDetection);
        }
        phy->SetChannel(m_channels.at(i));
        phy->SetDevice(device);
        phy->SetMobility(node->GetObject<MobilityModel>());
        ret.emplace_back(phy);
    }

    return ret;
}

} // namespace ns3

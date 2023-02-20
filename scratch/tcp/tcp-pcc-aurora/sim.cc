/*
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
 */

 /* Topology:
 *
 *   Right Leafs (Clients)                      Left Leafs (Sinks)
 *           |            \                    /        |
 *           |             \    bottleneck    /         |
 *           |              R0--------------R1          |
 *           |             /                  \         |
 *           |   access   /                    \ access |
 *           N -----------                      --------N
 */

// #include "ns3/config-store.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/opengym-module.h"

#include <fstream>
#include <string>
#include <iomanip>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpPccAuroraSim");

static void
CwndTracer(uint32_t oldval, uint32_t newval)
{
    std::cout << "cwnd " << std::fixed << std::setprecision(6) << Simulator::Now().GetSeconds()
               << std::setw(12) << oldval << " " << newval << std::endl;
}

static void
PacingRateTracer(DataRate oldval, DataRate newval)
{
    std::cout << "pacing rate " << std::fixed << std::setprecision(6) << Simulator::Now().GetSeconds()
                     << std::setw(12) << newval.GetBitRate() / 1e6 << std::endl;
}

static void
SsThreshTracer(uint32_t oldval, uint32_t newval)
{
    std::cout << "ss threshold " << std::fixed << std::setprecision(6) << Simulator::Now().GetSeconds()
                   << std::setw(12) << newval << std::endl;
}

void
ConnectSocketTraces()
{
    Config::ConnectWithoutContext("/NodeList/4/$ns3::TcpL4ProtocolCustom/SocketList/0/CongestionWindow",
                                  MakeCallback(&CwndTracer));
    Config::ConnectWithoutContext("/NodeList/4/$ns3::TcpL4ProtocolCustom/SocketList/0/PacingRate",
                                  MakeCallback(&PacingRateTracer));
    Config::ConnectWithoutContext("/NodeList/4/$ns3::TcpL4ProtocolCustom/SocketList/0/SlowStartThreshold",
                                  MakeCallback(&SsThreshTracer));
}

int
main(int argc, char* argv[])
{
    // OpenGym Env --- has to be created before any other thing
    uint32_t openGymPort = 5555;
    Ptr<OpenGymInterface> openGymInterface;
    openGymInterface = OpenGymInterface::Get(openGymPort);
    uint32_t run = 0;
    double tcpEnvTimeStep = 0.1;
    double duration = 10.0;
    bool tracing = false;
    uint32_t maxBytes = 0;
    uint32_t isTest = 0;

    //
    // Allow the user to override any of the defaults at
    // run-time, via command-line arguments
    //
    CommandLine cmd(__FILE__);
    cmd.AddValue("tracing", "Flag to enable/disable tracing", tracing);
    cmd.AddValue("maxBytes", "Total number of bytes for application to send", maxBytes);
    cmd.AddValue("openGymPort", "Port number for OpenGym env. Default: 5555", openGymPort);
    cmd.AddValue("simSeed", "Seed for random generator. Default: 1", run);
    cmd.AddValue("envTimeStep", "Time step interval for time-based TCP env [s]. Default: 0.1s", tcpEnvTimeStep);
    cmd.AddValue("duration", "Time to allow flows to run in seconds", duration);
    cmd.AddValue("test", "Print Flowstats", isTest);
    cmd.Parse(argc, argv);

    Time::SetResolution (Time::NS);
    LogComponentEnableAll (LOG_PREFIX_TIME);
    LogComponentEnableAll (LOG_PREFIX_FUNC);
    LogComponentEnableAll (LOG_PREFIX_NODE);
    // LogComponentEnable("BulkSendApplication", LOG_LEVEL_INFO);
    // LogComponentEnable("TcpPccAurora", LOG_LEVEL_INFO);
    // LogComponentEnable("TcpSocketBaseCustom", LOG_LEVEL_INFO);

    // Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno"));
    // ConfigStore config;
    // config.ConfigureDefaults ();

    //
    // Explicitly create the nodes required by the topology (shown above).
    //
    NS_LOG_INFO("Create nodes.");
    NodeContainer nodes;
    nodes.Create(2);

    NS_LOG_INFO("Create channels.");

    //
    // Explicitly create the point-to-point link required by the topology (shown above).
    //
    PointToPointHelper accessLink;
    accessLink.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    accessLink.SetChannelAttribute("Delay", StringValue("0.1ms"));
    // accessLink.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue("1p"));

    PointToPointHelper bottleneckLink;
    bottleneckLink.SetDeviceAttribute("DataRate", StringValue("12Mbps"));
    bottleneckLink.SetChannelAttribute("Delay", StringValue("30ms"));
    bottleneckLink.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", StringValue("128p"));

    // Arrange Dumbell
    PointToPointDumbbellHelper dumbell (1, accessLink,
                                    1, accessLink,
                                    bottleneckLink);
    // auto leftNodeId = dumbell.GetLeft(0)->GetId();
    // std::cout << "  Left Node Id: " << leftNodeId << "\n";

    //
    // Install the internet stack on the nodes
    //
    InternetStackHelper internet;
    // dumbell.InstallStack(internet);
    internet.Install(dumbell.GetRight());
    internet.Install(dumbell.GetLeft());
    internet.Install(dumbell.GetRight(0));

    InternetStackHelper internetCustomL4;
    internetCustomL4.SetTcp("ns3::TcpL4ProtocolCustom");
    internetCustomL4.Install(dumbell.GetLeft(0));

    //
    // We've got the "hardware" in place.  Now we need to add IP addresses.
    //
    NS_LOG_INFO("Assign IP Addresses.");
    dumbell.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                                 Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"),
                                 Ipv4AddressHelper ("10.3.1.0", "255.255.255.0"));
    NS_LOG_INFO ("Initialize Global Routing.");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    NS_LOG_INFO("Create Applications.");
    //
    // Create a BulkSendApplication and install it on node left
    //
    uint16_t port = 1337; // well-known echo port number

    BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(dumbell.GetRightIpv4Address(0), port));
    Config::Set("/NodeList/4/$ns3::TcpL4ProtocolCustom/SocketType", StringValue("ns3::TcpPccAurora"));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(2500000));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(5000000));
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(2));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448));
    // Set the amount of data to send in bytes.  Zero is unlimited.
    source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
    ApplicationContainer sourceApps = source.Install(dumbell.GetLeft(0));
    sourceApps.Start(Seconds(0.0));
    sourceApps.Stop(Seconds(55.0));

    //
    // Create a PacketSinkApplication and install it on node right
    //
    PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps = sink.Install(dumbell.GetRight(0));
    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(55.0));

    //
    // Set up tracing if enabled
    //
    if (tracing)
    {
        AsciiTraceHelper ascii;
        bottleneckLink.EnableAsciiAll(ascii.CreateFileStream("tcp-bulk-send.tr"));
        bottleneckLink.EnablePcapAll("tcp-bulk-send", false);
    }

    // Simulator::Schedule(MicroSeconds(1001), &ConnectSocketTraces);

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();    

    // config.ConfigureAttributes ();
    //
    // Now, do the actual simulation.
    //
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(Seconds(60.0));
    Simulator::Run();

    if (isTest)
    {
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
        // std::cout << "here here\n";
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        if (t.sourceAddress != dumbell.GetLeftIpv4Address(0))
            {
            continue;
            }
        auto flow_duration_tx = i->second.timeLastTxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();
        auto flow_duration_rx = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstRxPacket.GetSeconds();
        std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        std::cout << "  Flow Duration: " << flow_duration_tx << "s | " << flow_duration_rx << "s\n";
        std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
        std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
        std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / flow_duration_tx / 1000 / 1000  << "Mbps\n";
        std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
        std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
        std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / flow_duration_rx / 1000 / 1000  << "Mbps\n";
        std::cout << "  Avg Delay:   " << i->second.delaySum.GetMilliSeconds() / i->second.rxPackets << "ms\n";
        std::cout << "  Loss:   " << 1.0 - (static_cast<float>(i->second.rxBytes) / i->second.txBytes) << "\n";
    }
    }

    openGymInterface->NotifySimulationEnd();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    // Ptr<PacketSink> sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
    // std::cout << "Total Bytes Received: " << sink1->GetTotalRx() << std::endl;
    // std::cout << "Estimated Throughput: " << sink1->GetTotalRx() * 8 / 10 / 1000 << std::endl;

    return 0;
}

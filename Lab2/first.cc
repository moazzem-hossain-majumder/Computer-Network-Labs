/*
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"

#include <map>
#include <string>
#include <iostream>

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//
using namespace ns3;
NS_LOG_COMPONENT_DEFINE("FirstScriptExample");
int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);
    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    NodeContainer nodes;
    nodes.Create(2);
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);
    InternetStackHelper stack;
    stack.Install(nodes);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));
    UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));

    // Packet sizes to test: 2424, 1132, 4242, 2311
    echoClient.SetAttribute("PacketSize", UintegerValue(2311));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    // 1. Setup FlowMonitor
    FlowMonitorHelper flowHelper;
    Ptr<FlowMonitor> monitor = flowHelper.InstallAll();

    // 2. Run Simulation
    Simulator::Stop(Seconds(20.0));
    Simulator::Run();

    // 3. Process Statistics
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =
        DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

    for (auto const& [id, stat] : stats)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(id);

        std::string proto = (t.protocol == 6) ? "TCP" : (t.protocol == 17) ? "UDP" : "Unknown";
        std::cout << "FlowID: " << id << " (" << proto << " " << t.sourceAddress << "/"
                  << t.sourcePort << " --> " << t.destinationAddress << "/" << t.destinationPort
                  << ")\n";
        std::cout << "  Tx Bytes: " << stat.txBytes << "\n";
        std::cout << "  Rx Bytes: " << stat.rxBytes << "\n";
        if (stat.rxPackets > 0)
        {
            std::cout << "  Mean Delay: " << stat.delaySum.GetSeconds() / stat.rxPackets
                      << " s\n";
            std::cout << "  Throughput: " << (stat.rxBytes * 8.0) / 18.0 << " bps\n";
        }
    }

    Simulator::Destroy();
    return 0;
}
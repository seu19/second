/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 1.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

int
main(int argc, char* argv[])
{
    bool verbose = true;
    uint32_t nCsma = 3;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);

    cmd.Parse(argc, argv);

    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    nCsma = nCsma == 0 ? 1 : nCsma;

    NodeContainer p2pNodes;
    p2pNodes.Create(2);

    NodeContainer csmaNodes;
    csmaNodes.Add(p2pNodes.Get(1));
    csmaNodes.Create(nCsma);

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("3ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("275Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(500)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);

    InternetStackHelper stack;
    stack.Install(p2pNodes.Get(0));
    stack.Install(csmaNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);

    UdpEchoServerHelper echoServer(8080);

    ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(2));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(2), 8080);
    echoClient.SetAttribute("MaxPackets", UintegerValue(2));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(2048));

    ApplicationContainer clientApps = echoClient.Install(p2pNodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    pointToPoint.EnablePcapAll("ICTXXX");
    csma.EnablePcap("csma", csmaDevices.Get(1), true);
    csma.EnablePcap("p2p",p2pDevices.Get(0),true);
    
    AnimationInterface anim("ICT313.xml");
    anim.SetConstantPosition(p2pNodes.Get(0),10.0,10.0);
    anim.SetConstantPosition(p2pNodes.Get(1),30.0,20.0);
    anim.SetConstantPosition(csmaNodes.Get(1),80.0,30.0);
    anim.SetConstantPosition(csmaNodes.Get(2),50.0,40.0);
    anim.SetConstantPosition(csmaNodes.Get(3),20.0,50.0);
    
    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("p2p.tr"));
    csma.EnableAsciiAll(ascii.CreateFileStream("csma.tr"));

    Simulator::Run();
    Simulator::Destroy();
    return 0;



/////////////////////////////////////////////////////

./ns3 run scratch/second.cc
./NetAnim
java -jar  1024.tr 


////////////////////////////////////////////////


set terminal pdf
set output "smartphones.pdf"

set title "Smartphone Attributes"
set ylabel "Price (USD)"
set xlabel "Battery Capacity (mAh)"
plot "SmartphoneData.txt" using 2:4 with lines title "Battery Capacity vs Price"

set ylabel "Price (USD)"
set xlabel "Screen Size (inches)"
replot "SmartphoneData.txt" using 3:4 with linespoints title "Screen Size vs Price"


set style data histogram
set style histogram cluster gap 1
set style fill solid border -1
set boxwidth 0.8
set xlabel "Smartphone Index"
set ylabel "Price (USD)"
set title "Price Distribution of Smartphones"
plot "SmartphoneData.txt" using 4:xtic(1) title "Price Histogram"

//////////

gnuplot smartphones.plt

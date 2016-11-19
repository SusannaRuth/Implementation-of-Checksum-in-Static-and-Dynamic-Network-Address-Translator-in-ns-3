/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012
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
 * Authors: Frank Helbert <frank@ime.usp.br>,
 *          Luiz Arthur Feitosa dos Santos <luizsan@ime.usp.br> and
 *          Rodrigo Campiolo <campiolo@ime.usp.br>
 */

#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpEchoExample");

// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |    ns-3 TCP    |    |    ns-3 TCP    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                100 Mbps, 1 ms
//
// This is a simple test of TCP connection. We create two nodes,
// one will be a TCP client echo and other the TCP server echo, 
// the nodes transmit data in the simple point-to-point channel. 
// In this scenario the node 0 (client) send one packet TCP to 
// the node 1 (server), then server receive this packet, return 
// this back to the node 0 (client) and finish TCP connection.
//
// ===========================================================================
//

int
main (int argc, char *argv[])
{
  
  //
  // The three lines below enable debugging mode. Comment these three lines for disable.
  //
  //LogComponentEnable ("TcpEchoExample", LOG_LEVEL_INFO);
  //LogComponentEnable ("TcpEchoClientApplication", LOG_LEVEL_ALL);
  //LogComponentEnable ("TcpEchoServerApplication", LOG_LEVEL_ALL);


  //LogComponentEnable ("Socket", LOG_LEVEL_ALL);
  //LogComponentEnable ("TcpSocketBase", LOG_LEVEL_ALL);
  LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
  GlobalValue::Bind("ChecksumEnabled",BooleanValue(true));
  //
  // Allow the user to override any of the defaults and the above Bind() at
  // run-time, via command-line arguments
  //
  bool useV6 = false;
  Address serverAddress;

  CommandLine cmd;
  cmd.AddValue ("useIpv6", "Use Ipv6", useV6);
  cmd.Parse (argc, argv);
  
  //
  // Create two nodes required by the topology (point-to-point).
  //
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  nodes.Create (2);

  //
  // Create and configure channel for the communication.
  //
  NS_LOG_INFO("Create and configuring channels.");
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);
  
  //
  // Now, add IP address in the nodes.
  //

  NS_LOG_INFO("Assign IP Address.");
  if (useV6 == false)
    {
      Ipv4AddressHelper ipv4;
      ipv4.SetBase ("10.1.1.0", "255.255.255.0");
      Ipv4InterfaceContainer i4 = ipv4.Assign (devices);
      // for IPv4 GetAddress (1) is equivalent to GetAddress (1,0)
      serverAddress = Address(i4.GetAddress (1));
    }
  else
    {
      Ipv6AddressHelper ipv6;
      //ipv6.NewNetwork ("2001:0000:f00d:cafe::", 64);
      ipv6.NewNetwork ();
      Ipv6InterfaceContainer i6 = ipv6.Assign (devices);
      // Notice the difference between IPv4 and IPv6.
      // In both cases GetAddress (0,0) is the localhost (127.0.0.1 or ::1)
      // However in IPv6 GetAddress (1,0) is the Link-Local address
      // and GetAddress (1,1) is the Global address.
      serverAddress = Address(i6.GetAddress (1,1));
    }
  
  //
  // Create a TcpEchoServer on node 1.
  //
  NS_LOG_INFO("Create Server Application.");
  uint16_t port = 7; // well-known echo port number.
  TcpEchoServerHelper echoServer (port);
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (11.0));
  
  // Create a TcpEchoClient application to send TCP packet to server.
  NS_LOG_INFO("Create Client Application.");
  TcpEchoClientHelper echoClient (serverAddress, port);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (183));
  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

#if 0
//
// Users may find it convenient to initialize echo packets with actual data;
// the below lines suggest how to do this
//
  echoClient.SetFill (clientApps.Get (0), "Hello World");

  echoClient.SetFill (clientApps.Get (0), 0xa5, 1024);

  uint8_t fill[] = { 0, 1, 2, 3, 4, 5, 6};
  echoClient.SetFill (clientApps.Get (0), fill, sizeof(fill), 1024);
#endif

  //
  // Enable packet trace in pcap format and save on file tcp_echo_example.pcap.
  // Comment the two lines below to disable this.
  AsciiTraceHelper ascii;
  pointToPoint.EnablePcapAll("tcp_echo_example",true);

  // Start the simulation.
  NS_LOG_INFO("Start Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO("Simulation finished.");
  return 0;
}

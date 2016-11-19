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
// Desired topology:  n0 <----> n1 <-----> n2
// n0 and n1 in first container, n1 and n2 in second
//
// ===========================================================================
//

int
main (int argc, char *argv[])
{
  
  //
  // The lines below enable debugging mode. Comment these three lines for disable.
  //
  LogComponentEnable ("TcpEchoExample", LOG_LEVEL_INFO);
  LogComponentEnable ("TcpEchoClientApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("TcpEchoServerApplication", LOG_LEVEL_ALL);
  //LogComponentEnableAll(LOG_LEVEL_ALL);

  GlobalValue::Bind("ChecksumEnabled",BooleanValue(true));
  
  Address serverAddress;
  
  //
  // Create two nodes required by the topology (point-to-point).
  //
  NS_LOG_INFO ("Create nodes.");
  
  NodeContainer first;
  first.Create (2);

  NodeContainer second;
  second.Add ( first.Get (1) );
  second.Create (1);

  //
  // Create and configure channel for the communication.
  //
  NS_LOG_INFO("Create and configuring channels.");
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NetDeviceContainer devices1;
  devices1 = pointToPoint.Install (first);

  NetDeviceContainer devices2;
  devices2 = pointToPoint.Install (second);

  InternetStackHelper stack;

  stack.Install (first);

  stack.Install (second.Get (1));
  
  //
  // Now, add IP address in the nodes.
  //
  //        private address    NAT      public address
  // n0 <--------------------> n1 <-----------------------> n2
  // 192.168.1.1   192.168.1.2    203.82.48.1  203.82.48.2
  //

  NS_LOG_INFO("Assign IP Address.");

  Ipv4AddressHelper address1;
  address1.SetBase ("192.168.1.0", "255.255.255.0");

  Ipv4AddressHelper address2;
  address2.SetBase ("203.82.48.0", "255.255.255.0");

  Ipv4InterfaceContainer firstInterfaces = address1.Assign (devices1);
  Ipv4InterfaceContainer secondInterfaces = address2.Assign (devices2);
      
  serverAddress = Address(secondInterfaces.GetAddress (1));
      
  Ipv4NatHelper natHelper;
  // The zeroth element of the second node container is the NAT node
  Ptr<Ipv4Nat> nat = natHelper.Install (second.Get (0));

  // Configure which of its Ipv4Interfaces are inside and outside interfaces
  // The zeroth Ipv4Interface is reserved for the loopback interface
  // Hence, the interface facing n0 is numbered "1" and the interface
  // facing n2 is numbered "2" (since it was assigned in the second step above)
  nat->SetInside (1);
  nat->SetOutside (2);

  // Add a rule here to map outbound connections from n0, port 49153, UDP

  Ipv4StaticNatRule rule2 (Ipv4Address ("192.168.1.1"), 49153, Ipv4Address ("203.82.48.100"), 8080, 0);
  nat->AddStaticRule (rule2);

  // Now print them out
  Ptr<OutputStreamWrapper> natStream = Create<OutputStreamWrapper> ("static_nat.rules", std::ios::out);
  nat->PrintTable (natStream);

  //       Static Nat Table is as follows
  //Local IP     Local Port     Global IP           Global Port 
  //192.168.1.1     49153           203.82.48.100      8081            
  
  //
  // Create a TcpEchoServer.
  //
  NS_LOG_INFO("Create Server Application.");
  uint16_t port = 7; // well-known echo port number.
  TcpEchoServerHelper echoServer (port);
  ApplicationContainer serverApps = echoServer.Install (second.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (11.0));
  
  // Create a TcpEchoClient application to send TCP packet to server.
  NS_LOG_INFO("Create Client Application.");
  TcpEchoClientHelper echoClient (serverAddress, port);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (183));
  ApplicationContainer clientApps = echoClient.Install (first.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));


  // Prepare to run the simulation
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

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
  pointToPoint.EnablePcapAll("Static_Nat_TCP",true);

  // Start the simulation.
  NS_LOG_INFO("Start Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO("Simulation finished.");
  return 0;
}

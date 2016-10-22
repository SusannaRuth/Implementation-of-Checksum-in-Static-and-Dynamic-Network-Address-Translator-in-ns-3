/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Sindhuja Venkatesh
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
 * Authors: Sindhuja Venkatesh <intutivestriker88@gmail.com>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-netfilter-hook.h"
#include "ns3/ipv4-l3-protocol.h"
#include <fstream>
#include <string>

using namespace ns3;
using std::string;

NS_LOG_COMPONENT_DEFINE ("Static-Nat-Example");


// gives an string value for the drop reason
string dropReason(Ipv4L3Protocol::DropReason reason){
	string returnValue = "---";
	switch (reason)
	{
		case Ipv4L3Protocol::DROP_TTL_EXPIRED:
				returnValue= "DROP_TTL_EXPIRED";
				break;
		case Ipv4L3Protocol::DROP_NO_ROUTE:
				returnValue= "DROP_NO_ROUTE";
				break;
		case Ipv4L3Protocol::DROP_BAD_CHECKSUM:
				returnValue= "DROP_BAD_CHECKSUM";
				break;
		case Ipv4L3Protocol::DROP_INTERFACE_DOWN:
				returnValue= "DROP_INTERFACE_DOWN";
				break;
		case Ipv4L3Protocol::DROP_ROUTE_ERROR:
				returnValue= "DROP_ROUTE_ERROR";
				break;
		case Ipv4L3Protocol::DROP_FRAGMENT_TIMEOUT:
				returnValue= "DROP_FRAGMENT_TIMEOUT";
				break;
		case Ipv4L3Protocol::DROP_NF_DROP:
				returnValue= "DROP_NF_DROP : No match in Static Rule Table";
				break;
		default:
				NS_LOG_UNCOND("UNEXPECTED Drop value");
				break;
	}
	return returnValue;
}

// Trace call back function
void DropTrace (const Ipv4Header & header, Ptr<const Packet> packet,
		Ipv4L3Protocol::DropReason reason, Ptr<Ipv4> ipv4,  uint32_t interface)
{
    NS_LOG_UNCOND("-------- Drop Trace --------");
   
    std::cout << "Dropped packet!! from " << header.GetSource()<< " to "
		  << header.GetDestination()<<", Reason: " << dropReason(reason)
			  << std::endl;


}


int
main (int argc, char *argv[])
{
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_INFO);
  LogComponentEnable ("Ipv4Header", LOG_LEVEL_INFO);
  LogComponentEnable ("Ipv4Nat", LOG_LEVEL_INFO);

  GlobalValue::Bind("ChecksumEnabled",BooleanValue(true));

  // Desired topology:  n0 <----> n1 <-----> n2
  // n0 and n1 in first container, n1 and n2 in second

  NodeContainer first;
  first.Create (2);

  NodeContainer second;
  second.Add ( first.Get (1) );
  second.Create (1);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices1;
  devices1 = pointToPoint.Install (first);

  NetDeviceContainer devices2;
  devices2 = pointToPoint.Install (second);


  InternetStackHelper stack;

  stack.Install (first);

  stack.Install (second.Get (1));

  //        private address    NAT      public address
  // n0 <--------------------> n1 <-----------------------> n2
  // 192.168.1.1   192.168.1.2    203.82.48.1  203.82.48.2


  Ipv4AddressHelper address1;
  address1.SetBase ("192.168.1.0", "255.255.255.0");

  Ipv4AddressHelper address2;
  address2.SetBase ("203.82.48.0", "255.255.255.0");

  Ipv4InterfaceContainer firstInterfaces = address1.Assign (devices1);
  Ipv4InterfaceContainer secondInterfaces = address2.Assign (devices2);


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
           

  // Configure applications to generate traffic
  UdpEchoServerHelper echoServer (9);


  ApplicationContainer serverApps = echoServer.Install (second.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  // This application from client 192.168.1.1 with port 49153 is sending a packet of size 512B.
  // This application corresponds to the first rule hence the global ip address 203.82.48.100 replaces the local ip address 192.168.1.1
 // the global port 8080 replaces the local port 49153 as per the NAT Rule
  UdpEchoClientHelper echoClient1 (secondInterfaces.GetAddress (1), 9);
  echoClient1.SetAttribute ("MaxPackets", UintegerValue (2));
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (1.)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (512));

 
  ApplicationContainer clientApp1 = echoClient1.Install (first.Get (0));
  clientApp1.Start (Seconds (2.0));
  clientApp1.Stop (Seconds (5.0));

// This application from client 192.168.1.1 with port 49154 is sending a packet of size 512B.
 // This application doesn't have a corresponding port specific Static NAT Rule, hence the packet gets dropped by the NAT node
 
 ApplicationContainer clientApp2 = echoClient1.Install (first.Get (0));
  clientApp2.Start (Seconds (5.0));
  clientApp2.Stop (Seconds (10.0));


  // Prepare to run the simulation
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  pointToPoint.EnablePcapAll ("Static-Nat", true);

  first.Get (1)->GetObject<Ipv4> ()-> TraceConnectWithoutContext ("Drop", MakeCallback(&DropTrace));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

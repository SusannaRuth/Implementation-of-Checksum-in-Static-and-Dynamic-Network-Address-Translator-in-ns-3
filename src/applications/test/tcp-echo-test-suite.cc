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

#include <fstream>
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/tcp-echo-server.h"
#include "ns3/tcp-echo-client.h"
#include "ns3/tcp-echo-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/simple-channel.h"
#include "ns3/test.h"
#include "ns3/simulator.h"

using namespace ns3;

/**
 * Test if all tcp packets generated by a tcpEchoClient application are
 * correctly received by an tcpEchoServer application.
 * 
 */
class TcpEchoTestCase : public TestCase
{
public:
  TcpEchoTestCase ();
  virtual ~TcpEchoTestCase ();

private:
  virtual void DoRun (void);
};

TcpEchoTestCase::TcpEchoTestCase ()
  : TestCase ("Tcp-echo test if data sent by an client returns back to server")
{
}

TcpEchoTestCase::~TcpEchoTestCase ()
{
}

void
TcpEchoTestCase::DoRun (void)
{
  NodeContainer n;
  n.Create (2);

  InternetStackHelper internet;
  internet.Install (n);

  // link the two nodes
  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice> ();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice> ();
  n.Get (0)->AddDevice (txDev);
  n.Get (1)->AddDevice (rxDev);
  Ptr<SimpleChannel> channel1 = CreateObject<SimpleChannel> ();
  rxDev->SetChannel (channel1);
  txDev->SetChannel (channel1);
  NetDeviceContainer d;
  d.Add (txDev);
  d.Add (rxDev);

  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (d);

  uint16_t port = 4000;
  TcpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (n.Get (1));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  uint32_t MaxPacketSize = 183;
  Time interPacketInterval = Seconds (1.);
  uint32_t maxPacketCount = 5;
  TcpEchoClientHelper client (i.GetAddress (1), port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
  apps = client.Install (n.Get (0));
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_ASSERT_MSG_EQ (client.GetClient ()->GetBytesSent (), client.GetClient ()->GetBytesReceivedBack (), "Bytes received differ from bytes sent !");
}

/**
 * Test if all tcp packets generated by a tcpEchoClient application are
 * correctly received by an tcpEchoServer apllication - IPv6 version.
 *
 */
class TcpEchoTestCase6 : public TestCase
{
public:
  TcpEchoTestCase6 ();
  virtual ~TcpEchoTestCase6 ();

private:
  virtual void DoRun (void);
};

TcpEchoTestCase6::TcpEchoTestCase6 ()
  : TestCase ("Tcp-echo test if data sent by an client returns back to server - IPv6")
{
}

TcpEchoTestCase6::~TcpEchoTestCase6 ()
{
}

void
TcpEchoTestCase6::DoRun (void)
{
  NodeContainer n;
  n.Create (2);

  InternetStackHelper internet;
  internet.Install (n);

  // link the two nodes
  Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice> ();
  Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice> ();
  n.Get (0)->AddDevice (txDev);
  n.Get (1)->AddDevice (rxDev);
  Ptr<SimpleChannel> channel1 = CreateObject<SimpleChannel> ();
  rxDev->SetChannel (channel1);
  txDev->SetChannel (channel1);
  NetDeviceContainer d;
  d.Add (txDev);
  d.Add (rxDev);

  Ipv6AddressHelper ipv6;
  //ipv6.NewNetwork ("2001:0000:f00d:cafe::", 64);
  ipv6.NewNetwork ();
  Ipv6InterfaceContainer i6 = ipv6.Assign (d);

  uint16_t port = 4000;
  TcpEchoServerHelper server (port);
  ApplicationContainer apps = server.Install (n.Get (1));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (11.0));

  uint32_t MaxPacketSize = 183;
  Time interPacketInterval = Seconds (1.);
  uint32_t maxPacketCount = 5;
  TcpEchoClientHelper client (i6.GetAddress (1,1), port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
  apps = client.Install (n.Get (0));
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_ASSERT_MSG_EQ (client.GetClient ()->GetBytesSent (), client.GetClient ()->GetBytesReceivedBack (), "Bytes received differ from bytes sent !");
}

class TcpEchoTestSuite : public TestSuite
{
public:
  TcpEchoTestSuite ();
};

TcpEchoTestSuite::TcpEchoTestSuite ()
  : TestSuite ("tcp-echo", UNIT)
{
  AddTestCase (new TcpEchoTestCase,TestCase::QUICK);
  AddTestCase (new TcpEchoTestCase6,TestCase::QUICK);
}

static TcpEchoTestSuite tcpEchoTestSuite;

#include <ns3/ipv4.h>
#include <ns3/mobility-helper.h>
#include <ns3/udp-socket-factory.h>

#include "ns3-utils.hpp"

using namespace ns3;

void initializePositions(NodeContainer &node_container) {
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

  for (int i=0; i<node_container.GetN(); i++) {
    positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  }

  MobilityHelper mobility;
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(node_container);

  NS_LOG_INFO("Initialized random positions for nodes.");
}

void sendPacket(Ptr<Node> senderNode, Ptr<Node> receiverNode, Ptr<Packet> packet, uint32_t port) {
  Ptr<Socket> senderSocket = Socket::CreateSocket(senderNode, UdpSocketFactory::GetTypeId());
  Ipv4Address receiverAddress  = receiverNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
  InetSocketAddress remoteAddr = InetSocketAddress(receiverAddress, port);

  senderSocket->Connect(remoteAddr);
  senderSocket->Send(packet);
}

Ptr<Node> getNodeFromIP(Ipv4Address ip, NodeContainer &nodeContainer) {
  for (uint32_t i = 0; i < nodeContainer.GetN(); ++i) {
    Ptr<Node> node = nodeContainer.Get(i);
    if (node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal() == ip) {
      return node;
    }
  }

  return nullptr;
}


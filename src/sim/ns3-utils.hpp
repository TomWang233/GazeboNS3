#pragma once

#include <ns3/core-module.h>
#include <ns3/node-container.h>
#include <ns3/pointer.h>
#include <ns3/socket.h>

void initializePositions(ns3::NodeContainer &node_container);

void receivePacket(ns3::Ptr<ns3::Socket> socket);

void sendPacket(ns3::Ptr<ns3::Node> senderNode, ns3::Ptr<ns3::Node> receiverNode, ns3::Ptr<ns3::Packet> packet, uint32_t port);

ns3::Ptr<ns3::Node> getNodeFromIP(ns3::Ipv4Address ip, ns3::NodeContainer &nodeContainer);


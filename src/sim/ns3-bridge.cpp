#include <cassert>
#include <ns3/callback.h>
#include <thread>
#include <memory>

#include <ns3/wifi-helper.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/yans-wifi-helper.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/udp-socket-factory.h>
#include <ns3/mobility-model.h>
#include <ns3/double.h>
#include <ns3/uinteger.h>

#include "src/sim/ns3-bridge.hpp"
#include "src/sim/ns3-message.hpp"
#include "src/sim/ns3-utils.hpp"
#include "src/sim/thread-safe-queue.hpp"

using namespace ns3;

#define PORT 3030

NS_LOG_COMPONENT_DEFINE( "NS3BridgeImpl" );

struct Position {
  double x;
  double y;
  double altitude;
};

NetworkBridge::NetworkBridge() {}

NetworkBridge::~NetworkBridge() {}

class NS3Bridge : public NetworkBridge {
private:
  bool m_stopSimulation;
  bool m_flag = false; // Shared flag

  std::unique_ptr<std::thread> m_ns3Thread;

  NodeContainer m_nodeContainer;
  std::vector<ThreadSafeQueue<Message>*> m_mailboxRcv;
  std::vector<ThreadSafeQueue<Message>*> m_mailboxSnd;
  std::vector<Position> m_positions;

  void m_update();
  void runNS3Thread(uint32_t drone_count);

  void initNetwork(int numNodes) override;
  void stopSimulation() override {};

  void receivePacket(Ptr<Node> thisNode, Ptr<Socket> socket);
  void setListener(Ptr<Node> node);

public:
  NS3Bridge();
  ~NS3Bridge() override;

  NetworkSocket getSocket(int nodeId) override {
    assert(nodeId < m_nodeContainer.GetN());
    return std::make_pair(m_mailboxRcv[nodeId], m_mailboxSnd[nodeId]);
  }

  void updatePos(int nodeId, double latitude, double longitude, double altitude) override;
};

NS3Bridge::NS3Bridge() : m_stopSimulation(false) {}

void NS3Bridge::m_update() {
  // Run update loop to send any messages, etc.
  NS_LOG_INFO("Update...");

  // Forward all messages from the send boxes
  for (int nodeId=0; nodeId<m_nodeContainer.GetN(); nodeId++) {
    auto nodeMailbox = m_mailboxSnd[nodeId];

    while (!nodeMailbox->empty()) {
      auto packetOpt = nodeMailbox->tryPop();
      if (!packetOpt) {
        NS_LOG_INFO("WARNING: Invalid packet in sendbox");
        continue;
      }

      NS3Message message = NS3Message(packetOpt.value());
      int targetNodeId = message.getTargetNode();

      if (targetNodeId < 0) {
        targetNodeId = m_nodeContainer.GetN() + targetNodeId;
      }

      auto targetNode = m_nodeContainer.Get(targetNodeId);

      sendPacket(m_nodeContainer.Get(nodeId), targetNode, message.toPacket(), PORT);
      std::cout << "POP" << std::endl;
    }

    auto pos = m_positions[nodeId];
    auto mobility = m_nodeContainer.Get(nodeId)->GetObject<MobilityModel>();
    mobility->SetPosition(ns3::Vector3D(pos.x, pos.y, pos.altitude));
  }


  // Reschedule update function
  if (!m_stopSimulation) {
    Simulator::Schedule(MilliSeconds(50), MakeEvent(&NS3Bridge::m_update, this));
  }
}

void NS3Bridge::receivePacket(Ptr<Node> thisNode, Ptr<Socket> socket) {
  Ptr<Packet> packet;
  Address senderAddr;

  while ((packet = socket->RecvFrom(senderAddr))) {
    Message message = NS3Message::fromPacket(packet, thisNode->GetId());
    auto n = getNodeFromIP(InetSocketAddress::ConvertFrom(senderAddr).GetIpv4(), m_nodeContainer)->GetId();
    message.setSourceNode(getNodeFromIP(InetSocketAddress::ConvertFrom(senderAddr).GetIpv4(), m_nodeContainer)->GetId());
    m_mailboxRcv[thisNode->GetId()]->push(std::move(message));
  }
}

void NS3Bridge::setListener(Ptr<Node> node) {
  Ptr<Socket> receiverSocket =
      Socket::CreateSocket(node, UdpSocketFactory::GetTypeId());
  uint16_t receiverPort = PORT;
  InetSocketAddress localAddr = node->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
  localAddr.SetPort(receiverPort);
  if (receiverSocket->Bind(localAddr) == -1) {
    NS_LOG_ERROR("Could not bind listener on address");
    return;
  }
  receiverSocket->SetRecvCallback(MakeCallback(&NS3Bridge::receivePacket, this, node));

  NS_LOG_INFO("Listening...");
}


void NS3Bridge::runNS3Thread(uint32_t drone_count) {
  GlobalValue::Bind("SimulatorImplementationType",
                    StringValue("ns3::RealtimeSimulatorImpl"));

  std::string phyMode ("DsssRate1Mbps");
  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel",
                                      "Exponent", DoubleValue (2.0), // Typical value for drones in free space
                                      "ReferenceDistance", DoubleValue (10.0),
                                      "ReferenceLoss", DoubleValue (40.0));
  /*wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");*/
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetChannel(wifiChannel.Create());
  wifiPhy.Set("TxPowerStart", DoubleValue (125.0));             // Transmit power: 30 dBm
  wifiPhy.Set("TxPowerEnd", DoubleValue (125.0));
  wifiPhy.Set("RxSensitivity", DoubleValue (-85.0));
  wifiPhy.Set("TxGain", DoubleValue(4.0));
  wifiPhy.Set("RxGain", DoubleValue(4.0));
  wifiPhy.Set("ChannelSettings", StringValue("{0, 0, BAND_2_4GHZ, 0}"));

  WifiMacHelper wifiMac;
  wifiMac.SetType("ns3::AdhocWifiMac");

  m_nodeContainer.Create(drone_count);

  NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, m_nodeContainer);
  initializePositions(m_nodeContainer);

  // Install Internet stack
  InternetStackHelper internet;
  internet.Install(m_nodeContainer);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  wifiPhy.EnablePcapAll("trace", false);

  for (uint32_t i = 0; i < m_nodeContainer.GetN(); ++i) {
    Ptr<Node> node = m_nodeContainer.Get(i);
    setListener(node);
  }

  // Start the periodic check at the beginning of the simulation
  Simulator::Schedule(Seconds(0.0), MakeEvent(&NS3Bridge::m_update, this));
  m_flag = true;

  std::cout << "Starting simulation" << std::endl;

  Simulator::Run();
}

void NS3Bridge::initNetwork(int drone_count) {
  for (int i=0; i<drone_count; i++) {
    m_mailboxSnd.push_back(new ThreadSafeQueue<Message>());
    m_mailboxRcv.push_back(new ThreadSafeQueue<Message>());
    m_positions.push_back({.x = 0.0, .y = 0.0, .altitude = 0.0});
  }

  std::cout << "Initializing network with " << drone_count << " nodes" << std::endl;

  m_ns3Thread = std::make_unique<std::thread>(std::bind(&NS3Bridge::runNS3Thread, this, drone_count));

  while (!m_flag) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void NS3Bridge::updatePos(int nodeId, double x, double y, double altitude) {
  if (nodeId < 0) {
    nodeId = m_nodeContainer.GetN() + nodeId;
  }

  assert(nodeId < m_positions.size());

  m_positions[nodeId] = {
    .x = x,
    .y = y,
    .altitude = altitude
  };

  std::cout << "The position of drone " << nodeId << " is " << x << ", " << y << ", " << altitude << std::endl;
}

NS3Bridge::~NS3Bridge() {
  for (auto item : m_mailboxRcv)
    delete item;

  for (auto item : m_mailboxSnd)
    delete item;
}

NetworkBridge* createNS3Bridge() {
  return new NS3Bridge();
}

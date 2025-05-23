#pragma once

#include "src/sim/message.hpp"
#include <ns3/packet.h>
#include <ns3/core-module.h>

class NS3Header : public ns3::Header {
  uint32_t m_msgId;

public:
  NS3Header() : m_msgId(0) {}
  NS3Header(uint32_t msgId) : m_msgId(msgId) {}
  
  uint32_t getMsgId() {
    return m_msgId;
  }

  static ns3::TypeId GetTypeId() {
    static ns3::TypeId tid = ns3::TypeId("ns3::NS3Header").SetParent<Header>().AddConstructor<NS3Header>();
    return tid;
  }

  ns3::TypeId GetInstanceTypeId() const { return GetTypeId(); }

  void Print(std::ostream &os) const {
    os << "Message Id=" << m_msgId;
  }

  uint32_t GetSerializedSize() const { return sizeof(uint32_t); }

  void Serialize(ns3::Buffer::Iterator start) const {
    start.WriteHtonU32(m_msgId);
  }

  uint32_t Deserialize(ns3::Buffer::Iterator start) {
    m_msgId = start.ReadNtohU32();
    return GetSerializedSize();
  }
};

class NS3Message : public Message {
public:
  NS3Message(const Message& message) : Message(message) {}

  ns3::Ptr<ns3::Packet> toPacket() {
    NS3Header header = NS3Header(this->getId());

    auto content_ptr = this->as<uint8_t*>();

    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>(content_ptr, this->getLength());
    packet->AddHeader(header);

    return packet;
  }

  static Message fromPacket(ns3::Ptr<ns3::Packet> packet, uint32_t target) {
    NS3Header header;
    packet->RemoveHeader(header);

    uint8_t* data = new uint8_t[packet->GetSize()];
    packet->CopyData(data, packet->GetSize());

    const Message message = Message(target, (MessageId)header.getMsgId(), data, packet->GetSize());
    delete[] data;
    return message;
  }
};

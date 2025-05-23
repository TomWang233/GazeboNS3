#pragma once

#include <cstdint>
#include <cstring>

enum MessageId {
  HelloMessage,
  ActorPosMessage
};

class Message {
  int m_targetNode;
  uint32_t m_sourceNode;

  MessageId m_messageId;

  void* m_contents;
  uint32_t m_length;

public:
  Message(int targetNode, MessageId messageId, void* contents, const uint32_t length, int sourceNode = 0)
    : m_targetNode(targetNode), m_messageId(messageId), m_length(length), m_contents(nullptr), m_sourceNode(sourceNode) {
    this->m_contents = new uint8_t[length];
    memcpy(this->m_contents, contents, length);
  }

  Message(const Message& message) : Message(message.m_targetNode, message.m_messageId, message.m_contents, message.m_length, message.m_sourceNode) {}
  Message(Message&& message)
    : m_targetNode(message.m_targetNode), m_messageId(message.m_messageId), m_contents(message.m_contents), m_length(message.m_length), m_sourceNode(message.m_sourceNode) {
    message.m_contents = nullptr;
  }

  void operator=(const Message& message) {
    this->m_targetNode = message.m_targetNode;
    this->m_messageId = message.m_messageId;
    this->m_length = message.m_length;
    this->m_sourceNode = message.m_sourceNode;

    delete[] (uint8_t*)this->m_contents;
    this->m_contents = new uint8_t[this->m_length];
    memcpy(this->m_contents, message.m_contents, this->m_length);
  }

  ~Message() {
    delete[] (uint8_t*)this->m_contents;
  }

  template<typename T>
  T as() {
    return static_cast<T>(this->m_contents);
  }

  const uint32_t getLength() {
    return this->m_length;
  }

  const MessageId getId() {
    return this->m_messageId;
  }

  const int getTargetNode() {
    return this->m_targetNode;
  }

  const int getSourceNode() {
    return this->m_sourceNode;
  }

  void setSourceNode(uint32_t sourceNode) {
    this->m_sourceNode = sourceNode;
  }
};

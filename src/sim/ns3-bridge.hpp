#pragma once

#include "src/sim/thread-safe-queue.hpp"
#include "src/sim/message.hpp"

class NetworkSocket;

class NetworkBridge {
public:
  NetworkBridge();
  virtual ~NetworkBridge();

  virtual void updatePos(int nodeId, double x, double y, double altitude) = 0;
  virtual NetworkSocket getSocket(int nodeId) = 0;
  
  virtual void initNetwork(int numNodes) = 0;
  virtual void stopSimulation() = 0;
};

class NetworkSocket {
  ThreadSafeQueue<Message>* m_queueRcv;
  ThreadSafeQueue<Message>* m_queueSnd;

public:
  NetworkSocket(std::pair<ThreadSafeQueue<Message>*, ThreadSafeQueue<Message>*> queuePair)
    : m_queueRcv(queuePair.first), m_queueSnd(queuePair.second) {}

  NetworkSocket(const NetworkSocket&& other)
    : m_queueRcv(other.m_queueRcv), m_queueSnd(other.m_queueSnd) {} 

  std::optional<Message> receive() {
    if (!m_queueRcv->empty()) {
      return std::optional(m_queueRcv->waitPop());
    } else {
      return std::optional<Message>();
    }
  }

  void send(Message& message) {
    m_queueSnd->push(message);
  }
};

NetworkBridge* createNS3Bridge();

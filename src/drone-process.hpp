#pragma once

#include <ns3/ptr.h>
#include <ns3/packet.h>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

#include "src/sim/ns3-bridge.hpp"

void droneProcess(std::shared_ptr<mavsdk::System> system, uint32_t id, std::vector<uint32_t> peer_ids, NetworkSocket socket);


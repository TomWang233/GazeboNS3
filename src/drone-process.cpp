#include <cstdint>
#include <iostream>
#include <thread>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

#include "src/sim/ns3-bridge.hpp"

using namespace std::this_thread;
using namespace std::chrono;
using namespace mavsdk;

enum State {
    OnGroundUnarmed,
    OnGroundArmed,
    inAir,
    inAirSetup,
    land,
    Exit
};

State onGroundUnarmed(Action& action, Telemetry& telemetry);
State onGroundArmed(Action& action, Telemetry& telemetry);
State InAirSetup(Action& action, Telemetry& telemetry);
State InAir(Action& action, Telemetry& telemetry, NetworkSocket* socket);
State Land(Action& action, Telemetry& telemetry);

void droneProcess(std::shared_ptr<System> system, uint32_t id, std::vector<uint32_t> peer_ids, NetworkSocket socket) {
    auto telemetry = Telemetry{system};
    auto action = Action{system};
    auto vehicleState = State::OnGroundUnarmed;
    
    telemetry.set_rate_position(1.0);
    telemetry.set_rate_rc_status(1.0);
    telemetry.set_rate_in_air(1.0);

    // Mainloop
    bool mainloopRun = true;
    while (mainloopRun) {
        switch(vehicleState) {
            case OnGroundUnarmed:
                vehicleState = onGroundUnarmed(action, telemetry);
                break;
            case OnGroundArmed:
                vehicleState = onGroundArmed(action, telemetry);
                break;
            case inAirSetup:
                vehicleState = InAirSetup(action, telemetry);
                break;
            case inAir:
                vehicleState = InAir(action, telemetry, &socket);
                break;
            case land:
                vehicleState = Land(action, telemetry);
                break;
            case Exit:
                mainloopRun = false;
                break;
        };
        sleep_for(milliseconds(100));
    }
}

State onGroundUnarmed(Action& action, Telemetry& telemetry) {
    // How many health checks before we decide to arm anyways?
    const uint32_t MAX_HEALTH_CHECKS = 50;
    thread_local uint32_t healthCheckCount = 0;

    if (telemetry.armed()) {
        return State::OnGroundArmed;
    }

    if (!telemetry.health_all_ok() && healthCheckCount < MAX_HEALTH_CHECKS) {
        healthCheckCount++;
        return State::OnGroundUnarmed;
    }
    healthCheckCount = 0;

    // Ready to try arming!
    const Action::Result arm_result = action.arm();

    if (arm_result != Action::Result::Success) {
        std::cerr << "Arming failed:" << arm_result << '\n';
    }
    sleep_for(seconds(1));
    return State::OnGroundUnarmed;
}

State onGroundArmed(Action& action, Telemetry& telemetry) {
    if (!telemetry.armed()) {
        return State::OnGroundUnarmed;
    } else if (telemetry.in_air()) {
        return State::inAirSetup;
    }

    action.takeoff();
    return State::inAirSetup;
}


State InAirSetup(Action& action, Telemetry& telemetry) {
    if (!telemetry.in_air()) {
        return State::OnGroundArmed;
    }

    return State::inAir;
}

State InAir(Action& action, Telemetry& telemetry, NetworkSocket* socket) {
    if (!telemetry.in_air()) {
        return State::OnGroundArmed;
    }
    return State::inAir;
}

State Land(Action& action, Telemetry& telemetry) {
    if (!telemetry.in_air()) {
        return State::Exit;
    }
    return State::land;
}

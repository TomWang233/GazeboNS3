#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "src/sim/ns3-bridge.hpp"
#include "src/drone-process.hpp"
#include "src/sim/world_parser.hpp"

using namespace mavsdk;
using namespace std::this_thread;
using namespace std::chrono;

std::atomic<bool> running(true);

void latLngToMeters(double latitude, double longitude, double& x, double& y);
void metersToLatLng(double x, double y, double &latitude, double &longitude);

// Default 100ms
#define BRIDGE_MAINLOOP_DELAY_MS 1000

void updateProcess(std::vector<std::shared_ptr<System>>* processes, NetworkBridge* ns3Bridge) {
    std::vector<Telemetry*> telemetries;

    for (int i=0; i<processes->size(); i++) {
        auto proc = std::ref((*processes)[i]);
        telemetries.push_back(new Telemetry{proc});
        telemetries[i]->set_rate_position(1.0);
        telemetries[i]->set_rate_gps_info(1.0);
    }

    while (running) {
        for (int i=0; i<processes->size(); i++) {
            auto telemetry = telemetries[i];
            auto gt = telemetry->raw_gps();
            double x, y;

            latLngToMeters(gt.latitude_deg, gt.longitude_deg, x, y);
            ns3Bridge->updatePos(i, x, y, gt.absolute_altitude_m);
        }

        sleep_for(milliseconds(BRIDGE_MAINLOOP_DELAY_MS));
    }

    for (auto item : telemetries) {
        delete item;
    }
}

void usage(const std::string& bin_name) {
    // Reference taken from `multiple_drones` example in the MavSdk project.
    // Please see their Github for more details.
    std::cerr << "Usage : " << bin_name << " <connection_url_1> [<connection_url_2> ...]\n"
              << "Connection URL format should be :\n"
              << " For TCP : tcp://[server_host][:server_port]\n"
              << " For UDP : udp://[bind_host][:bind_port]\n"
              << " For Serial : serial:///path/to/serial/dev[:baudrate]\n"
              << "For example, to connect to the simulator use URL: udpin://0.0.0.0:14540\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Please specify connection\n";
        usage(argv[0]);
        return 1;
    }
    
    #ifdef __APPLE__
    Mavsdk mavsdk{Mavsdk::Configuration{Mavsdk::ComponentType::GroundStation}};
    #else
    Mavsdk mavsdk{Mavsdk::Configuration{ComponentType::GroundStation}};
    #endif

    size_t total_udp_ports = argc - 1;

    // the loop below adds the number of ports the sdk monitors.
    for (int i = 1; i < argc; ++i) {
        ConnectionResult connection_result = mavsdk.add_any_connection(argv[i]);
        if (connection_result != ConnectionResult::Success) {
            std::cerr << "Connection error: " << connection_result << '\n';
            return 1;
        }
    }

    std::atomic<size_t> num_systems_discovered{0};

    std::cout << "Waiting to discover system...\n";
    mavsdk.subscribe_on_new_system([&mavsdk, &num_systems_discovered]() {
        const auto systems = mavsdk.systems();

        if (systems.size() > num_systems_discovered) {
            std::cout << "Discovered system\n";
            num_systems_discovered = systems.size();
        }
    });

    // We usually receive heartbeats at 1Hz, therefore we should find a system after around 2
    // seconds.
    sleep_for(seconds(2));

    if (num_systems_discovered != total_udp_ports) {
        std::cerr << "Not all systems found, exiting.\n";
        return 1;
    }

    std::vector<uint32_t> peer_ids;
    for (uint32_t i = 0; i < num_systems_discovered; i++) {
        peer_ids.push_back(i);
    }

    std::cout << "Initing network...\n" << std::endl;

    // Start NS-3 Thread
    NetworkBridge* ns3Bridge = createNS3Bridge();
    ns3Bridge->initNetwork(mavsdk.systems().size()); 

    std::cout << "Done initing networking...\n" << std::endl;

    std::vector<std::thread> threads;
    uint32_t i = 0;
    for (auto system : mavsdk.systems()) {
        std::cout << "Creating drone " << i << std::endl;
        auto socket = ns3Bridge->getSocket(i);

        std::vector<uint32_t> localPeerIds(peer_ids);
        localPeerIds.erase(localPeerIds.begin() + i);

        std::thread t(&droneProcess, std::ref(system), i, std::move(localPeerIds), std::move(socket));
        threads.push_back(std::move(t));
        sleep_for(seconds(1));
        i++;
    }

    auto systems = mavsdk.systems();
    std::thread posUpdate(&updateProcess, &systems, ns3Bridge);

    for (auto& t : threads) {
        t.join();
    }
    running = false;

    ns3Bridge->stopSimulation();
    delete ns3Bridge;
    return 0;
}

const double EARTH_RADIUS_EQUATORIAL = 6378137.0; // in meters
const double METERS_PER_DEGREE_LAT = 111132.0;    // Approximate meters per degree of latitude

void latLngToMeters(double latitude, double longitude, double& x, double& y) {
    // Convert latitude and longitude to radians
    double latRad = latitude * M_PI / 180.0;
    double lonRad = longitude * M_PI / 180.0;

    // Calculate meters per degree of longitude at the given latitude
    double metersPerDegreeLon = M_PI * EARTH_RADIUS_EQUATORIAL * std::cos(latRad) / 180.0;

    // Convert lat/lng to meters
    x = longitude * metersPerDegreeLon;
    y = latitude * METERS_PER_DEGREE_LAT;
}

void metersToLatLng(double x, double y, double &latitude, double &longitude) {
    // Calculate latitude from y meters
    latitude = y / METERS_PER_DEGREE_LAT;

    // Convert latitude to radians for longitude calculation
    double latRad = latitude * M_PI / 180.0;

    // Calculate meters per degree of longitude at the given latitude
    double metersPerDegreeLon = M_PI * EARTH_RADIUS_EQUATORIAL * std::cos(latRad) / 180.0;

    // Calculate longitude from x meters
    longitude = x / metersPerDegreeLon;
}

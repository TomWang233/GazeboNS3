#include <iostream>
#include <sstream>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cmath>

#include "world_parser.hpp"

using namespace tinyxml2;

std::optional<std::pair<double, Position>> serverQuery(const char* server_ip);

std::vector<std::pair<double, Position>> actorPositions;
Position hqPosition;
double delay;

void worldInit() {
    XMLDocument doc;
    // This shouldn't be hardcoded but fine for now
    auto result = doc.LoadFile( "../gazebo/data/worlds/iris_runway.sdf" );

    std::cout << "XML Read Result: " << result << std::endl;

    const XMLElement* sdf = doc.FirstChildElement("sdf");
    actorPositions = std::move(readActor(sdf, delay));
}

std::optional<PairPos> getHQAndActorPositions() {
    // Interpolate actor position
    const char* HOST = getenv("SERVER_HOST");
    auto queryResults = serverQuery(HOST != NULL ? HOST : "127.0.0.1");

    if (!queryResults.has_value()) {
        return std::optional<PairPos>();
    }

    auto simTime = queryResults->first;
    simTime -= delay;


    auto headquartersPosition = queryResults->second;

    auto lastTime = actorPositions[0].first;
    auto lastPos = actorPositions[0].second;
    if (simTime < 0) {
        return std::pair(headquartersPosition, lastPos);
    }

    auto highestTime = actorPositions[actorPositions.size()-1].first;
    simTime = std::fmod(simTime, highestTime);

    for (int i=0; i<actorPositions.size(); i++) {
        auto curPos = &actorPositions[i];
        auto time = curPos->first;
        auto pos = curPos->second;

        if (simTime < time) {
            auto timeDelta = (simTime - lastTime) / (time - lastTime);
            return std::pair(
                headquartersPosition,
                Position(
                    lastPos.x + (pos.x - lastPos.x) * timeDelta,
                    lastPos.y + (pos.y - lastPos.y) * timeDelta,
                    0.0
                )
            );
        }
        lastTime = time;
        lastPos = pos;
    }
    return std::optional<PairPos>();
}

std::vector<std::pair<double, Position>> readActor(const XMLElement* sdf, double &delay) {
    const XMLElement* actor = sdf->FirstChildElement("world")->FirstChildElement("actor");

    delay = std::stod(actor->FirstChildElement("script")->FirstChildElement("delay_start")->GetText());
    auto trajectory = actor->FirstChildElement("script")->FirstChildElement("trajectory");

    auto childrenN = trajectory->ChildElementCount("waypoint");
    auto waypoint = trajectory->FirstChildElement("waypoint");

    std::vector<std::pair<double, Position>> positions;
    for (int i=0; i<childrenN; i++) {
        auto time = std::stod(waypoint->FirstChildElement("time")->GetText());
        auto pose = waypoint->FirstChildElement("pose")->GetText();

        std::istringstream iss(pose);
        std::vector<double> numbers;
        double splitPose;

        // Read each number from the stringstream into the vector
        while (iss >> splitPose) {
            numbers.push_back(splitPose);
        }

        auto position = Position(numbers[0], numbers[1], numbers[2]);
        positions.push_back(std::pair(time, position));
        waypoint = waypoint->NextSiblingElement("waypoint");
    }

    return positions;
}

std::optional<std::pair<double, Position>> serverQuery(const char* server_ip) {
    const int server_port = 3030;  // Server port

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        return std::optional<std::pair<double, Position>>();
    }

    // Server address setup
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("Invalid address or Address not supported");
        close(sockfd);
        return std::optional<std::pair<double, Position>>();
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return std::optional<std::pair<double, Position>>();
    }

    char buffer[1024];
    double time = 0.0;
    double x = 0.0, y = 0.0, z = 0.0;

    // Read data from the socket
    ssize_t bytes_read = read(sockfd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        // If no data or connection is closed
        return std::optional<std::pair<double, Position>>();
    }
    
    buffer[bytes_read] = '\0';  // Null-terminate the received data
    
    // Parse the data
    std::istringstream stream(buffer);
    stream >> time; // Parse the time
    stream >> x >> y >> z; // Parse x, y, z

    // Close the socket
    close(sockfd);
    return std::pair(time, Position(x, y, z));
}

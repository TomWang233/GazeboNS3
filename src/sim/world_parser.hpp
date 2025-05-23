#pragma once

#include <vector>
#include <optional>
#include "src/libs/tinyxml2.h"

struct Position {
    double x;
    double y;
    double altitude;

    Position(double _x, double _y, double _altitude) : x(_x), y(_y), altitude(_altitude) {}
    Position() : x(0), y(0), altitude(0) {}
    Position(const Position& p) : x(p.x), y(p.y), altitude(p.altitude) {}
    Position(const Position&& p) : x(p.x), y(p.y), altitude(p.altitude) {}

    void operator=(Position& p) {
        x = p.x;
        y = p.y;
        altitude = p.altitude;
    }
};

typedef std::pair<Position, Position> PairPos;

void worldInit();

std::vector<std::pair<double, Position>> readActor(const tinyxml2::XMLElement* sdf, double &delay);
std::optional<PairPos> getHQAndActorPositions();

#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace domain {

struct Stop {
	std::string name;
	geo::Coordinates coordinates;
};

struct Bus {
	std::string name;
	std::vector<const Stop*> route;
	bool ring_route = false;
};

struct BusStat {
	std::string_view name;
	int stops;
	int unic_stops;
	int length;
	double curvature;
};

} // namespace domain
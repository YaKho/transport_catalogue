#include <stdexcept>
#include <unordered_set>

#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coordinates) {
	stops_.push_back({ name, coordinates });
	name_to_stop_[stops_.back().name] = &stops_.back();
	buses_at_stop_[stops_.back().name];
}

void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stops, bool ring_route) {
	domain::Bus bus;
	bus.name = name;
	bus.ring_route = ring_route;

	for (const auto& stop : stops) {
		bus.route.push_back(&GetStop(stop));
	}
	buses_.push_back(std::move(bus));
	name_to_bus_[buses_.back().name] = &buses_.back();

	for (const auto& stop : stops) {
		buses_at_stop_[name_to_stop_.at(stop)->name].insert(buses_.back().name);
	}
}

const domain::BusStat TransportCatalogue::GetBusInfo(const std::string& name) const {
	const auto& bus = GetBus(name);
	const std::string_view bus_name = bus.name;
	int stops_count = static_cast<int>(bus.route.size());
	int unic_stops = 0;
	double route_length_straight = 0.0;
	int route_length = 0;
	const domain::Stop* pre_stop = nullptr;
	std::unordered_set<std::string_view> tmp;

	for (const auto& stop : bus.route) {
		tmp.insert(stop->name);
		if (pre_stop) {
			route_length_straight += ComputeDistance(pre_stop->coordinates, stop->coordinates);
			route_length += GetDistance(pre_stop->name, stop->name);
		}
		pre_stop = stop;
	}
	if (bus.ring_route == false) {
		route_length_straight *= 2;
		stops_count = 2 * stops_count - 1;
		for (auto it_from = bus.route.rbegin(), it_to = it_from + 1; it_to < bus.route.rend(); ++it_to, ++it_from) { // count backward distance
			route_length += GetDistance((*it_from)->name, (*it_to)->name);
		}
	}
	unic_stops = static_cast<int>(tmp.size());

	double curvature = route_length / route_length_straight;

	return { bus_name, stops_count, unic_stops, route_length, curvature };
}

const std::set<std::string_view>& TransportCatalogue::GetBusesAtStop(const std::string& name) const {
	if (buses_at_stop_.count(name) == 0) {
		throw std::out_of_range("Stop " + name + ": not found");
	}
	return buses_at_stop_.at(name);

}

void TransportCatalogue::SetDistance(const std::string& from, const std::string& to, int distance) {
	distances_[{ &GetStop(from), &GetStop(to) }] = distance;
}

int TransportCatalogue::GetDistance(const std::string& from, const std::string& to) const {
	if (distances_.count({ &GetStop(from), &GetStop(to) })) {
		return distances_.at({ &GetStop(from), &GetStop(to) });
	} else if (distances_.count({ &GetStop(to), &GetStop(from) })) {
		return distances_.at({ &GetStop(to), &GetStop(from) });
	} else {
		throw std::out_of_range("no distance between " + to + " and " + from);
	}

	return 0;
}

const std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>,
	int, DistancesHash>& TransportCatalogue::GetAllDistances() const {
	return distances_;
}

const std::unordered_map<std::string_view, const domain::Bus*>& TransportCatalogue::GetAllBuses() const {
	return name_to_bus_;
}

const std::unordered_map<std::string_view, const domain::Stop*>& TransportCatalogue::GetAllStops() const {
	return name_to_stop_;
}

const domain::Stop& TransportCatalogue::GetStop(const std::string& name) const {
	if (name_to_stop_.count(name) == 0) {
		throw std::out_of_range("Stop " + name + ": not found");
	}
	return *name_to_stop_.at(name);
}

const domain::Bus& TransportCatalogue::GetBus(const std::string& name) const {
	if (name_to_bus_.count(name) == 0) {
		throw std::out_of_range("Bus " + name + ": not found");
	}
	return *name_to_bus_.at(name);
}

} // transport_catalogue
#pragma once

#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <deque>

#include "domain.h"

namespace transport_catalogue {

struct DistancesHash {
	std::hash<const void*> hasher;

	size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*> dist) const {
		return hasher(dist.first) * 37 + hasher(dist.second);
	}
};

class TransportCatalogue {
public:
	TransportCatalogue() = default;

	void AddStop(const std::string& name, geo::Coordinates coordinates);

	void AddBus(const std::string& name, const std::vector<std::string>& stops, bool ring_route);

	const domain::BusStat GetBusInfo(const std::string& name) const;

	const std::set<std::string_view>& GetBusesAtStop(const std::string& name) const;

	void SetDistance(const std::string& from, const std::string& to, int distance);

	int GetDistance(const std::string& from, const std::string& to) const;

	const std::unordered_map<std::string_view, const domain::Bus*>& GetAllBuses() const;

	const std::unordered_map<std::string_view, const domain::Stop*>& GetAllStops() const;

	const std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>,
		int, DistancesHash>& GetAllDistances() const;
	
private:
	std::deque<domain::Stop> stops_;
	std::deque<domain::Bus> buses_;

	std::unordered_map<std::string_view, const domain::Stop*> name_to_stop_;
	std::unordered_map<std::string_view, const domain::Bus*> name_to_bus_;
	std::unordered_map<std::string_view, std::set<std::string_view>> buses_at_stop_;
	std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, int, DistancesHash> distances_;

	const domain::Stop& GetStop(const std::string& name) const;

	const domain::Bus& GetBus(const std::string& name) const;
};

} // namespace transport_catalogue
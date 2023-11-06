#include <cassert>
#include <string>
#include <iostream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "tests.h"
#include "transport_catalogue.h"

namespace transport_catalogue::tests {

void TestTransportCatalogue() {
	using namespace transport_catalogue;

	TransportCatalogue catalog;
	catalog.AddStop("Tolstopaltsevo", { 55.611087, 37.208290 });
	catalog.AddStop("Marushkino", { 55.595884, 37.209755 });
	catalog.AddStop("Rasskazovka", { 55.632761, 37.333324 });
	catalog.AddStop("Biryulyovo Zapadnoye", { 55.574371, 37.651700 });
	catalog.AddStop("Biryusinka", { 55.581065, 37.648390 });
	catalog.AddStop("Universam", { 55.587655, 37.645687 });
	catalog.AddStop("Biryulyovo Tovarnaya", { 55.592028, 37.653656 });
	catalog.AddStop("Biryulyovo Passazhirskaya", { 55.580999, 37.659164 });
	catalog.AddStop("Rossoshanskaya ulitsa", { 55.595579, 37.605757 });
	catalog.AddStop("Prazhskaya", { 55.611678, 37.603831 });

	catalog.SetDistance("Tolstopaltsevo", "Marushkino", 3900);
	catalog.SetDistance("Marushkino", "Rasskazovka", 9900);
	catalog.SetDistance("Marushkino", "Marushkino", 100);
	catalog.SetDistance("Rasskazovka", "Marushkino", 9500);
	catalog.SetDistance("Biryulyovo Zapadnoye", "Rossoshanskaya ulitsa", 7500);
	catalog.SetDistance("Biryulyovo Zapadnoye", "Biryusinka", 1800);
	catalog.SetDistance("Biryulyovo Zapadnoye", "Universam", 2400);
	catalog.SetDistance("Biryusinka", "Universam", 750);
	catalog.SetDistance("Universam", "Rossoshanskaya ulitsa", 5600);
	catalog.SetDistance("Universam", "Biryulyovo Tovarnaya", 900);
	catalog.SetDistance("Biryulyovo Tovarnaya", "Biryulyovo Passazhirskaya", 1300);
	catalog.SetDistance("Biryulyovo Passazhirskaya", "Biryulyovo Zapadnoye", 1200);

	catalog.AddBus("256", { "Biryulyovo Zapadnoye", "Biryusinka", "Universam", "Biryulyovo Tovarnaya", "Biryulyovo Passazhirskaya", "Biryulyovo Zapadnoye" }, true);
	catalog.AddBus("750", { "Tolstopaltsevo", "Marushkino", "Marushkino", "Rasskazovka" }, false);
	catalog.AddBus("828", { "Biryulyovo Zapadnoye", "Universam", "Rossoshanskaya ulitsa", "Biryulyovo Zapadnoye" }, true);

	auto res = catalog.GetBusInfo("256");

	assert(res.name == "256");
	assert(res.stops == 6);
	assert(res.unic_stops == 5);
	assert(res.length == 5950);
	assert(std::abs(res.curvature - 1.36124) < 1e-5);

	res = catalog.GetBusInfo("750");

	assert(res.name == "750");
	assert(res.stops == 7);
	assert(res.unic_stops == 3);
	assert(res.length == 27400);
	assert(std::abs(res.curvature - 1.30853) < 1e-5);

	try {
		const auto buses_at_stop = catalog.GetBusesAtStop("Samara");
	} catch (std::out_of_range& e) {
		assert(std::string(e.what()) == "Stop Samara: not found");
	}

	{
		const auto buses_at_stop = catalog.GetBusesAtStop("Prazhskaya");
		assert(buses_at_stop.size() == 0);
	}

	{
		const auto buses_at_stop = catalog.GetBusesAtStop("Biryulyovo Zapadnoye");
		assert(buses_at_stop.size() == 2);
		assert(*buses_at_stop.cbegin() == "256");
	}

	std::cout << __FUNCTION__ << " OK" << std::endl;
}

void TestJSONReader() {


	std::cout << __FUNCTION__ << " OK" << std::endl;
}

void TestAll() {
	using namespace transport_catalogue;

	TransportCatalogue catalog;

	std::cout << __FUNCTION__ << " OK" << std::endl;
}

} // tests
#pragma once
#include <string>
#include <unordered_map>
#include <set>
#include <deque>
#include <vector>
#include <tuple>

#include "geo.h"

namespace transport_catalogue{
	struct Stop{
		std::string name;
		geography::Coordinates coordinates;

		size_t operator()(const Stop& stop) const {
			return std::hash<std::string>{}(stop.name);
		}

		bool operator==(const Stop& stop) const{
			return name == stop.name && coordinates.lat == stop.coordinates.lat && coordinates.lng == stop.coordinates.lng;
		}
	};

	struct Bus{
		std::string name;
		std::deque<const Stop*> stops;

		size_t operator()(const Bus* bus) const {
			return std::hash<std::string>{}(bus->name);
		}

		bool operator==(const Bus& bus) const{
			return bus.name == name && std::equal(stops.begin(), stops.end(), bus.stops.begin(), bus.stops.end());
		}

		bool operator<(const Bus& bus) const{
			return name < bus.name;
		}
	};

	struct BusInfo{
		int R;
		int U;
		double L; 

		bool operator==(const BusInfo& info){
			return std::tie(info.L, info.R, info.U) == std::tie(L, R, U);
		}
	};

	class TransportCatalogue {
	public:
		TransportCatalogue() = default;

		void AddStop(std::string name, geography::Coordinates coord);

		void AddBus(std::string name, const std::vector<std::string_view>& stops_name);

		const Bus* FindBus(std::string_view name) const;

		const Stop* FindStop(std::string_view name) const;

		const BusInfo GetInfo(std::string_view name) const;

		const std::pair<std::string_view, std::vector<std::string_view>> GetStopInfo(std::string_view name) const;
	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, const Stop*> stops_points_;
		std::unordered_map<std::string_view, const Bus*> buses_points_;
		std::unordered_map<std::string_view, std::vector<std::string_view>> stop_to_buses;
	};
}
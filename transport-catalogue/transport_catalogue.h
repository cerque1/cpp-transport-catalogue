#pragma once
#include <string>
#include <unordered_map>
#include <set>
#include <deque>
#include <vector>
#include <tuple>

#include "geo.h"

namespace transport_catalogue {

	namespace details{
		struct StringPairHashes{
			size_t operator()(std::pair<std::string, std::string> other) const {
				std::hash<std::string> hasher;
				return hasher(other.first) * 37 + hasher(other.second) * 37 * 37;
			}
		};
	}

	struct Stop {
		std::string name;
		geography::Coordinates coordinates;

		size_t operator()(const Stop& stop) const {
			return std::hash<std::string>{}(stop.name);
		}

		bool operator==(const Stop& stop) const{
			return name == stop.name && coordinates.lat == stop.coordinates.lat && coordinates.lng == stop.coordinates.lng;
		}
	};

	struct Bus {
		std::string name;
		std::vector<const Stop*> stops;

		size_t operator()(const Bus* bus) const {
			return std::hash<std::string>{}(bus->name);
		}

		bool operator==(const Bus& bus) const{
			return bus.name == name && std::equal(stops.begin(), stops.end(), bus.stops.begin(), bus.stops.end());
		}
	};

	struct BusInfo {
		int stops_route;
		int unique_stops;
		int length; 
		double curvature;

		bool operator==(const BusInfo& info){
			return std::tie(info.length, info.stops_route, info.unique_stops) == std::tie(length, stops_route, unique_stops);
		}
	};

	struct DistanceToStop{
		std::string stop_name;
		int distance;
	};

	class TransportCatalogue {
	public:
		TransportCatalogue() = default;

		void AddStop(const std::string& name, geography::Coordinates coord);

		void AddBus(const std::string& name, const std::vector<std::string_view>& stops_name);

		void AddDistance(const std::string& stop_from, const std::string& stop_to, int distance);

		const Bus* FindBus(std::string_view name) const;

		const Stop* FindStop(std::string_view name) const;

		int FindDistance(const std::string& stop_from, const std::string& stop_to) const;

		BusInfo GetInfo(const Bus* bus) const;

		std::vector<std::string_view> GetStopInfo(std::string_view name) const;
	private:
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, const Stop*> stops_points_;
		std::unordered_map<std::string_view, const Bus*> buses_points_;
		std::unordered_map<std::string_view, std::vector<std::string_view>> stop_to_buses_;
		std::unordered_map<std::pair<std::string, std::string>, int, details::StringPairHashes> distances_;
	};
}
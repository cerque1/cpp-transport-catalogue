#pragma once
#include <string>
#include <unordered_map>
#include <set>
#include <deque>
#include <vector>
#include <tuple>

#include "geo.h"
#include "domain.h"

namespace transport_catalogue {

	namespace details{
		
		struct StringPairHashes{
			size_t operator()(std::pair<std::string, std::string> other) const {
				std::hash<std::string> hasher;
				return hasher(other.first) * 37 + hasher(other.second) * 37 * 37;
			}
		};
	}

	struct DistanceToStop{
		std::string stop_name;
		int distance;
	};

	class TransportCatalogue {
	public:
		TransportCatalogue() = default;

		void AddStop(const std::string& name, geo::Coordinates coord);

		void AddBus(const std::string& name, const std::vector<std::string_view>& stops_name, bool is_round);

		void AddDistance(const std::string& stop_from, const std::string& stop_to, int distance);

		const std::unordered_map<std::string_view, const Bus*>& GetBusesPoints() const;

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
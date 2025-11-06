#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <chrono>

using namespace std;
using Clock = std::chrono::system_clock;

// ---------- Enums & helpers ----------
enum class VehicleType { Bike, Car, Truck };
enum class SpotSize   { Small, Medium, Large };

static string to_string(VehicleType t) {
    switch (t) {
        case VehicleType::Bike:  return "Bike";
        case VehicleType::Car:   return "Car";
        case VehicleType::Truck: return "Truck";
    }
    return "?";
}
static string to_string(SpotSize s) {
    switch (s) {
        case SpotSize::Small:  return "Small";
        case SpotSize::Medium: return "Medium";
        case SpotSize::Large:  return "Large";
    }
    return "?";
}

// ---------- Core entities ----------
struct Vehicle {
    string registration;
    VehicleType type;

    Vehicle(string reg, VehicleType t) : registration(std::move(reg)), type(t) {}
};

struct ParkingSpot {
    int id;            // globally unique (in this demo)
    int floorIdx;
    SpotSize size;
    bool occupied{false};
    optional<int> currentTicketId;

    bool canFit(VehicleType vt) const {
        // Fit matrix:
        // Bike  -> Small/Medium/Large
        // Car   -> Medium/Large
        // Truck -> Large
        if (vt == VehicleType::Bike)  return true;
        if (vt == VehicleType::Car)   return size != SpotSize::Small;
        if (vt == VehicleType::Truck) return size == SpotSize::Large;
        return false;
    }
};

struct ParkingFloor {
    int index;
    vector<ParkingSpot> spots;

    // availability counters
    int freeSmall{0}, freeMedium{0}, freeLarge{0};

    void bump(SpotSize sz, int delta) {
        if (sz == SpotSize::Small)  freeSmall  += delta;
        if (sz == SpotSize::Medium) freeMedium += delta;
        if (sz == SpotSize::Large)  freeLarge  += delta;
    }

    void addSpot(const ParkingSpot& s) {
        spots.push_back(s);
        if (!s.occupied) bump(s.size, +1);
    }

    ParkingSpot* findFreeOfSize(SpotSize sz) {
        for (auto &s : spots) {
            if (!s.occupied && s.size == sz) return &s;
        }
        return nullptr;
    }
};

struct Ticket {
    int id;
    Vehicle* vehicle{nullptr};       // direct mapping
    ParkingSpot* spot{nullptr};      // direct mapping
    Clock::time_point entryTime;
    optional<Clock::time_point> exitTime;

    long long minutesParked() const {
        auto end = exitTime.value_or(Clock::now());
        return chrono::duration_cast<chrono::minutes>(end - entryTime).count();
    }
};

// ---------- Pricing Strategy Pattern ----------
class PricingStrategy {
public:
    virtual ~PricingStrategy() = default;
    virtual double computeFee(long long minutes, VehicleType type) const = 0;

protected:
    static long long billableHours(long long minutes) {
        long long hrs = (minutes + 59) / 60;   // round up
        return std::max(1LL, hrs);
    }
};

class FlatRateStrategy : public PricingStrategy {
    double perHour;
public:
    explicit FlatRateStrategy(double ratePerHour) : perHour(ratePerHour) {}
    double computeFee(long long minutes, VehicleType) const override {
        return billableHours(minutes) * perHour;
    }
};

class VehicleTypeRateStrategy : public PricingStrategy {
    unordered_map<VehicleType, double> rate;
public:
    VehicleTypeRateStrategy(double bikeRate, double carRate, double truckRate) {
        rate[VehicleType::Bike]  = bikeRate;
        rate[VehicleType::Car]   = carRate;
        rate[VehicleType::Truck] = truckRate;
    }
    double computeFee(long long minutes, VehicleType type) const override {
        double perHour = rate.at(type);
        return billableHours(minutes) * perHour;
    }
};

// ---------- ParkingLot orchestrator ----------
class ParkingLot {
    vector<ParkingFloor> floors;
    unique_ptr<PricingStrategy> pricing;

    int nextTicketId{1};
    unordered_map<int, Ticket> tickets;                 // ticketId -> Ticket
    unordered_map<string, unique_ptr<Vehicle>> vehicles;// reg -> Vehicle
    unordered_map<string, int> activeTicketByReg;       // reg -> ticketId

    static vector<SpotSize> preferredSizes(VehicleType vt) {
        if (vt == VehicleType::Bike)  return {SpotSize::Small, SpotSize::Medium, SpotSize::Large};
        if (vt == VehicleType::Car)   return {SpotSize::Medium, SpotSize::Large};
        return {SpotSize::Large}; // Truck
    }

    ParkingSpot* findBestSpot(VehicleType vt, int& outFloorIdx) {
        auto sizes = preferredSizes(vt);
        for (auto &floor : floors) {
            for (auto sz : sizes) {
                ParkingSpot* s = floor.findFreeOfSize(sz);
                if (s && s->canFit(vt)) {
                    outFloorIdx = floor.index;
                    return s;
                }
            }
        }
        return nullptr;
    }

public:
    explicit ParkingLot(unique_ptr<PricingStrategy> strategy)
        : pricing(std::move(strategy)) {}

    ParkingFloor& addFloor(int index) {
        floors.push_back(ParkingFloor{index});
        return floors.back();
    }

    void setPricing(unique_ptr<PricingStrategy> strategy) {
        pricing = std::move(strategy);
    }

    Vehicle* getOrCreateVehicle(const string& reg, VehicleType t) {
        auto it = vehicles.find(reg);
        if (it == vehicles.end()) {
            vehicles[reg] = make_unique<Vehicle>(reg, t);
            return vehicles[reg].get();
        }
        // If vehicle exists but type changed, keep original type (real systems would reconcile).
        return it->second.get();
    }

    // Park: auto-assign best-fit spot, create ticket, return ticket id
    optional<int> park(const string& reg, VehicleType type, optional<long long> backdateMinutes = nullopt) {
        if (activeTicketByReg.count(reg)) {
            cout << "Vehicle " << reg << " already parked with ticket " << activeTicketByReg[reg] << "\n";
            return nullopt;
        }

        Vehicle* v = getOrCreateVehicle(reg, type);

        int floorIdx = -1;
        ParkingSpot* s = findBestSpot(v->type, floorIdx);
        if (!s) {
            cout << "No suitable spot available for " << to_string(v->type) << " [" << reg << "]\n";
            return nullopt;
        }

        // Occupy spot & floor counters
        s->occupied = true;
        floors[s->floorIdx].bump(s->size, -1);

        // Create ticket
        Ticket t;
        t.id = nextTicketId++;
        t.vehicle = v;
        t.spot = s;
        t.entryTime = Clock::now();
        if (backdateMinutes && *backdateMinutes > 0) {
            t.entryTime -= chrono::minutes(*backdateMinutes);
        }

        tickets[t.id] = t;
        s->currentTicketId = t.id;
        activeTicketByReg[reg] = t.id;

        cout << "Parked " << to_string(v->type) << " [" << reg << "] at "
             << "Floor " << s->floorIdx << ", Spot " << s->id
             << " (" << to_string(s->size) << "). Ticket " << t.id << "\n";

        return t.id;
    }

    // Unpark by ticket, release spot, compute fee
    optional<double> unpark(int ticketId) {
        auto it = tickets.find(ticketId);
        if (it == tickets.end()) {
            cout << "Invalid ticket " << ticketId << "\n";
            return nullopt;
        }
        Ticket &t = it->second;
        if (t.exitTime.has_value()) {
            cout << "Ticket " << ticketId << " already closed.\n";
            return nullopt;
        }

        t.exitTime = Clock::now();
        long long mins = t.minutesParked();
        double fee = pricing->computeFee(mins, t.vehicle->type);

        // Release spot & counters
        ParkingSpot* s = t.spot;
        s->occupied = false;
        s->currentTicketId.reset();
        floors[s->floorIdx].bump(s->size, +1);

        activeTicketByReg.erase(t.vehicle->registration);

        cout << "Unparked " << to_string(t.vehicle->type) << " [" << t.vehicle->registration << "] "
             << "from Floor " << s->floorIdx << ", Spot " << s->id
             << ". Duration " << mins << " min. Fee: " << fee << "\n";

        return fee;
    }

    void printAvailability() const {
        cout << "=== Real-time Availability ===\n";
        for (auto const& f : floors) {
            cout << "Floor " << f.index
                 << " -> Small: "  << f.freeSmall
                 << ", Medium: "   << f.freeMedium
                 << ", Large: "    << f.freeLarge << "\n";
        }
        cout << "==============================\n";
    }
};

// ---------- Demo / Driver ----------
int main() {
    // Pricing: per-vehicle-type hourly rates
    auto pricing = make_unique<VehicleTypeRateStrategy>(
        /*Bike*/ 10.0, /*Car*/ 30.0, /*Truck*/ 50.0
    );
    ParkingLot lot(std::move(pricing));

    // Build floors & spots (configurable)
    // Floor 0: 2 Small, 2 Medium, 1 Large
    {
        auto &f0 = lot.addFloor(0);
        int sid = 0;
        f0.addSpot(ParkingSpot{sid++, 0, SpotSize::Small});
        f0.addSpot(ParkingSpot{sid++, 0, SpotSize::Small});
        f0.addSpot(ParkingSpot{sid++, 0, SpotSize::Medium});
        f0.addSpot(ParkingSpot{sid++, 0, SpotSize::Medium});
        f0.addSpot(ParkingSpot{sid++, 0, SpotSize::Large});
    }
    // Floor 1: 1 Small, 2 Medium, 2 Large
    {
        auto &f1 = lot.addFloor(1);
        int sid = 5; // continue unique IDs
        f1.addSpot(ParkingSpot{sid++, 1, SpotSize::Small});
        f1.addSpot(ParkingSpot{sid++, 1, SpotSize::Medium});
        f1.addSpot(ParkingSpot{sid++, 1, SpotSize::Medium});
        f1.addSpot(ParkingSpot{sid++, 1, SpotSize::Large});
        f1.addSpot(ParkingSpot{sid++, 1, SpotSize::Large});
    }

    lot.printAvailability();

    // Hardcoded parking requests (simulate elapsed time via backdating)
    auto t1 = lot.park("KA01-AA-1111", VehicleType::Bike, 30);   // 30 min
    auto t2 = lot.park("KA02-BB-2222", VehicleType::Car, 95);    // 1h35m -> 2h billed
    auto t3 = lot.park("KA03-CC-3333", VehicleType::Truck, 10);  // 10 min
    auto t4 = lot.park("KA04-DD-4444", VehicleType::Car, 130);   // 2h10m -> 3h billed
    auto t5 = lot.park("KA05-EE-5555", VehicleType::Bike, 61);   // 1h1m -> 2h billed
    auto t6 = lot.park("KA06-FF-6666", VehicleType::Truck, 15);  // maybe fails if no Large

    lot.printAvailability();

    // Unpark a couple
    if (t1) lot.unpark(*t1);
    if (t2) lot.unpark(*t2);

    lot.printAvailability();

    // Switch to flat-rate pricing at runtime (e.g., night tariff)
    lot.setPricing(make_unique<FlatRateStrategy>(25.0));

    if (t3) lot.unpark(*t3);
    if (t4) lot.unpark(*t4);
    if (t5) lot.unpark(*t5);
    if (t6) lot.unpark(*t6);

    lot.printAvailability();

    return 0;
}

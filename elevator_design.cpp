#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <climits>

using namespace std;

// -----------------------------------------------------
// ENUM
// -----------------------------------------------------
enum class Direction { UP, DOWN, IDLE };

// -----------------------------------------------------
// REQUEST
// -----------------------------------------------------
class Request {
public:
    int floor;
    Direction direction;

    Request(int f, Direction d) : floor(f), direction(d) {}
};

class Elevator; // forward declaration

// -----------------------------------------------------
// STRATEGY PATTERN INTERFACE
// -----------------------------------------------------
class IElevatorSelectionStrategy {
public:
    virtual Elevator* selectElevator(
        const vector<Elevator*>& elevators,
        int requestedFloor,
        Direction direction
    ) = 0;

    virtual ~IElevatorSelectionStrategy() = default;
};

// -----------------------------------------------------
// ELEVATOR CLASS (SCAN scheduling built-in)
// -----------------------------------------------------
class Elevator {
private:
    int id;
    int currentFloor;
    Direction dir;

    // SCAN queues
    priority_queue<int, vector<int>, greater<int>> upQueue; // min-heap
    priority_queue<int> downQueue;                          // max-heap

public:
    Elevator(int id) : id(id), currentFloor(0), dir(Direction::IDLE) {}

    int getId() const { return id; }
    int getCurrentFloor() const { return currentFloor; }
    Direction getDirection() const { return dir; }
    bool isIdle() const { return dir == Direction::IDLE; }

    int getPendingRequests() const {
        return upQueue.size() + downQueue.size();
    }

    // Pressing button inside elevator
    void pressFloorButton(int floor) {
        cout << "Passenger pressed floor " << floor
             << " inside Elevator " << id << "\n";
        addInternalRequest(floor);
    }

    // Internal request (destination)
    void addInternalRequest(int floor) {
        if (floor == currentFloor) return;

        if (floor > currentFloor)
            upQueue.push(floor);
        else
            downQueue.push(floor);

        updateDirection();
    }

    // External request (pickup)
    void addExternalRequest(const Request& r) {
        addInternalRequest(r.floor);
    }

    // Simulation tick
    void step() {
        switch (dir) {
            case Direction::UP: moveUp(); break;
            case Direction::DOWN: moveDown(); break;
            case Direction::IDLE:
                cout << "Elevator " << id << " is IDLE at floor " << currentFloor << "\n";
                break;
        }
    }

private:
    void updateDirection() {
        if (dir == Direction::IDLE) {
            if (!upQueue.empty()) dir = Direction::UP;
            else if (!downQueue.empty()) dir = Direction::DOWN;
            return;
        }

        if (dir == Direction::UP && upQueue.empty()) {
            if (!downQueue.empty()) dir = Direction::DOWN;
            else dir = Direction::IDLE;
        }

        if (dir == Direction::DOWN && downQueue.empty()) {
            if (!upQueue.empty()) dir = Direction::UP;
            else dir = Direction::IDLE;
        }
    }

    void moveUp() {
        currentFloor++;
        cout << "[UP] Elevator " << id << " → floor " << currentFloor << "\n";

        if (!upQueue.empty() && currentFloor == upQueue.top()) {
            cout << "Elevator " << id << " opening doors at " << currentFloor << "\n";
            upQueue.pop();
        }
        updateDirection();
    }

    void moveDown() {
        currentFloor--;
        cout << "[DOWN] Elevator " << id << " → floor " << currentFloor << "\n";

        if (!downQueue.empty() && currentFloor == downQueue.top()) {
            cout << "Elevator " << id << " opening doors at " << currentFloor << "\n";
            downQueue.pop();
        }
        updateDirection();
    }
};

// -----------------------------------------------------
// STRATEGY 1: NEAREST IDLE ELEVATOR
// -----------------------------------------------------
class NearestIdleElevatorStrategy : public IElevatorSelectionStrategy {
public:
    Elevator* selectElevator(const vector<Elevator*>& elevators,
                             int floor,
                             Direction d) override {
        Elevator* best = nullptr;
        int minDist = INT_MAX;

        for (auto e : elevators) {
            if (!e->isIdle()) continue;
            int dist = abs(e->getCurrentFloor() - floor);
            if (dist < minDist) {
                minDist = dist;
                best = e;
            }
        }

        if (!best) best = elevators[0];
        return best;
    }
};

// -----------------------------------------------------
// STRATEGY 2: DIRECTION MATCHING
// -----------------------------------------------------
class DirectionMatchingStrategy : public IElevatorSelectionStrategy {
public:
    Elevator* selectElevator(const vector<Elevator*>& elevators,
                             int floor,
                             Direction reqDir) override {
        Elevator* best = nullptr;

        // Prefer elevators already moving in same direction
        for (auto e : elevators) {
            if (e->getDirection() == reqDir) {
                best = e;
                break;
            }
        }

        if (!best) best = elevators[0];
        return best;
    }
};

// -----------------------------------------------------
// STRATEGY 3: LEAST LOAD (fewest pending requests)
// -----------------------------------------------------
class LeastLoadStrategy : public IElevatorSelectionStrategy {
public:
    Elevator* selectElevator(const vector<Elevator*>& elevators,
                             int floor,
                             Direction d) override {
        Elevator* best = nullptr;
        int minLoad = INT_MAX;

        for (auto e : elevators) {
            int load = e->getPendingRequests();
            if (load < minLoad) {
                minLoad = load;
                best = e;
            }
        }

        return best;
    }
};

// -----------------------------------------------------
// ELEVATOR CONTROLLER
// -----------------------------------------------------
class ElevatorController {
private:
    vector<Elevator*> elevators;
    IElevatorSelectionStrategy* strategy;

public:
    ElevatorController(int count, IElevatorSelectionStrategy* s)
        : strategy(s) {
        for (int i = 0; i < count; i++)
            elevators.push_back(new Elevator(i));
    }

    void handleExternalRequest(int floor, Direction d) {
        Elevator* e =
            strategy->selectElevator(elevators, floor, d);

        cout << "[Strategy] Assigning Elevator " << e->getId()
             << " to floor " << floor << "\n";

        e->addExternalRequest(Request(floor, d));
    }

    void pressInsideButton(int elevatorId, int floor) {
        elevators[elevatorId]->pressFloorButton(floor);
    }

    void step() {
        for (auto e : elevators) e->step();
    }
};

// -----------------------------------------------------
// BUILDING
// -----------------------------------------------------
class Building {
private:
    ElevatorController controller;

public:
    Building(int elevators, IElevatorSelectionStrategy* strategy)
        : controller(elevators, strategy) {}

    void pickupRequest(int floor, Direction d) {
        controller.handleExternalRequest(floor, d);
    }

    void pressButtonInsideElevator(int elevatorId, int floor) {
        controller.pressInsideButton(elevatorId, floor);
    }

    void step() { controller.step(); }
};

// -----------------------------------------------------
// MAIN
// -----------------------------------------------------
int main() {

    // ✅ Choose which strategy you want to test:
    // Building building(2, new NearestIdleElevatorStrategy());
    // Building building(2, new DirectionMatchingStrategy());
    Building building(2, new LeastLoadStrategy());

    building.pickupRequest(5, Direction::UP);
    building.pickupRequest(2, Direction::DOWN);
    building.pickupRequest(8, Direction::UP);

    cout << "\n--- Passenger enters Elevator 0 and presses floor 7 ---\n";
    building.pressButtonInsideElevator(0, 7);

    cout << "\n--- Simulation Start ---\n";
    for (int i = 0; i < 20; i++)
        building.step();

    return 0;
}

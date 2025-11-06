#include <bits/stdc++.h>
using namespace std;

enum class State { RUNNING, STOPPED };

struct Machine {
    string id;
    int totalCpu;
    int totalMem;
    int usedCpu = 0;
    int usedMem = 0;
};

struct Container {
    string name;
    string image;
    int cpu;
    int mem;
    string machineId;
    State state = State::RUNNING;   // assigned containers begin as RUNNING
};

class ContainerManager {
private:
    unordered_map<string, Machine> machines;            // machineId -> Machine
    unordered_map<string, Container> containers;        // containerName -> Container

public:

    // 1) Constructor
    ContainerManager(const vector<string>& machineRows) {
        for (auto &row : machineRows) {
            // format: "machineId,totalCpuUnits,totalMemoryMB"
            stringstream ss(row);
            string id, cpuStr, memStr;

            getline(ss, id, ',');
            getline(ss, cpuStr, ',');
            getline(ss, memStr, ',');

            machines[id] = Machine{id, stoi(cpuStr), stoi(memStr), 0, 0};
        }
    }

    // 2) assignMachine()
    string assignMachine(int criteria, string containerName, string imageUrl,
                         int cpuUnits, int memMb)
    {
        if (containerName.empty() || cpuUnits <= 0 || memMb <= 0) return "";
        if (containers.count(containerName)) return "";   // name must be unique

        string bestId = "";
        int bestScore = -1;   // we choose MAX score (CPU or MEM)

        for (auto &p : machines) {
            auto &m = p.second;

            int freeCpu = m.totalCpu - m.usedCpu;
            int freeMem = m.totalMem - m.usedMem;

            if (freeCpu < cpuUnits || freeMem < memMb)
                continue;  // cannot fit

            int score = (criteria == 0 ? freeCpu : freeMem);

            if (score > bestScore) {
                bestScore = score;
                bestId = m.id;
            } else if (score == bestScore && score != -1) {
                // tie → lexicographically smaller machineId
                if (m.id < bestId) bestId = m.id;
            }
        }

        if (bestId == "") return "";

        // Assign container to chosen machine
        machines[bestId].usedCpu += cpuUnits;
        machines[bestId].usedMem += memMb;

        containers[containerName] =
            Container{containerName, imageUrl, cpuUnits, memMb, bestId, State::RUNNING};

        return bestId;
    }

    // 3) stop()
    bool stop(const string &name) {
        auto it = containers.find(name);
        if (it == containers.end()) return false;

        Container &c = it->second;
        if (c.state == State::STOPPED) return false;

        // free machine resources
        Machine &m = machines[c.machineId];
        m.usedCpu -= c.cpu;
        m.usedMem -= c.mem;

        c.state = State::STOPPED;
        return true;
    }
};


/////////////////// SAMPLE USAGE ///////////////////
int main() {
    vector<string> machines = {
        "m1,8,16000",
        "m2,4,8000",
        "a3,10,32000"
    };

    ContainerManager mgr(machines);

    cout << mgr.assignMachine(0, "c1", "nginx", 2, 200) << "\n";  // choose max free CPU → a3
    cout << mgr.assignMachine(1, "c2", "redis", 3, 100) << "\n";  // choose max free MEM → a3
    cout << mgr.assignMachine(0, "c3", "svc", 4, 4000) << "\n";   // may choose m1 if fits

    cout << mgr.stop("c1") << "\n";  // 1 (success)
    cout << mgr.stop("c1") << "\n";  // 0 (already stopped)
}

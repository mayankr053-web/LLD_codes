#include <bits/stdc++.h>
using namespace std;

// -------------------- Utilities ----------------------
static string toLowerCopy(const string& s) {
    string res = s;
    transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
}

// -------------------- Machine Model ----------------------
struct Machine {
    string id;
    unordered_set<string> capabilities;     // lowercase
    int unfinished = 0;
    int finished = 0;
};

// -------------------- Strategy Pattern ----------------------
class AssignmentStrategy {
public:
    virtual ~AssignmentStrategy() = default;

    // Returns best machine index from the candidates
    virtual int select(const vector<Machine*>& machines) = 0;
};

// Criterion 0: least unfinished
class LeastUnfinishedStrategy : public AssignmentStrategy {
public:
    int select(const vector<Machine*>& machines) override {
        int best = -1;
        for (int i = 0; i < machines.size(); i++) {
            if (best == -1 ||
                machines[i]->unfinished < machines[best]->unfinished ||
                (machines[i]->unfinished == machines[best]->unfinished &&
                 machines[i]->id < machines[best]->id))
            {
                best = i;
            }
        }
        return best;
    }
};

// Criterion 1: most finished
class MostFinishedStrategy : public AssignmentStrategy {
public:
    int select(const vector<Machine*>& machines) override {
        int best = -1;
        for (int i = 0; i < machines.size(); i++) {
            if (best == -1 ||
                machines[i]->finished > machines[best]->finished ||
                (machines[i]->finished == machines[best]->finished &&
                 machines[i]->id < machines[best]->id))
            {
                best = i;
            }
        }
        return best;
    }
};

// -------------------- Job Manager ----------------------
class JobManager {
private:
    unordered_map<string, Machine> machines;      // machineId → Machine
    unordered_map<string, string> jobToMachine;   // jobId → machineId

    // Strategy factory
    AssignmentStrategy* getStrategy(int criteria) {
        static LeastUnfinishedStrategy leastUnfinished;
        static MostFinishedStrategy mostFinished;

        if (criteria == 0) return &leastUnfinished;
        if (criteria == 1) return &mostFinished;

        return &leastUnfinished; // default fallback
    }

public:

    // -------------------- API Methods ----------------------

    void addMachine(const string& machineId, const vector<string>& caps) {
        Machine m;
        m.id = machineId;

        for (auto &c : caps)
            m.capabilities.insert(toLowerCopy(c));

        machines[machineId] = std::move(m);
    }

    string assignMachineToJob(const string& jobId,
                              const vector<string>& requiredCaps,
                              int criteria)
    {
        vector<string> req;
        for (auto &r : requiredCaps)
            req.push_back(toLowerCopy(r));

        // Step 1: Collect compatible machines
        vector<Machine*> candidates;

        for (auto& [id, m] : machines) {
            bool ok = true;
            for (auto &cap : req) {
                if (!m.capabilities.count(cap)) {
                    ok = false; break;
                }
            }
            if (ok) candidates.push_back(&machines[id]);
        }

        if (candidates.empty())
            return "";

        // Step 2: Use strategy
        AssignmentStrategy* strategy = getStrategy(criteria);
        int bestIndex = strategy->select(candidates);
        if (bestIndex == -1)
            return "";

        Machine* chosen = candidates[bestIndex];

        // Step 3: Assign job
        chosen->unfinished++;
        jobToMachine[jobId] = chosen->id;

        return chosen->id;
    }

    void jobCompleted(const string& jobId) {
        string machineId = jobToMachine[jobId];
        Machine &m = machines[machineId];

        m.unfinished--;
        m.finished++;

        // The job remains known, but no further use needed.
    }
};

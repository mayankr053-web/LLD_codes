#include <bits/stdc++.h>
using namespace std;

// -------------------- Time Helpers --------------------
using Clock = chrono::steady_clock;
using TimePoint = Clock::time_point;
using Ms = chrono::milliseconds;

static inline TimePoint now() { return Clock::now(); }

// -------------------- Strategy Interface --------------------
class IRateLimiterStrategy {
public:
    virtual ~IRateLimiterStrategy() = default;
    virtual bool allow(const string& user, TimePoint t) = 0;
};

// -------------------- Sliding Window Strategy --------------------
// Stores a deque of timestamps per user.
class SlidingWindowStrategy : public IRateLimiterStrategy {
    int limit_;
    Ms window_;
    unordered_map<string, deque<TimePoint>> hits_;
    mutex m_;

    void removeOld(deque<TimePoint>& dq, TimePoint t) {
        TimePoint cutoff = t - window_;
        while (!dq.empty() && dq.front() < cutoff)
            dq.pop_front();
    }

public:
    SlidingWindowStrategy(int limit, Ms window)
        : limit_(limit), window_(window) {}

    bool allow(const string& user, TimePoint t) override {
        lock_guard<mutex> lock(m_);
        auto& dq = hits_[user];
        removeOld(dq, t);

        if ((int)dq.size() < limit_) {
            dq.push_back(t);
            return true;    // allowed
        }
        return false;       // limit reached
    }
};

// -------------------- Token Bucket Strategy --------------------
// Each user has a bucket with capacity + refill rate.
class TokenBucketStrategy : public IRateLimiterStrategy {
    struct Bucket {
        double tokens{0};
        TimePoint lastRefill{};
    };

    double capacity_;
    double refillPerSec_;
    unordered_map<string, Bucket> buckets_;
    mutex m_;

    void refill(Bucket& b, TimePoint t) {
        using seconds_d = chrono::duration<double>;
        if (b.lastRefill == TimePoint{}) {
            b.lastRefill = t;
            b.tokens = capacity_;
            return;
        }

        double elapsed = chrono::duration_cast<seconds_d>(t - b.lastRefill).count();
        b.tokens = min(capacity_, b.tokens + elapsed * refillPerSec_);
        b.lastRefill = t;
    }

public:
    TokenBucketStrategy(double capacity, double refill)
        : capacity_(capacity), refillPerSec_(refill) {}

    bool allow(const string& user, TimePoint t) override {
        lock_guard<mutex> lock(m_);
        auto& b = buckets_[user];

        if (b.lastRefill == TimePoint{}) {
            b.lastRefill = t;
            b.tokens = capacity_;
        }

        refill(b, t);

        if (b.tokens >= 1.0) {
            b.tokens -= 1.0;
            return true;
        }
        return false;
    }
};

// -------------------- RateLimiter (Context) --------------------
class RateLimiter {
    unique_ptr<IRateLimiterStrategy> strategy_;

public:
    RateLimiter(unique_ptr<IRateLimiterStrategy> s)
        : strategy_(std::move(s)) {}

    bool allow(const string& userId) {
        return strategy_->allow(userId, now());
    }

    void setStrategy(unique_ptr<IRateLimiterStrategy> s) {
        strategy_ = std::move(s);
    }
};

// -------------------- Demo --------------------
int main() {
    // Example 1: Sliding window 5 requests per second
    RateLimiter rl(make_unique<SlidingWindowStrategy>(5, Ms(1000)));
    string user = "user123";

    cout << "--- Sliding Window: 5 req/sec ---\n";
    for (int i = 0; i < 8; i++) {
        cout << (rl.allow(user) ? "Allowed\n" : "Blocked\n");
        this_thread::sleep_for(chrono::milliseconds(150));
    }

    // Swap to Token Bucket
    cout << "\n--- Token Bucket: capacity=5, refill=2 tokens/sec ---\n";
    rl.setStrategy(make_unique<TokenBucketStrategy>(5.0, 2.0));

    for (int i = 0; i < 10; i++) {
        cout << (rl.allow(user) ? "Allowed\n" : "Blocked\n");
        this_thread::sleep_for(chrono::milliseconds(200));
    }

    return 0;
}

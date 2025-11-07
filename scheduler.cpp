class ThreadPool {
public:
    explicit ThreadPool(size_t n) : stop(false) {
        for (size_t i = 0; i < n; i++) {
            workers.emplace_back([this]() { workerLoop(); });
        }
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        for (auto &t : workers) {
            if (t.joinable()) t.join();
        }
    }

    void submit(std::function<void()> func) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(std::move(func));
        }
        cv.notify_one();
    }

private:
    void workerLoop() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]() { return stop || !tasks.empty(); });
                if (stop && tasks.empty())
                    return;

                task = std::move(tasks.front());
                tasks.pop();
            }

            task();
        }
    }

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stop;
};


class ScheduledExecutorService {
public:
    ScheduledExecutorService(size_t schedulerThreads = 1,
                             size_t workerThreads = 4)
        : stop(false),
          pool(workerThreads)
    {
        scheduler = std::thread([this]() { runScheduler(); });
    }

    ~ScheduledExecutorService() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();

        if (scheduler.joinable())
            scheduler.join();
    }

    void schedule(std::function<void()> command, long delayMs) {
        scheduleTask(std::move(command),
                     std::chrono::steady_clock::now() + std::chrono::milliseconds(delayMs),
                     0, TaskType::ONE_SHOT);
    }

    void scheduleAtFixedRate(std::function<void()> command,
                             long initialDelayMs,
                             long periodMs) {
        scheduleTask(std::move(command),
                     std::chrono::steady_clock::now() + std::chrono::milliseconds(initialDelayMs),
                     periodMs, TaskType::FIXED_RATE);
    }

    void scheduleWithFixedDelay(std::function<void()> command,
                                long initialDelayMs,
                                long delayMs) {
        scheduleTask(std::move(command),
                     std::chrono::steady_clock::now() + std::chrono::milliseconds(initialDelayMs),
                     delayMs, TaskType::FIXED_DELAY);
    }

private:
    enum class TaskType { ONE_SHOT, FIXED_RATE, FIXED_DELAY };

    struct Task {
        std::function<void()> func;
        std::chrono::steady_clock::time_point nextRun;
        long intervalMs;
        TaskType type;

        bool operator>(const Task &other) const {
            return nextRun > other.nextRun;
        }
    };

    std::priority_queue<Task, std::vector<Task>, std::greater<>> tasks;
    std::mutex mtx;
    std::condition_variable cv;

    ThreadPool pool;
    std::thread scheduler;
    std::atomic<bool> stop;

    // Push into scheduler queue
    void scheduleTask(std::function<void()> func,
                      std::chrono::steady_clock::time_point nextRun,
                      long intervalMs,
                      TaskType type) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(Task{std::move(func), nextRun, intervalMs, type});
        }
        cv.notify_one();
    }

    // Single scheduler thread
    void runScheduler() {
        while (true) {
            Task task;

            {
                std::unique_lock<std::mutex> lock(mtx);

                cv.wait(lock, [&]() { return stop || !tasks.empty(); });
                if (stop && tasks.empty())
                    return;

                auto now = std::chrono::steady_clock::now();
                Task &next = tasks.top();

                if (now < next.nextRun) {
                    cv.wait_until(lock, next.nextRun);
                    continue;
                }

                task = next;
                tasks.pop();
            }

            // Submit task to ThreadPool
            pool.submit(task.func);

            // Reschedule
            if (task.type == TaskType::FIXED_RATE) {
                task.nextRun += std::chrono::milliseconds(task.intervalMs);
                std::lock_guard<std::mutex> lock(mtx);
                tasks.push(task);
                cv.notify_one();
            } else if (task.type == TaskType::FIXED_DELAY) {
                task.nextRun = std::chrono::steady_clock::now() + std::chrono::milliseconds(task.intervalMs);
                std::lock_guard<std::mutex> lock(mtx);
                tasks.push(task);
                cv.notify_one();
            }
        }
    }
};

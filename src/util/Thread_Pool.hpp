
#pragma once
/*
    Just an utility which allows us to quickly start a bunch of "worker" threads, which are
    notified when to shut down.

    Threads operate on functions:
        void fn(Stop_Flag&, ...)

    Where { Stop_Flag } is just an alias for { std::atomic<bool> } and can be used to notify
    the thread that { Thread_Pool } is being destroyed and thread should stop running.

    { Thread_Pool } is artificially limited to number of hardware threads, though the minimum
    capacity is set to 8.
*/
namespace sphinx {

    using Stop_Flag = std::atomic<bool>;

    static inline void
    sleep_for(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    class Thread_Pool {
    public:
        static constexpr int MIN_CAPACITY { 8 };

    private:
        std::vector<std::thread> threads;
        Stop_Flag                should_stop { false };

    public:
        Thread_Pool();
        ~Thread_Pool();

        [[nodiscard]]
        int
        capacity();

        [[nodiscard]]
        bool
        is_full();

        template <typename Fn, typename... Args>
        void
        start_thread(Fn&& task, Args&&... args) {
            EXPECT(!is_full());
            int thread_index = threads.size();

            threads.emplace_back(
                std::forward<Fn>(task),
                std::ref(should_stop),
                thread_index,
                std::forward<Args>(args)...
            );
        }
    };

}

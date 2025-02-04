
// Implements:
#include <util/Thread_Pool.hpp>

namespace sphinx {

    Thread_Pool::Thread_Pool() {
        int capacity = max(std::thread::hardware_concurrency(), Thread_Pool::MIN_CAPACITY);
        threads.reserve(capacity);
    }

    Thread_Pool::~Thread_Pool() {
        should_stop = true;
        for (std::thread& current_thread: threads) {
            if (current_thread.joinable()) {
                current_thread.join();
            }
        }
    }

    int
    Thread_Pool::capacity() {
        return static_cast<int>(threads.capacity());
    }

    bool
    Thread_Pool::is_full() {
        return !(threads.size() < threads.capacity());
    }

}

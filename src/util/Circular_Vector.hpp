
#pragma once

namespace sphinx {

    template <typename T>
    class Circular_Vector {
    private:
        std::vector<T>      buffer;
        std::atomic<size_t> read_index;
        std::atomic<size_t> write_index;
        std::atomic<size_t> count;
        size_t              capacity;

    public:
        explicit
        Circular_Vector(int capacity):
            buffer(capacity),
            read_index(0),
            write_index(0),
            count(0),
            capacity(capacity) {
        }

        void
        write(const T& data) {
            buffer[write_index] = data;
            write_index         = (write_index + 1) % capacity;

            if (count == capacity) {
                read_index = (read_index + 1) % capacity;
            } else {
                count += 1;
            }
        }

        T&
        read() {
            if (count == 0) {
                static T sentinel;
                return sentinel;
            }

            T& element = buffer[read_index];
            read_index = (read_index + 1) % capacity;
            count      -= 1;

            return element;
        }

        int
        get_count() const {
            return count;
        }

        bool
        is_empty() const {
            return (count == 0);
        }
    };

}

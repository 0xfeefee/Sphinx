
#pragma once

namespace sphinx {

    /*
    @temporary:
    - Do not finalize until encryption step is done!
    */

    template <typename T>
    class SWSR_Buffer {
    private:
        std::vector<T>   buffer;
        std::atomic<int> reader_index;

    public:
        SWSR_Buffer(): reader_index(0) {
            buffer.reserve(32);
        }

        void
        write(const T& data) {
            buffer.push_back(data);
        }

        [[nodiscard]]
        bool
        is_empty() {
            return reader_index == buffer.size();
        }

        [[nodiscard]]
        T&
        read() {
            return buffer[reader_index++];
        }
    };

    /*
        SWSR throwaway shit.
    */
    template <typename T>
    class SWSR_Throwaway {
    private:
        std::vector<T>      buffer;
        std::atomic<size_t> reader_index;

    public:
        SWSR_Throwaway(): reader_index(0) {
            buffer.reserve(32);
        }

        void
        write(const T& data) {
            buffer.push_back(data);
        }

        [[nodiscard]]
        T&
        read() {
            if (reader_index == buffer.size()) {
                static T sentinel;
                return sentinel;
            }

            return buffer[reader_index++];
        }

        void
        clear() {
            buffer.clear();
            reader_index = 0;
        }
    };

    template <typename T, int READER_COUNT>
    class SWMR_Throwaway {
    private:
        int                                         writer_index;
        std::array<SWSR_Throwaway<T>, READER_COUNT> buffers;

    public:
        SWMR_Throwaway(): writer_index(0) {}

        void
        write(const T& data) {
            buffers[writer_index].write(data);
            writer_index = (writer_index + 1) % READER_COUNT;
        }

        [[nodiscard]]
        T&
        read(int reader_index) {
            return buffers[reader_index].read();
        }
    };

}

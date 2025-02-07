
// Implements:
#include <sphinx/PNG_Image.hpp>

namespace sphinx {

    constexpr int MESSAGE_HEADER_SIZE_BITS = 16;

    bool
    PNG_Image::is_loaded() const {
        return (data != nullptr);
    }

    bool
    PNG_Image::is_ready_to_render() const {
        return (texture_id != -1);
    }

    void
    PNG_Image::clear_header() {
        EXPECT(is_loaded());
        for (int i = 0; i < MESSAGE_HEADER_SIZE_BITS; ++i) {
            // Clear the LSB:
            data[i] &= 0xFE;
        }
    }

    bool
    PNG_Image::try_write(const std::string& message, const std::string& image_filename) {
        EXPECT(message.size() > 0);
        printf("Message: %s\n", message.c_str());

        if (!is_loaded()) {
            printf("Image is not loaded yet!");
            return false;
        }

        int message_size_bits = message.size() * 8;
        if ((message_size_bits + MESSAGE_HEADER_SIZE_BITS) > width*height) {
            printf("Message too large!: %d bits!\n", message_size_bits);
            return false;
        }

        u8* message_data = (u8*)message.c_str();
        printf("Message size: %d\n", message_size_bits);

        // Write the header:
        for (int i = 0; i < MESSAGE_HEADER_SIZE_BITS; ++i) {
            // Clear the LSB:
            data[i] &= 0xFE;
            // Write header bit:
            data[i] |= (message_size_bits >> (MESSAGE_HEADER_SIZE_BITS - 1 - i)) & 1UL;
        }

        // Write the message:
        for (int i = 0; i < message_size_bits; ++i) {
            // Clear the LSB:
            data[i + MESSAGE_HEADER_SIZE_BITS] &= 0xFE;
            // Write the message bit:
            data[i + MESSAGE_HEADER_SIZE_BITS] |= (message_data[i/8] >> ((message_size_bits - 1 - i) % 8)) & 1;
        }

        return true;
    }

    std::string
    PNG_Image::read() {
        if (!is_loaded()) {
            return "";
        }

        int message_size_bits = 0;

        // Read the header to get the message size:
        for (int i = 0; i < MESSAGE_HEADER_SIZE_BITS; ++i) {
            message_size_bits = (message_size_bits << 1) | data[i] & 1;
        }

        if (message_size_bits <= 0 || message_size_bits > MAX_MESSAGE_SIZE_BITS) {
            return "";
        }

        std::string message(message_size_bits+1, '\0');
        for (int i = 0; i < message_size_bits; ++i) {
            message[i / 8] = (message[i / 8] << 1) | (data[i + MESSAGE_HEADER_SIZE_BITS] & 1);
        }

        return message;
    }

}


#include <sphinx/AES.hpp>

int main() {
    using namespace sphinx;
    using namespace std;

    // Example usage of AES_128_Key_Expansion
    uint8_t userkey[AES_BLOCK_SIZE] = {
        'M', 'e', 'a', 'n', 'i', 'n', 'g', 'O',
        'f', 'L', 'i', 'f', 'e', '0', '4', '2'
    };

    // 16 bytes per key, 11 keys:
    uint8_t key_schedule[AES_BLOCK_SIZE*AES_KEY_SCHEDULE_COUNT];

    // Expand the user key to a key schedule:
    aes128_expand_key(userkey, key_schedule);

    cout << "Done!\n";
    return 0;
}


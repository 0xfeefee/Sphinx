/*
# AES encryption

    We are using Intel's AES-NI instruction set. This code is based on the official guide from Intel!
    For our purposes we only care about AES128, ECB - which should be perfectly adequate for small
    strings such as passwords, since those are not big enough to be easily defeated with repeating
    pattern analysis.
*/

#include <wmmintrin.h>

namespace sphinx {

    static constexpr int BLOCK_SIZE = 16;

    struct AES_Block {
        u8 data[BLOCK_SIZE] = {};

        u8&
        operator[](int index) {
            ERROR_IF(index >= BLOCK_SIZE);
            return data[index];
        }

        void
        print() {
        #if PROJECT_BUILD_DEBUG
            cout << "[";
            for (u8 c : data) cout << c;
            cout << "]\n";
        #endif
        }
    };

    /*
        User Key will automatically expand to 16 bytes if user provides string which is
        smaller than 16 bytes.
    */
    struct AES_User_Key {
        AES_Block block;

        AES_User_Key(cstr_t key_string) {
            int key_length = cstr_length(key_string);
            ERROR_IF(key_length < 1 || key_length > 16, "Key of invalid length provided!");

            memcpy(block.data, key_string, key_length);
            for (int i = key_length; i < BLOCK_SIZE; ++i) {
                int copy_index = (i-key_length) % key_length;
                block[i] = block[copy_index];
            }

            block.print();
        }
    };

    struct AES128_Context {
        __m128i key_schedule[20];
    };

    /*
        Expands the user key into the key schedule we can use.
    */
    void
    aes128_init(AES128_Context *ctx, AES_User_Key key);

    void
    aes128_encrypt(AES128_Context *ctx, void *pt, const void *ct);

    void
    aes128_decrypt(AES128_Context *ctx, void *ct, const void *pt);

}

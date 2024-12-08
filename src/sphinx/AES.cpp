
// Implements:
#include <sphinx/AES.hpp>
#include <wmmintrin.h>

namespace sphinx {

    /*
        For ease of use we allow user to construct the key from a string implicitly.
    */
    AES_User_Key::AES_User_Key(cstr_t key_string) {
        int key_length = cstr_length(key_string);
        EXPECT(key_length > 0 && key_length <= 16);

        memcpy(block.data, key_string, key_length);
        for (int i = key_length; i < BLOCK_SIZE; ++i) {
            int copy_index = (i-key_length) % key_length;
            block[i] = block[copy_index];
        }
    }

    /*
        Helpers for key expansion rounds:
    */
    #define AES128_KEYROUND(i, rcon) \
        key = key_schedule[i - 1]; \
        gen = _mm_aeskeygenassist_si128(key, rcon); \
        gen = _mm_shuffle_epi32(gen, 255); \
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4)); \
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4)); \
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4)); \
        key_schedule[i] = _mm_xor_si128(key, gen)


    AES128::AES128(AES_User_Key user_key) {
        __m128i key;
        __m128i gen;

        key_schedule[0] = _mm_loadu_si128(reinterpret_cast<const __m128i*>(user_key.block.data));

        AES128_KEYROUND(1, 0x01);
        AES128_KEYROUND(2, 0x02);
        AES128_KEYROUND(3, 0x04);
        AES128_KEYROUND(4, 0x08);
        AES128_KEYROUND(5, 0x10);
        AES128_KEYROUND(6, 0x20);
        AES128_KEYROUND(7, 0x40);
        AES128_KEYROUND(8, 0x80);
        AES128_KEYROUND(9, 0x1b);
        AES128_KEYROUND(10, 0x36);

    #ifdef SPHINX_PRECOMPUTE_REVERSE_KEYS
        key_schedule[11] = _mm_aesimc_si128(key_schedule[9]);
        key_schedule[12] = _mm_aesimc_si128(key_schedule[8]);
        key_schedule[13] = _mm_aesimc_si128(key_schedule[7]);
        key_schedule[14] = _mm_aesimc_si128(key_schedule[6]);
        key_schedule[15] = _mm_aesimc_si128(key_schedule[5]);
        key_schedule[16] = _mm_aesimc_si128(key_schedule[4]);
        key_schedule[17] = _mm_aesimc_si128(key_schedule[3]);
        key_schedule[18] = _mm_aesimc_si128(key_schedule[2]);
        key_schedule[19] = _mm_aesimc_si128(key_schedule[1]);
    #endif
    }

    AES128::~AES128() {}

    void
    AES128::encrypt(void* in, void* out) {
        __m128i data_block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(in));

        data_block = _mm_xor_si128(data_block, key_schedule[0]);
        for (int i = 1; i < 10; ++i) {
            data_block = _mm_aesenc_si128(data_block, key_schedule[i]);
        }
        data_block = _mm_aesenclast_si128(data_block, key_schedule[10]);

        _mm_storeu_si128(reinterpret_cast<__m128i*>(out), data_block);
    }

    void
    AES128::decrypt(void* in, void* out) {
        __m128i data_block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(in));
        data_block = _mm_xor_si128(data_block, key_schedule[10]);

    #ifdef SPHINX_PRECOMPUTE_REVERSE_KEYS
        for (int i = 11; i > 20; --i) {
            data_block = _mm_aesdec_si128(data_block, key_schedule[i]);
        }
    #else
        for (int i = 9; i > 0; --i) {
            data_block = _mm_aesdec_si128(data_block, _mm_aesimc_si128(key_schedule[i]));
        }
    #endif

        data_block = _mm_aesdeclast_si128(data_block, key_schedule[0]);
        _mm_storeu_si128(reinterpret_cast<__m128i*>(out), data_block);
    }


    void
    AES128::encrypt(AES_String& in, AES_String& out) {
        for (int i = 0; i < in.block_count; ++i) {
            AES_Block* b = &in.blocks[i];
            __m128i data_block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(b));

            data_block = _mm_xor_si128(data_block, key_schedule[0]);
            for (int i = 1; i < 10; ++i) {
                data_block = _mm_aesenc_si128(data_block, key_schedule[i]);
            }
            data_block = _mm_aesenclast_si128(data_block, key_schedule[10]);

            _mm_storeu_si128(reinterpret_cast<__m128i*>(&out.blocks[i]), data_block);
        }
    }

    void
    AES128::decrypt(AES_String& in, AES_String& out) {
        for (int i = 0; i < in.block_count; ++i) {
            __m128i data_block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&in.blocks[i]));
            data_block = _mm_xor_si128(data_block, key_schedule[10]);

        #ifdef SPHINX_PRECOMPUTE_REVERSE_KEYS
            for (int i = 11; i > 20; --i) {
                data_block = _mm_aesdec_si128(data_block, key_schedule[i]);
            }
        #else
            for (int i = 9; i > 0; --i) {
                data_block = _mm_aesdec_si128(data_block, _mm_aesimc_si128(key_schedule[i]));
            }
        #endif

            data_block = _mm_aesdeclast_si128(data_block, key_schedule[0]);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(&out.blocks[i]), data_block);
        }
    }
}

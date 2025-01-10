
// Implements:
#include <sphinx/AES.hpp>

// Dependencies:
#include <wmmintrin.h>

namespace sphinx {

    /*
    ## AES_User_Key: implementation
    */
    AES_User_Key::AES_User_Key(cstr_t key_string) {
        int key_length = cstr_length(key_string);
        EXPECT(key_length > 0 && key_length <= 16);

        memcpy(block.data, key_string, key_length);
        for (int i = key_length; i < BLOCK_SIZE; ++i) {
            int copy_index = (i-key_length) % key_length;
            block[i]       = block[copy_index];
        }
    }


    /*
    ## AES_Block: implementation
    */
    u8&
    AES_Block::operator[](int index) {
        EXPECT(index < BLOCK_SIZE);
        return data[index];
    }

    static inline int
    byte_count_to_blocks(int byte_count) {
        return (byte_count + BLOCK_SIZE - 1) / BLOCK_SIZE;
    }


    /*
    ## AES_String: implementation
    */
    AES_String::AES_String(cstr_t text) {
        int length = cstr_length(text);
        EXPECT(length > 0);

        block_count = byte_count_to_blocks(length);
        blocks      = new AES_Block[block_count];

        EXPECT(blocks);
        memcpy(blocks, text, length);
    }

    AES_String::AES_String(const std::string& string):
    AES_String(string.c_str()) {}

    AES_String::AES_String(int size_in_bytes) {
        EXPECT(size_in_bytes > 0);

        block_count = byte_count_to_blocks(size_in_bytes);
        blocks      = new AES_Block[block_count];

        EXPECT(blocks);
    }

    AES_String::AES_String(const AES_String& other) {
        block_count = other.block_count;
        blocks      = new AES_Block[block_count];

        EXPECT(blocks);
        memcpy(blocks, other.blocks, block_count*BLOCK_SIZE);
    }

    AES_String::~AES_String() {
        delete[] blocks;
    }

    int
    AES_String::size_in_bytes() const {
        return block_count * BLOCK_SIZE;
    }

    u8*
    AES_String::byte_ptr() const {
        return reinterpret_cast<u8*>(blocks);
    }

    std::string
    AES_String::to_string() {
        return std::string(reinterpret_cast<char*>(blocks), block_count * BLOCK_SIZE);
    }


    /*
    ## AES128 ECB: implementation
    */
    #define AES128_KEYROUND(i, rcon) \
        key = key_schedule[i - 1]; \
        gen = _mm_aeskeygenassist_si128(key, rcon); \
        gen = _mm_shuffle_epi32(gen, 255); \
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4)); \
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4)); \
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4)); \
        key_schedule[i] = _mm_xor_si128(key, gen)

    AES128::AES128(const AES_User_Key& user_key) {
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

    AES_String
    AES128::encrypt(const AES_String& in) const {
        AES_String out(in.size_in_bytes());

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

        return out;
    }

    AES_String
    AES128::decrypt(const AES_String& in) const {
        AES_String out(in.size_in_bytes());

        for (int i = 0; i < in.block_count; ++i) {
            __m128i data_block = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&in.blocks[i]));
            // Last key used in encryption step:
            data_block = _mm_xor_si128(data_block, key_schedule[10]);

            // Precomputed keys are: [19 to 11]
        #ifdef SPHINX_PRECOMPUTE_REVERSE_KEYS
            for (int i = 19; i >= 11; --i) {
                data_block = _mm_aesdec_si128(data_block, key_schedule[i]);
            }
        #else
            // Otherwise we go from: [9 to 0]
            for (int i = 9; i > 0; --i) {
                data_block = _mm_aesdec_si128(data_block, _mm_aesimc_si128(key_schedule[i]));
            }
        #endif

            data_block = _mm_aesdeclast_si128(data_block, key_schedule[0]);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(&out.blocks[i]), data_block);
        }

        return out;
    }

} // sphinx


// Implements:
#include <sphinx/AES.hpp>

// Dependencies (3rd_party):
#include <wmmintrin.h>


namespace sphinx {

    /*
        Assist in generating a new key for the key schedule from an existing key.
    */
    static inline __m128i
    aes128_assist(__m128i temp1, __m128i temp2) {
        __m128i temp3;

        temp2 = _mm_shuffle_epi32(temp2 ,0xff);
        temp3 = _mm_slli_si128(temp1, 0x4);
        temp1 = _mm_xor_si128(temp1, temp3);
        temp3 = _mm_slli_si128(temp3, 0x4);
        temp1 = _mm_xor_si128(temp1, temp3);
        temp3 = _mm_slli_si128(temp3, 0x4);
        temp1 = _mm_xor_si128(temp1, temp3);
        temp1 = _mm_xor_si128(temp1, temp2);

        return temp1;
    }


    void
    aes128_expand_key(const uint8_t *userkey, uint8_t *key) {
        __m128i temp1, temp2;
        __m128i *Key_Schedule = (__m128i*)key;
        temp1 = _mm_loadu_si128((__m128i*)userkey);

        // Generate key rounds:
        Key_Schedule[0] = temp1;

        temp2 = _mm_aeskeygenassist_si128(temp1 ,0x1);
        temp1 = aes128_assist(temp1, temp2);
        Key_Schedule[1] = temp1;

        temp2 = _mm_aeskeygenassist_si128(temp1,0x2);
        temp1 = aes128_assist(temp1, temp2);
        Key_Schedule[2] = temp1;

        temp2 = _mm_aeskeygenassist_si128(temp1,0x4);
        temp1 = aes128_assist(temp1, temp2);
        Key_Schedule[3] = temp1;

        temp2 = _mm_aeskeygenassist_si128(temp1,0x8);
        temp1 = aes128_assist(temp1, temp2);
        Key_Schedule[4] = temp1;

        temp2 = _mm_aeskeygenassist_si128(temp1,0x10);
        temp1 = aes128_assist(temp1, temp2);
        Key_Schedule[5] = temp1;

        temp2 = _mm_aeskeygenassist_si128(temp1,0x20);
        temp1 = aes128_assist(temp1, temp2);
        Key_Schedule[6] = temp1;

        temp2 = _mm_aeskeygenassist_si128(temp1,0x40);
        temp1 = aes128_assist(temp1, temp2);
        Key_Schedule[7] = temp1;

        temp2 = _mm_aeskeygenassist_si128(temp1,0x80);
        temp1 = aes128_assist(temp1, temp2);
        Key_Schedule[8] = temp1;

        temp2 = _mm_aeskeygenassist_si128(temp1,0x1b);
        temp1 = aes128_assist(temp1, temp2);
        Key_Schedule[9] = temp1;

        temp2 = _mm_aeskeygenassist_si128(temp1,0x36);
        temp1 = aes128_assist(temp1, temp2);
        Key_Schedule[10] = temp1;
    }

}

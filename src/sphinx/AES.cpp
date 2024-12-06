
// Implements:
#include <sphinx/AES.hpp>

namespace sphinx {

    /*
        Helpers for key expansion rounds:
    */
    #define AES128_KEYROUND(i, rcon) \
        key = ctx->key_schedule[i - 1]; \
        gen = _mm_aeskeygenassist_si128(key, rcon); \
        gen = _mm_shuffle_epi32(gen, 255); \
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4)); \
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4)); \
        key = _mm_xor_si128(key, _mm_slli_si128(key, 4)); \
        ctx->key_schedule[i] = _mm_xor_si128(key, gen)


    void
    aes128_init(AES128_Context* ctx, AES_User_Key user_key) {
        __m128i key, gen;
        ctx->key_schedule[0] = _mm_loadu_si128(reinterpret_cast<const __m128i *>(user_key.block.data));

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

        ctx->key_schedule[11] = _mm_aesimc_si128(ctx->key_schedule[9]);
        ctx->key_schedule[12] = _mm_aesimc_si128(ctx->key_schedule[8]);
        ctx->key_schedule[13] = _mm_aesimc_si128(ctx->key_schedule[7]);
        ctx->key_schedule[14] = _mm_aesimc_si128(ctx->key_schedule[6]);
        ctx->key_schedule[15] = _mm_aesimc_si128(ctx->key_schedule[5]);
        ctx->key_schedule[16] = _mm_aesimc_si128(ctx->key_schedule[4]);
        ctx->key_schedule[17] = _mm_aesimc_si128(ctx->key_schedule[3]);
        ctx->key_schedule[18] = _mm_aesimc_si128(ctx->key_schedule[2]);
        ctx->key_schedule[19] = _mm_aesimc_si128(ctx->key_schedule[1]);
    }

    void
    aes128_encrypt(AES128_Context* ctx, void *pt, const void *ct) {
        __m128i m = _mm_loadu_si128(reinterpret_cast<const __m128i *>(ct));

        m = _mm_xor_si128(m, ctx->key_schedule[0]);
        for (int i = 1; i < 10; ++i) {
            m = _mm_aesenc_si128(m, ctx->key_schedule[i]);
        }
        m = _mm_aesenclast_si128(m, ctx->key_schedule[10]);

        _mm_storeu_si128(reinterpret_cast<__m128i *>(pt), m);
    }

    void
    aes128_decrypt(AES128_Context* ctx, void *ct, const void *pt) {
        __m128i m = _mm_loadu_si128(reinterpret_cast<const __m128i *>(pt));

        m = _mm_xor_si128(m, ctx->key_schedule[10]);
        for (int i = 11; i < 20; ++i) {
            m = _mm_aesdec_si128(m, ctx->key_schedule[i]);
        }
        m = _mm_aesdeclast_si128(m, ctx->key_schedule[0]);

        _mm_storeu_si128(reinterpret_cast<__m128i *>(ct), m);
    }
}

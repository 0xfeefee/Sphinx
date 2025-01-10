
#pragma once
/*
# AES encryption

    We are using Intel's AES-NI. Code in this module is based on the official guide from Intel!
    For our purposes we only care about AES128, ECB - which should be perfectly adequate
    for small strings such as passwords, since those are not big enough to be easily defeated
    with repeating pattern analysis. Though we could do a better key derivation strategy, right
    now we use a key as-is if it's 16 characters long, otherwise we repeat the key until it's
    16 characters long.

    Config:
    - { SPHINX_PRECOMPUTE_REVERSE_KEY_SCHEDULE }: if defined will cause the reverse key schedule
    to be generated together with the key schedule.

    Here we also define { m128 } to avoid including { <wmmintrin.h> } in this header.
*/
#define SPHINX_PRECOMPUTE_REVERSE_KEY_SCHEDULE 1
typedef long long m128 __attribute__((__vector_size__(16), __aligned__(16)));

namespace sphinx {

    /*
        We are using ECB, meaning we are processing in blocks, each block is 16 bytes for AES128.
    */
    static constexpr int BLOCK_SIZE = 16;
#if SPHINX_PRECOMPUTE_REVERSE_KEY_SCHEDULE
    static constexpr int KEY_ROUNDS = 20;
#else
    static constexpr int KEY_ROUNDS = 11;
#endif

    /*
        A single block abstracted out for convenience, we can remove this later and just do
        an array of bytes which is a multiple of { BLOCK_SIZE }, in practice this is the same
        though - since we are storing these blocks contiguously.
    */
    struct AES_Block {
        u8 data[BLOCK_SIZE] = {};

        [[nodiscard]]
        u8&
        operator[](int index);
    };

    /*
        User key is essentially a master password, it's required for this key to have the size
        of 1 AES block, i.e: 16 bytes in this case. If user supplies a key shorter than 16 bytes
        we will wrap it around until it's a 16 byte key.

        For the ease of use we allow it to be constructed from a string implicitly.
    */
    struct AES_User_Key {
        AES_Block block;

        AES_User_Key(cstr_t key_string);
    };

    /*
        Automatically split the string into blocks, that are encrypted/decrypted independently (ECB).
        Since every { AES_Block } is set to zero per default, we do not need to explicitly add padding.
    */
    struct AES_String {
        int        block_count;
        AES_Block* blocks;

        AES_String(cstr_t text);
        AES_String(const std::string& string);
        AES_String(int size_in_bytes);
        AES_String(const AES_String& other);

        ~AES_String();

        [[nodiscard]]
        int
        size_in_bytes() const;

        [[nodiscard]]
        u8*
        byte_ptr() const;

        [[nodiscard]]
        std::string
        to_string();
    };

    /*
        Just a simple class which allows us to encrypt/decrypt with a master key - { AES_User_Key }
    */
    class AES128 {
    private:
        m128 key_schedule[KEY_ROUNDS];

    public:
        AES128();
        AES128(const AES_User_Key& user_key);

        ~AES128();

        [[nodiscard]]
        AES_String
        encrypt(const AES_String& plain_text) const;

        [[nodiscard]]
        AES_String
        decrypt(const AES_String& cipher_text) const;
    };

} // sphinx

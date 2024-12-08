/*
# AES encryption

    We are using Intel's AES-NI instruction set. This code is based on the official guide from Intel!
    For our purposes we only care about AES128, ECB - which should be perfectly adequate for small
    strings such as passwords, since those are not big enough to be easily defeated with repeating
    pattern analysis.

    Config:
    - { SPHINX_PRECOMPUTE_REVERSE_KEY_SCHEDULE }: if defined will cause the reverse key schedule to be
      generated together with the key schedule.

    Here we also define { m128 } to avoid including { <wmmintrin.h> }.
*/
typedef long long m128 __attribute__((__vector_size__(16), __aligned__(16)));

namespace sphinx {

    /*
        We are using ECB, meaning we are processing in blocks, each block is 16 bytes for AES128.
    */
    static constexpr int BLOCK_SIZE = 16;

    static inline int
    byte_count_to_blocks(int byte_count) {
        return (byte_count + BLOCK_SIZE - 1) / BLOCK_SIZE;
    }

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
            cout << "]";
        #endif
        }
    };

    /*
        User key is essentially a master password, it's required for this key to have the size of 1
        AES block, i.e: 16 bytes in this case. If user supplies a key shorter than 16 bytes we will
        wrap it around until it's a 16 byte key.

        For the ease of use we allow it to be constructed from a string implicitly.
        This key is used to generate the key schedule for AES rounds.
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

        AES_String(cstr_t text) {
            int length = cstr_length(text);
            EXPECT(length > 0);

            block_count = byte_count_to_blocks(length);
            blocks      = new AES_Block[block_count];

            EXPECT(blocks);
            memcpy(blocks, text, length);
        }

        AES_String(int size_in_bytes) {
            EXPECT(size_in_bytes > 0);

            block_count = byte_count_to_blocks(size_in_bytes);
            blocks      = new AES_Block[block_count];

            EXPECT(blocks);
        }

        AES_String(const AES_String& other) {
            block_count = other.block_count;
            blocks      = new AES_Block[block_count];

            EXPECT(blocks);
            memcpy(blocks, other.blocks, block_count*BLOCK_SIZE);
        }

        void
        print() {
        #ifdef PROJECT_BUILD_DEBUG
            for (int i = 0; i < block_count; ++i) {
                blocks[i].print();
            }
            cout << endl;
        #endif
        }
    };

    /*
        Just a simple class which allows us to encrypt/decrypt with a master key - { Use_Key }
    */
    class AES128 {
    private:
    #ifdef SPHINX_PRECOMPUTE_INVERSE_KEY_SCHEDULE
        m128 key_schedule[20];
    #else
        m128 key_schedule[11];
    #endif
    public:
        AES128(AES_User_Key user_key);
        ~AES128();

        void
        encrypt(void* in, void* out);

        void
        encrypt(AES_String& in, AES_String& out);

        void
        decrypt(void* in, void* out);

        void
        decrypt(AES_String& in, AES_String& out);
    };

}

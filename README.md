# Sphinx (v0.6.0)
A simple GUI tool for storing encrypted messages in `PNG` images.

## Idea
Store AES encrypted messages in `.PNG` images.

- For AES encryption we will use the
**Intel's AES-NI (Advanced Encryption Standard New Instructions)**
or [OpenSSL](https://github.com/openssl/openssl).
- For UI we will use [Dear ImGui](https://github.com/ocornut/imgui).

## Structure
- Time tracking: [docs/Time](./docs/Time.md)
- Docs

## Tested
- [X] Windows + Clang

## References
- [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ig_expand=243): Good reference for all of Intel's intrinsics.

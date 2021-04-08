#pragma once

#include "ssl_helpers_defines.h"

#include "aes256.h"

#include "sha512.h"

#include <memory>

namespace ssl_helpers {
namespace impl {

    template <class aes_context>
    class aes_stream_sm
    {
        enum class state : uint8_t
        {
            finalized = 0,
            initialized,
            processing
        };

    public:
        aes_stream_sm() = default;

        std::string start(const std::string& key, const std::string& add = {})
        {
            SSL_HELPERS_ASSERT(!key.empty(), "Key required");

            SSL_HELPERS_ASSERT(_state == state::finalized, "Invalid state");

            auto h_key = impl::sha512::hash(key);

            SSL_HELPERS_ASSERT(h_key.data_size() > 32);

            const char* ph_key = h_key.data();
            _context.init(create_from_string<gcm_key_type>(ph_key, h_key.data_size()),
                          create_from_string<gcm_iv_type>(ph_key + 32, h_key.data_size() - 32));
            if (!add.empty())
                _context.set_add(add.data(), add.size());

            return add;
        }

        std::string process(const std::string& plain_chunk)
        {
            SSL_HELPERS_ASSERT(_state == state::finalized || _state == state::processing, "Invalid state");

            _state = state::processing;

            std::vector<char> result(plain_chunk.size());
            auto sz = _context.process(plain_chunk.data(), plain_chunk.size(), result.data());
            result.resize(sz);

            return { result.data(), result.size() };
        }

        std::string finalize(const std::string& input_tag = {})
        {
            SSL_HELPERS_ASSERT(_state == state::processing, "Invalid state");

            _state = state::finalized;

            gcm_tag_type tag;
            if (!input_tag.empty())
                tag = create_from_string<gcm_tag_type>(input_tag.data(), input_tag.size());
            _context.finalize(tag);

            return to_string(tag);
        }

    private:
        aes_context _context;
        state _state = state::finalized;
    };

    class aes_encryption_stream_impl
    {
    public:
        aes_encryption_stream_impl(const std::string& key, const std::string& add);

        std::string start(const std::string& key, const std::string& add);
        std::string encrypt(const std::string& plain_chunk);
        std::string finalize();

        size_t last_add_size() const;
        size_t last_ecryption_size() const;
        size_t tag_size() const;

    private:
        aes_stream_sm<aes_stream_encryptor> _sm;
        std::string _key_shadow;
        std::string _add;
        size_t _last_add_size = 0;
        size_t _last_ecryption_size = 0;
    };

    class aes_decryption_stream_impl
    {
    public:
        aes_decryption_stream_impl(const std::string& key, const std::string& add);

        void start(const std::string& key, const std::string& add);
        std::string decrypt(const std::string& cipher_chunk);
        void finalize(const std::string& tag);

    private:
        aes_stream_sm<aes_stream_decryptor> _sm;
        std::string _key_shadow;
        std::string _add;
    };

} // namespace impl
} // namespace ssl_helpers

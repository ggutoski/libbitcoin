/*
 * Copyright (c) 2011-2014 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <secp256k1.h>
#include <bitcoin/bitcoin/utility/assert.hpp>
#include <bitcoin/bitcoin/utility/ec_keys.hpp>
#include <bitcoin/bitcoin/utility/hash.hpp>

namespace libbitcoin {

/**
 * Internal class to start and stop the secp256k1 library.
 */
class init_singleton
{
public:
    init_singleton()
      : init_done_(false)
    {}
    ~init_singleton()
    {
        if (init_done_)
            secp256k1_stop();
    }
    void init()
    {
        if (init_done_)
            return;

        // It may be more efficient on a per-call basis to pass these
        // individually via init(), however since we are caching the init
        // we must set both flags here.
        secp256k1_start(SECP256K1_START_VERIFY | SECP256K1_START_SIGN);

        init_done_ = true;
    }

private:
    bool init_done_;
};

static init_singleton init;

ec_point secret_to_public_key(const ec_secret& secret,
    bool compressed)
{
    init.init();
    size_t public_key_size = ec_uncompressed_size;
    if (compressed)
        public_key_size = ec_compressed_size;

    ec_point out(public_key_size);
    int out_size;
    if (!secp256k1_ec_pubkey_create(out.data(), &out_size, secret.data(),
            compressed))
        return ec_point();
    BITCOIN_ASSERT(public_key_size == static_cast<size_t>(out_size));
    return out;
}

bool verify_public_key(const ec_point& public_key)
{
    init.init();
    return secp256k1_ec_pubkey_verify(public_key.data(), public_key.size())
        == 1;
}

bool verify_public_key_fast(const ec_point& public_key)
{
    if (public_key.size() == ec_compressed_size)
        return public_key[0] == 0x02 || public_key[0] == 0x03;
    if (public_key.size() == ec_uncompressed_size)
        return public_key[0] == 0x04;
    return false;
}

bool verify_private_key(const ec_secret& private_key)
{
    init.init();
    return secp256k1_ec_seckey_verify(private_key.data()) == 1;
}

BC_API ec_secret create_nonce(ec_secret secret, hash_digest hash)
{
    std::reverse(hash.begin(), hash.end());
    init.init();

    hash_digest K
    {{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }};
    hash_digest V
    {{
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
    }};

    K = hmac_sha256_hash(V + byte_array<1>{{0x00}} + secret + hash, K);
    V = hmac_sha256_hash(V, K);
    K = hmac_sha256_hash(V + byte_array<1>{{0x01}} + secret + hash, K);
    V = hmac_sha256_hash(V, K);

    while (true)
    {
        V = hmac_sha256_hash(V, K);

        if (verify_private_key(V))
            return V;

        K = hmac_sha256_hash(V + byte_array<1>{{0x00}}, K);
        V = hmac_sha256_hash(V, K);
    }
}

data_chunk sign(ec_secret secret, hash_digest hash, ec_secret nonce)
{
    std::reverse(hash.begin(), hash.end());
    init.init();

    int out_size = 72;
    data_chunk signature(out_size);
    if (secp256k1_ecdsa_sign(hash.data(), hash.size(),
        signature.data(), &out_size, secret.data(), nonce.data()) == 1)
    {
        signature.resize(out_size);
        return signature;
    }

    // Error case:
    return data_chunk();
}

bool verify_signature(const ec_point& public_key, hash_digest hash,
    const data_chunk& signature)
{
    std::reverse(hash.begin(), hash.end());
    init.init();
    return 1 == secp256k1_ecdsa_verify(
        hash.data(), hash.size(),
        signature.data(), signature.size(),
        public_key.data(), public_key.size()
    );
}

bool ec_add(ec_point& a, const ec_secret& b)
{
    init.init();
    return secp256k1_ec_pubkey_tweak_add(a.data(), a.size(), b.data()) == 1;
}

bool ec_add(ec_secret& a, const ec_secret& b)
{
    init.init();
    return secp256k1_ec_privkey_tweak_add(a.data(), b.data()) == 1;
}

bool ec_multiply(ec_point& a, const ec_secret& b)
{
    init.init();
    return secp256k1_ec_pubkey_tweak_mul(a.data(), a.size(), b.data()) == 1;
}

bool ec_multiply(ec_secret& a, const ec_secret& b)
{
    init.init();
    return secp256k1_ec_privkey_tweak_mul(a.data(), b.data()) == 1;
}

} // namespace libbitcoin


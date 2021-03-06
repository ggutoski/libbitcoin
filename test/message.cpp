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
#include <boost/test/unit_test.hpp>
#include <bitcoin/bitcoin.hpp>

#include <iostream>
using namespace bc;

// echo -n "Satoshi" | sha256sum
static const ec_secret secret
{{
    0x00, 0x26, 0x88, 0xcc, 0x35, 0x0a, 0x53, 0x33,
    0xa8, 0x7f, 0xa6, 0x22, 0xea, 0xce, 0xc6, 0x26,
    0xc3, 0xd1, 0xc0, 0xeb, 0xf9, 0xf3, 0x79, 0x3d,
    0xe3, 0x88, 0x5f, 0xa2, 0x54, 0xd7, 0xe3, 0x93
}};

BOOST_AUTO_TEST_SUITE(message_tests)

BOOST_AUTO_TEST_CASE(message_sign_uncompressed_test)
{
    static const auto message = to_data_chunk(std::string("Bitcoin"));
    payment_address address(payment_address::pubkey_version,
        bitcoin_short_hash(secret_to_public_key(secret, false)));

    auto signature = sign_message(message, secret, false);
    BOOST_REQUIRE(verify_message(message, address, signature));
}

BOOST_AUTO_TEST_CASE(message_sign_compressed_test)
{
    static const auto message = to_data_chunk(std::string("Nakomoto"));
    payment_address address(payment_address::pubkey_version,
        bitcoin_short_hash(secret_to_public_key(secret, true)));

    auto signature = sign_message(message, secret, true);
    BOOST_REQUIRE(verify_message(message, address, signature));
}

BOOST_AUTO_TEST_CASE(message_electrum_test)
{
    // Generated using Electrum:
    message_signature signature =
    {{
        0x1f,
        0x14, 0x29, 0xdd, 0xc5, 0xe0, 0x38, 0x88, 0x41,
        0x10, 0x65, 0xe4, 0xb3, 0x6e, 0xec, 0x7d, 0xe4,
        0x90, 0x1d, 0x58, 0x0d, 0x51, 0xe6, 0x20, 0x97,
        0x98, 0xb9, 0xc0, 0x6f, 0xdd, 0x39, 0x46, 0x1a,
        0x48, 0x84, 0x67, 0x9f, 0x35, 0xd1, 0xe8, 0xd7,
        0x32, 0x1f, 0xe0, 0x1f, 0x34, 0x01, 0xed, 0x91,
        0x67, 0x32, 0x38, 0x3f, 0x6b, 0x5f, 0x8a, 0x68,
        0x8e, 0xa9, 0xae, 0x43, 0x21, 0xfb, 0xf4, 0xae
    }};
    auto message = to_data_chunk(std::string("Nakomoto"));
    payment_address address("1PeChFbhxDD9NLbU21DfD55aQBC4ZTR3tE");

    BOOST_REQUIRE(verify_message(message, address, signature));
}

BOOST_AUTO_TEST_SUITE_END()

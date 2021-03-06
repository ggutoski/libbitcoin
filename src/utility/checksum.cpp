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
#include <bitcoin/bitcoin/format.hpp>
#include <bitcoin/bitcoin/utility/checksum.hpp>
#include <bitcoin/bitcoin/utility/hash.hpp>

namespace libbitcoin {

void append_checksum(data_chunk& data)
{
    uint32_t checksum = bitcoin_checksum(data);
    extend_data(data, to_little_endian(checksum));
}

uint32_t bitcoin_checksum(const data_chunk& chunk)
{
    hash_digest hash = bitcoin_hash(chunk);
    return from_little_endian_unsafe<uint32_t>(hash.rbegin());
}

bool verify_checksum(const data_chunk& data)
{
    const size_t checksum_length = 4;

    // guard against insufficient data length
    if (data.size() < checksum_length)
        return false;

    data_chunk body(data.begin(), data.end() - checksum_length);
    auto checksum = from_little_endian_unsafe<uint32_t>(data.end() - checksum_length);
    return bitcoin_checksum(body) == checksum;
}

} // namespace libbitcoin


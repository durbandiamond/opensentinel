/*
 * Copyright (c) 2017-2018 Durban & Diamond, LLC.
 *
 * This file is part of Open Sentinel.
 *
 * Open Sentinel is free software: you can redistribute it and/or modify
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

#pragma once

#include <algorithm>
#include <cstdint>

#define ASIO_STANDALONE 1

#include <asio.hpp>

namespace opensentinel {

    class ipv4_header
    {
        public:
        
            /**
             * Constructor
             */
            ipv4_header()
            {
                std::fill(data_, data_ + sizeof(data_), 0);
            }

            /**
             * The version.
             */
            std::uint8_t version() const
            {
                return (data_[0] >> 4) & 0xF;
            }
        
            /**
             * The header length.
             */
            std::uint16_t header_length() const
            {
                return (data_[0] & 0xF) * 4;
            }
        
            /**
             * the type of service.
             */
            std::uint8_t type_of_service() const
            {
                return data_[1];
            }
        
            /**
             * The total length.
             */
            std::uint16_t total_length() const
            {
                return decode(2, 3);
            }
        
            /**
             * The identification.
             */
            std::uint16_t identification() const
            {
                return decode(4, 5);
            }
        
            /**
             * If false we are not fragmenting.
             */
            bool dont_fragment() const
            {
                return (data_[6] & 0x40) != 0;
            }
        
            /**
             * If true there are more fragments.
             */
            bool more_fragments() const
            {
                return (data_[6] & 0x20) != 0;
            }
        
            /**
             * The current fragment offset.
             */
            std::uint16_t fragment_offset() const
            {
                return decode(6, 7) & 0x1FFF;
            }
        
            /**
             * The TTL (time to live).
             */
            std::uint32_t time_to_live() const
            {
                return data_[8];
            }
        
            /**
             * The protocol.
             */
            std::uint8_t protocol() const
            {
                return data_[9];
            }
        
            /**
             * The header checksum.
             */
            std::uint16_t header_checksum() const
            {
                return decode(10, 11);
            }

            /**
             * The source asio::ip::address_v4.
             */
            asio::ip::address_v4 source_address() const
            {
                asio::ip::address_v4::bytes_type bytes =
                {
                    { data_[12], data_[13], data_[14], data_[15] }
                };
                
                return asio::ip::address_v4(bytes);
            }

            /**
             * The Destination asio::ip::address_v4.
             */
            asio::ip::address_v4 destination_address() const
            {
                asio::ip::address_v4::bytes_type bytes =
                {
                    { data_[16], data_[17], data_[18], data_[19] }
                };
                
                return asio::ip::address_v4(bytes);
            }

            /**
             * operator >>
             */
            friend std::istream & operator >> (
                std::istream & is, ipv4_header & header
                )
            {
                is.read(reinterpret_cast<char *>(header.data_), 20);
                
                if (header.version() != 4)
                {
                    is.setstate(std::ios::failbit);
                }
                
                std::streamsize options_length = header.header_length() - 20;
                
                if (options_length < 0 || options_length > 40)
                {
                    is.setstate(std::ios::failbit);
                }
                else
                {
                    is.read(
                        reinterpret_cast<char *>(header.data_) + 20,
                        options_length
                    );
                }
                
                return is;
            }

        private:
        
            /**
             * Decodes
             * @param a The a.
             * @param b The b.
             */
            std::uint16_t decode(
                const std::int32_t & a, const std::int32_t & b
                ) const
            {
                return (data_[a] << 8) + data_[b];
            }

        protected:
        
            /**
             * The data.
             */
            std::uint8_t data_[60];
    };

} // namespace opensentinel

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

#include <cstdint>
#include <istream>
#include <ostream>
#include <algorithm>

#define ASIO_STANDALONE 1

#include <asio.hpp>

namespace opensentinel {

    /**
     * Implements ICMP related functionality.
     */
    class icmp
    {
        public:

            /**
             * Implements an ICMP header for both IPv4 and IPv6.
             */
            class header
            {
                public:
            
                    /**
                     * The types.
                     */
                    typedef enum type_s
                    {
                        type_echo_reply = 0,
                        type_destination_unreachable = 3,
                        type_source_quench = 4,
                        type_redirect = 5,
                        type_echo_request = 8,
                        type_time_exceeded = 11,
                        type_parameter_problem = 12,
                        type_timestamp_request = 13,
                        type_timestamp_reply = 14,
                        type_info_request = 15,
                        type_info_reply = 16,
                        type_address_request = 17,
                        type_address_reply = 18
                    } type_t;

                    /**
                     * Constructor
                     */
                    header()
                    {
                        std::fill(data_, data_ + sizeof(data_), 0);
                    }

                    /**
                     * The type.
                     */
                    std::uint8_t type() const
                    {
                        return data_[0];
                    }
                
                    /**
                     * The type (string).
                     */
                    const std::string type_string() const
                    {
                        std::string ret;
                        
                        switch (type())
                        {
                            case type_echo_reply:
                            {
                                ret = "type_echo_reply";
                            }
                            break;
                            case type_destination_unreachable:
                            {
                                ret = "type_destination_unreachable";
                            }
                            break;
                            case type_source_quench:
                            {
                                ret = "type_source_quench";
                            }
                            break;
                            case type_redirect:
                            {
                                ret = "type_redirect";
                            }
                            break;
                            case type_echo_request:
                            {
                                ret = "type_echo_request";
                            }
                            break;
                            case type_time_exceeded:
                            {
                                ret = "type_time_exceeded";
                            }
                            break;
                            case type_parameter_problem:
                            {
                                ret = "type_parameter_problem";
                            }
                            break;
                            case type_timestamp_request:
                            {
                                ret = "type_timestamp_request";
                            }
                            break;
                            case type_timestamp_reply:
                            {
                                ret = "type_timestamp_reply";
                            }
                            break;
                            case type_info_request:
                            {
                                ret = "type_info_request";
                            }
                            break;
                            case type_info_reply:
                            {
                                ret = "type_info_reply";
                            }
                            break;
                            case type_address_request:
                            {
                                ret = "type_address_request";
                            }
                            break;
                            case type_address_reply:
                            {
                                ret = "type_address_reply";
                            }
                            break;
                            default:
                            break;
                        }
                        
                        return ret;
                    }
                
                    /**
                     * The code.
                     */
                    std::uint8_t code() const
                    {
                        return data_[1];
                    }
                
                    /**
                     * The checksum.
                     */
                    std::uint16_t checksum() const
                    {
                        return decode(2, 3);
                    }
        
                    /**
                     * The identifier.
                     */
                    std::uint16_t identifier() const
                    {
                        return decode(4, 5);
                    }
                
                    /**
                     * The sequence number.
                     */
                    std::uint16_t sequence_number() const
                    {
                        return decode(6, 7);
                    }

                    /**
                     * Sets the type.
                     * @param val The value
                     */
                    void set_type(const std::uint8_t & val)
                    {
                        data_[0] = val;
                    }
                
                    /**
                     * Sets the code.
                     * @param val The value
                     */
                    void set_code(const std::uint8_t & val)
                    {
                        data_[1] = val;
                    }
                
                    /**
                     * Sets the checksum.
                     * @param val The value
                     */
                    void set_checksum(const std::uint16_t & val)
                    {
                        encode(2, 3, val);
                    }
                
                    /**
                     * Sets the identifier.
                     * @param val The value
                     */
                    void set_identifier(const std::uint16_t & val)
                    {
                        encode(4, 5, val);
                    }
                
                    /**
                     * Sets the sequence number.
                     * @param val The value
                     */
                    void set_sequence_number(const std::uint16_t & val)
                    {
                        encode(6, 7, val);
                    }

                    /**
                     * operator >>
                     */
                    friend std::istream & operator >> (
                        std::istream & is, header & val
                        )
                    {
                        return is.read(reinterpret_cast<char *> (val.data_), 8);
                    }

                    /**
                     * operator <<
                     */
                    friend std::ostream & operator << (
                        std::ostream & os, const header & val
                        )
                    {
                        return
                            os.write(reinterpret_cast<const char *> (val.data_),
                            8)
                        ;
                    }

                private:
                
                    /**
                     * Encodes
                     * @param a The a.
                     * @param b The b.
                     */
                    std::uint16_t decode(
                        const std::int32_t & a, const std::int32_t & b
                        ) const
                    {
                        return (data_[a] << 8) + data_[b];
                    }

                    /**
                     * Encodes
                     * @param a The a.
                     * @param b The b.
                     * @param n The n.
                     */
                    void encode(
                        const std::int32_t & a, const std::int32_t & b,
                        const std::uint16_t & n
                        )
                    {
                        data_[a] = static_cast<std::uint8_t> (n >> 8);
                        data_[b] = static_cast<std::uint8_t> (n & 0xFF);
                    }
                
                    /**
                     * Computes the checksum.
                     * @param val The header.
                     * @param begin The T.
                     * @param end The T.
                     */
                    template <typename T>
                    void compute_checksum(header & val, T begin, T end)
                    {
                        std::uint32_t sum =
                            (val.type() << 8) + val.code() + val.identifier() +
                            val.sequence_number()
                        ;

                        T it = begin;
                    
                        while (it != end)
                        {
                            sum +=
                                (static_cast<std::uint8_t> (*it++) << 8)
                            ;
                            
                            if (it != end)
                            {
                                sum += static_cast<std::uint8_t> (*it++);
                            }
                        }

                        sum = (sum >> 16) + (sum & 0xFFFF);
                        sum += (sum >> 16);
                        
                        val.set_checksum(static_cast<std::uint16_t> (~sum));
                    }
                
                private:
                
                    // ...
                
                protected:
                
                    /**
                     * The data.
                     */
                    std::uint8_t data_[8];
            };
        
        private:
        
            // ...
        
        protected:
        
            // ...
    };
    
} // namespace opensentinel

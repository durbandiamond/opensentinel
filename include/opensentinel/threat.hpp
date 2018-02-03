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
#include <string>
#include <vector>

#define ASIO_STANDALONE 1

#include <asio.hpp>

namespace opensentinel {

    class threat
    {
        public:
        
            /**
             * The protocol.
             */
            typedef enum protocol_s
            {
                protocol_none,
                protocol_tcp,
                protocol_udp,
                protocol_icmp,
            } protocol_t;
        
            /**
             * The level.
             */
            typedef enum level_s
            {
                level_0,
                level_1,
                level_2,
                level_3,
                level_4,
                level_5
            } level_t;
        
            /**
             * Constructor
             * @param addr The address.
             * @param port The port.
             * @param buf The buffer.
             * @param len The length.
             */
            explicit threat(
                const protocol_t & proto,
                const asio::ip::address & addr, const std::uint16_t & port,
                const char * buf, const std::size_t & len
            );
        
            /**
             * The asio::ip::address.
             */
            const asio::ip::address & address() const;
        
            /**
             * The port.
             */
            const std::uint16_t & port() const;
        
            /**
             * The buffer.
             */
            std::vector<char> & buffer();
        
            /**
             * The buffer.
             */
            const std::vector<char> & buffer() const;
        
            /**
             * Sets the level.
             * @param val The value.
             */
            void set_level(const level_t & val);
            
            /**
             * The level.
             */
            const level_t & level() const;
        
            /**
             * The level (string).
             */
            const std::string level_string() const;
        
            /**
             * The protocol.
             */
            const protocol_t & protocol() const;
        
            /**
             * The protocol (string).
             */
            const std::string protocol_string() const;
        
            /**
             * Prints
             */
            const void print() const;
        
        private:
        
            /**
             * The asio::ip::address.
             */
            asio::ip::address m_address;
        
            /**
             * The port.
             */
            std::uint16_t m_port = 0;
        
            /**
             * The buffer.
             */
            std::vector<char> m_buffer;
        
            /**
             * The level.
             */
            level_t m_level = level_0;
        
            /**
             * The protocol.
             */
            protocol_t m_protocol = protocol_none;
        
        protected:
        
            // ...
    };
}

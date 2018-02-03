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

#include <opensentinel/logger.hpp>
#include <opensentinel/threat.hpp>

using namespace opensentinel;

threat::threat(
    const protocol_t & proto, const asio::ip::address & addr,
    const std::uint16_t & port, const char * buf, const std::size_t & len
    )
    : m_address(addr)
    , m_port(port)
    , m_buffer(buf, buf + len)
    , m_level(level_0)
    , m_protocol(proto)
{
    // ...
}

const asio::ip::address & threat::address() const
{
    return m_address;
}

const std::uint16_t & threat::port() const
{
    return m_port;
}

std::vector<char> & threat::buffer()
{
    return m_buffer;
}

const std::vector<char> & threat::buffer() const
{
    return m_buffer;
}

void threat::set_level(const level_t & val)
{
    m_level = val;
}

const threat::level_t & threat::level() const
{
    return m_level;
}

const std::string threat::level_string() const
{
    std::string ret;
    
    switch (m_level)
    {
        case level_0:
        {
            ret = "LEVEL_0";
        }
        break;
        case level_1:
        {
            ret = "LEVEL_1";
        }
        break;
        case level_2:
        {
            ret = "LEVEL_2";
        }
        break;
        case level_3:
        {
            ret = "LEVEL_3";
        }
        break;
        case level_4:
        {
            ret = "LEVEL_4";
        }
        break;
        case level_5:
        {
            ret = "LEVEL_5";
        }
        break;
        default:
        break;
    }
    
    return ret;
}

const threat::protocol_t & threat::protocol() const
{
    return m_protocol;
}

const std::string threat::protocol_string() const
{
    std::string ret;
    
    switch (m_protocol)
    {
        case protocol_none:
        {
            ret = "NONE";
        }
        break;
        case protocol_tcp:
        {
            ret = "TCP";
        }
        break;
        case protocol_udp:
        {
            ret = "UDP";
        }
        break;
        case protocol_icmp:
        {
            ret = "ICMP";
        }
        break;
        default:
        break;
    }
    
    return ret;
}

const void threat::print() const
{
    /**
     * Only print a maximum of 256 bytes of the (sample) data.
     */
    const auto data = std::string(
        &m_buffer[0], std::min(static_cast<std::size_t> (256), m_buffer.size())
    );
    
    log_info(
        "Threat, endpoint = " << m_address.to_string() << ":" << m_port <<
        ", data size = " << m_buffer.size() << ", data = " << data << "."
    );
}


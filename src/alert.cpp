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

#include <sstream>

#include <opensentinel/alert.hpp>
#include <opensentinel/threat.hpp>
#include <opensentinel/utility.hpp>

using namespace opensentinel;

alert::alert(const threat & threat_data)
    : m_threat(std::make_shared<threat> (threat_data))
{
    // ...
}

const std::string alert::to_string() const
{
    std::stringstream ss;
    
    ss << m_threat->address().to_string();
    ss << ":";
    ss << m_threat->port();
    ss << ",";
    ss << m_threat->protocol_string();
    ss << ",";
    ss << m_threat->level_string();
    ss << ",";
    
    if (m_threat->buffer().size() > 0)
    {
        enum { maximum_sample_length = 1536 };
        
        if (m_threat->buffer().size() > maximum_sample_length)
        {
            m_threat->buffer().resize(maximum_sample_length);
        }
        
        /**
         * Detect certain ascii based protocols.
         */
        auto str = std::string(
            m_threat->buffer().begin(), m_threat->buffer().end()
        );

        /**
         * Check for the HTTP protocol.
         */
        if (str.find("HTTP/") != std::string::npos)
        {
            /**
             * Find out what type of HTTP request it is.
             */
            if (str.find("GET") != std::string::npos)
            {
                ss << "HTTP_GET ";
            }
            else if (str.find("POST") != std::string::npos)
            {
                ss << "HTTP_POST ";
            }
            else if (str.find("HEAD") != std::string::npos)
            {
                ss << "HTTP_HEAD ";
            }
            
            /**
             * Convert ascii packet to hexidecimal.
             */
            ss << utility::hex_string(
                m_threat->buffer().begin(), m_threat->buffer().end()
            );
        }
        else
        {
            /**
             * Convert binary packet to hexidecimal.
             */
            ss << utility::hex_string(
                m_threat->buffer().begin(), m_threat->buffer().end()
            );
        }
    }
    
    return ss.str();
}

const std::string alert::fingerprint() const
{
    std::stringstream ss;
    
    ss << m_threat->address();
    ss << ":";
    ss << m_threat->protocol();
    ss << ":";
    ss << m_threat->level();
    ss << ":";
    ss << (m_threat->buffer().size() > 0);
    
    return ss.str();
}


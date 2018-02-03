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

namespace opensentinel {

    class utility
    {
        public:
        
            /**
             * Attempts to raise the number of file descriptors to maximum.
             * @param maximum The maximum amount of file descriptors to attempt
             * to raise.
             * @ret The actual amount of file descriptors raised.
             */
            static std::uint64_t raise_file_descriptor_limit(
                const std::uint64_t & maximum
            );
        
            /**
             * Converts a binary array to a hexidecimal string.
             * @param it_begin T
             * @param it_end T
             * @param spaces bool
             */
            template<typename T>
            static inline std::string hex_string(
                const T it_begin, const T it_end, const bool & spaces = false
                )
            {
                std::string ret;
                
                static const std::int8_t g_hexmap[16] =
                {
                    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                    'a', 'b', 'c', 'd', 'e', 'f'
                };
                
                ret.reserve((it_end - it_begin) * 3);
                
                for (auto it = it_begin; it < it_end; ++it)
                {
                    auto val = static_cast<std::uint8_t> (*it);
                    
                    if (spaces && it != it_begin)
                    {
                        ret.push_back(' ');
                    }
                    
                    ret.push_back(g_hexmap[val >> 4]);
                    
                    ret.push_back(g_hexmap[val & 15]);
                }

                return ret;
            }

            /**
             * Converts a binary array to a hexidecimal string.
             * @param bytes The bytes.
             * @param spaces If true string will have spaces.
             */
            static std::string hex_string(
                const std::vector<std::uint8_t> & bytes,
                const bool & spaces = false
            );
        
        private:
        
            // ...
        
        protected:
        
            // ...
    };
}


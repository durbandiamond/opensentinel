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

#include <string>
#include <vector>

#if (defined _MSC_VER)
#include <windows.h>
#endif // _MSC_VER

namespace opensentinel {

    /**
     * Implements Windows OS utility methods.
     */
    class windows
    {
        public:
        
            /**
             * WideCharToMultiByte
             */
            static std::string wide_char_to_multi_byte(
                const std::wstring & val
                )
            {
                std::string ret;
                
#if (defined _MSC_VER)
                auto len = WideCharToMultiByte(
                    CP_UTF8, 0, val.data(), val.length(), 0, 0, 0, 0
                );
                
                if (len > 0)
                {
                    std::vector<char> buf(len);
                    
                    WideCharToMultiByte(
                        CP_UTF8, 0, val.data(), val.length(), &buf[0], len, 0, 0
                    );
            
                    ret = std::string(buf.begin(), buf.end());
                }
                
                return ret;
#else
                return std::string(val.begin(), val.end());
#endif // _MSC_VER
            }
        
            /**
             * MultiByteToWideChar
             */
            static std::wstring multi_byte_to_wide_char(
                const std::string & val
                )
            {
                std::wstring ret;
#if (defined _MSC_VER)
                auto len = MultiByteToWideChar(
                    CP_ACP, 0, val.data(), val.length(), 0, 0
                );
                
                if (len > 0)
                {
                    std::vector<wchar_t> buf(len);
                    
                    MultiByteToWideChar(
                        CP_ACP, 0, val.data(), val.length(), &buf[0], len
                    );

                    ret = std::wstring(buf.begin(), buf.end());
                }
                
                return ret;
#else
                return std::wstring(val.begin(), val.end());
#endif // _MSC_VER
            }
        
        private:
        
            // ...
        
        protected:
        
            // ...
    };
    
}; // namespace opensentinel


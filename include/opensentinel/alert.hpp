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

#include <memory>
#include <string>

namespace opensentinel {

    class threat;
    
    class alert
    {
        public:
        
            /**
             * Constructor
             * @param threat_data The threat.
             */
            explicit alert(const threat & threat_data);
        
            /**
             * The string representation.
             */
            const std::string to_string() const;
        
            /**
             * The fingerprint.
             */
            const std::string fingerprint() const;
        
            /**
             * operator ==
             */
            friend bool operator == (
                const alert & lhs, const alert & rhs
                )
            {
                return lhs.fingerprint() == rhs.fingerprint();
            }

            /**
             * operator !=
             */
            friend bool operator != (
                const alert & lhs, const alert & rhs
                )
            {
                return !(lhs == rhs);
            }
        
        private:
        
            /**
             * The threat.
             */
            std::shared_ptr<threat> m_threat;
        
        protected:
        
            // ...
    };
} // namespace opensentinel

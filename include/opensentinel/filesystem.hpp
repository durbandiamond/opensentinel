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

namespace opensentinel {

    class filesystem
    {
        public:
        
            /** 
             * File exists
             */
            static int error_already_exists;
        
            /**
             * Creates the last directory of the given path.
             * @param path The path.
             */
            static int create_path(const std::string & path);
        
            /** 
             * The user data directory.
             * @param app_name The application name.
             */
            static std::string data_path(
                const std::string & app_name = "opensentinel"
            );
        
        private:
        
            /** 
             * The user home directory.
             */
            static std::string home_path();
        
        protected:
        
            // ...
    };
    
} // opensentinel

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

#include <sys/resource.h>

#include <sstream>

#include <opensentinel/utility.hpp>

using namespace opensentinel;

std::uint64_t utility::raise_file_descriptor_limit(
    const std::uint64_t & maximum
    )
{
#if (defined _MSC_VER)
    return 2048;
#else
    struct rlimit rlimit_fd;
    
    if (getrlimit(RLIMIT_NOFILE, &rlimit_fd) != -1)
    {
        if (rlimit_fd.rlim_cur < (rlim_t)maximum)
        {
            rlimit_fd.rlim_cur = maximum;
            
            if (rlimit_fd.rlim_cur > rlimit_fd.rlim_max)
            {
                rlimit_fd.rlim_cur = rlimit_fd.rlim_max;
            }
            
            setrlimit(RLIMIT_NOFILE, &rlimit_fd);
            getrlimit(RLIMIT_NOFILE, &rlimit_fd);
        }
        
        return rlimit_fd.rlim_cur;
    }
    
    return maximum;
#endif
}

std::string utility::hex_string(
    const std::vector<std::uint8_t> & bytes,
    const bool & spaces
    )
{
    return hex_string(bytes.begin(), bytes.end(), spaces);
}

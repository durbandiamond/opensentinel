/*
 * Copyright (c) 2014-2017 BM-2cVZag8xxXdPC9BsLewmUotg6TBB8T2yTk
 *
 * This file is part of libCoin.
 *
 * libCoin is free software: you can redistribute it and/or modify
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
 
#include <cassert>
#include <cstdio>
#include <fstream>

#define ASIO_STANDALONE 1

#include <asio.hpp>

#include <opensentinel/filesystem.hpp>
#include <opensentinel/logger.hpp>

using namespace opensentinel;

#if (defined _MSC_VER)
#include <io.h>
#include "Shlobj.h"
#define ERRNO GetLastError()
static int _mkdir(const char * path)
{
    auto directory = windows::multi_byte_to_wide_char(path);
    
    return SHCreateDirectoryEx(0, directory.c_str(), 0);
}
#define CREATE_DIRECTORY(P) _mkdir(P)
#else
#include <dirent.h>
#include <sys/stat.h>
#define ERRNO errno
#define ERROR_ALREADY_EXISTS EEXIST
static int _mkdir(const char * dir)
{
    char tmp[256];
    char * p = NULL;
    size_t len;
 
    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    
    if (tmp[len - 1] == '/')
    {
        tmp[len - 1] = 0;
    }
    
    for (p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;

            mkdir(tmp, S_IRWXU);

            *p = '/';
        }
    }
    
    return mkdir(tmp, S_IRWXU);
}
#define CREATE_DIRECTORY(P) _mkdir(P)
#endif

int filesystem::error_already_exists = ERROR_ALREADY_EXISTS;

int filesystem::create_path(const std::string & path)
{
    if (CREATE_DIRECTORY(path.c_str()) == 0)
    {
        return 0;
    }
    
    return ERRNO;
}

std::string filesystem::data_path(const std::string & app_name)
{
    std::string ret;
#if (defined _MSC_VER)
    ret += getenv("APPDATA");
    ret += "\\" + app_name + "\\";
#elif (defined __APPLE__)
    ret = home_path();
    ret += "Library/";
    ret += "Application Support/";
    ret += app_name + "/";
#elif (defined __ANDROID__)
    ret = home_path() + "data/";
#else
    ret = home_path();
    ret += "." + app_name + "/data/";
#endif
    return ret;
}

std::string filesystem::home_path()
{
    std::string ret;
#if (defined __ANDROID__)
    std::string bundle_id = "com.domain.app";
    ret = "/data/data/" + bundle_id;
#else
    if (std::getenv("HOME"))
    {
        ret = std::getenv("HOME");
    }
    else if (std::getenv("USERPOFILE"))
    {
        ret = std::getenv("USERPOFILE");
    }
    else if (std::getenv("HOMEDRIVE") && std::getenv("HOMEPATH"))
    {
        ret = (
            std::string(std::getenv("HOMEDRIVE")) +
            std::string(std::getenv("HOMEPATH"))
        );
    }
    else
    {
        ret = ".";
    }
#endif // __ANDROID__
#if (defined _MSC_VER)
    return ret + "\\";
#else
    return ret + "/";
#endif
}

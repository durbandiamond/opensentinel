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

#if (defined __ANDROID__)
#include <android/log.h>
#endif
  
#if (defined _WIN32 || defined WIN32) || (defined _WIN64 || defined WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // (defined _WIN32 || defined WIN32) || (defined _WIN64 || defined WIN64)

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

namespace opensentinel {

    /**
     * Implements a logger.
     */
    class logger
    {
        public:
            
			/**
             * The severity levels.
			 */
			typedef enum severity_s
			{
				severity_none,
				severity_debug,
				severity_error,
				severity_info,
				severity_warning,
			} severity_t;
			
            /**
             * Singleton accessor.
             */
			static logger & instance()
			{
			    static logger g_logger;

			    return g_logger;
			}
            
            /**
             * operator <<
             */
            template <class T>
            logger & operator << (T const & val)
            {
                std::stringstream ss;
                
                ss << val;
                
                log(ss);
                
                ss.str(std::string());
                
                return logger::instance();
            }

            /**
             * Perform the actual logging.
             * @param val The value.
             */
			void log(std::stringstream & val)
			{
				std::lock_guard<std::mutex> lock(mutex_);
				
                const auto use_file = m_path.size() > 0;

			    if (use_file == true)
			    {
                    if (ofstream_.is_open() == false)
                    {
                        ofstream_.open(
                            m_path, std::fstream::out | std::fstream::app
                        );
                    }
                    
                    if (ofstream_.is_open() == true)
                    {
                        /**
                         * Use a maximum of 25 megabytes of file.
                         */
                        if (ofstream_.tellp() > 25 * 1000000)
                        {
                            ofstream_.close();
                            
                            ofstream_.open(m_path, std::fstream::out);
                        }
                        
                        ofstream_ << val.str() << std::endl;
                        
                        ofstream_.flush();
                    }
			    }

			    static auto use_cout = true;

			    if (use_cout == true)
			    {
#if (defined _WIN32 || defined WIN32) || (defined _WIN64 || defined WIN64)
#if defined(_UNICODE)
			        DWORD len = MultiByteToWideChar(
			            CP_ACP, 0, val.str().c_str(), -1, NULL, 0
			        );

			        std::unique_ptr<wchar_t> buf(new wchar_t[len]);

			        MultiByteToWideChar(
			            CP_ACP, 0, val.str().c_str(), -1, buf.get(), len
			        );

			        OutputDebugString(buf.get());
			        OutputDebugString(L"\n");

			        std::cerr << val.str() << std::endl;
#else
			        OutputDebugString(val.str().c_str());
			        OutputDebugString(L"\n");

			        std::cerr << val.str() << std::endl;
#endif // _UNICODE
#else // Not Windows.
#if (defined __ANDROID__)
					__android_log_print(
                        ANDROID_LOG_DEBUG, "logger", val.str().c_str()
                    );
#else
			        std::cerr << val.str() << std::endl;
#endif
#endif // defined _WIN32 || defined WIN32) || (defined _WIN64 || defined WIN64
			    }
			}
        
            /**
             * The path.
             * @param val The value.
             */
            void set_path(const std::string & val)
            {
                m_path = val;
            }
        
        private:
        
            /**
             * The path.
             */
            std::string m_path;
            
        protected:
        
            /**
             * The std::ofstream.
             */
            std::ofstream ofstream_;
            
			/**
			 * The std::mutex.
			 */
			std::mutex mutex_;
    };
    
    #define log_xx(severity, strm) \
    { \
        std::time_t time_now = std::chrono::system_clock::to_time_t( \
            std::chrono::system_clock::now()); \
        std::string time_str = std::ctime(&time_now); \
        time_str.pop_back(), time_str.pop_back(); \
        time_str.pop_back(), time_str.pop_back(); \
        time_str.pop_back(), time_str.pop_back(); \
        std::stringstream __ss; \
        switch (severity) \
        { \
            case opensentinel::logger::severity_debug: \
                __ss << time_str << " [DEBUG] - "; \
            break; \
            case opensentinel::logger::severity_error: \
                __ss << time_str << " [ERROR] - "; \
            break; \
            case opensentinel::logger::severity_info: \
                __ss << time_str << " [INFO] - "; \
            break; \
            case opensentinel::logger::severity_warning: \
                __ss << time_str << " [WARNING] - "; \
            break; \
            default: \
                __ss << std::ctime(&time_now) << " [UNKNOWN] - "; \
        } \
		__ss << __FUNCTION__ << ": "; \
        __ss << strm; \
        opensentinel::logger::instance() << __ss.str(); \
        __ss.str(std::string()); \
    } \

#define log_init(str) opensentinel::logger::instance().set_path(str)
#define log_none(strm) /** */
#if (defined NDEBUG && !defined DEBUG)
#define log_debug(strm) log_none(strm)
#else
#define log_debug(strm) log_xx(opensentinel::logger::severity_debug, strm)
#endif
#define log_error(strm) log_xx(opensentinel::logger::severity_error, strm)
#define log_info(strm) log_xx(opensentinel::logger::severity_info, strm)
#define log_warn(strm) log_xx(opensentinel::logger::severity_warning, strm)

} // namespace opensentinel


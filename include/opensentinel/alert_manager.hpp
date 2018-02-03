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

#include <chrono>
#include <ctime>
#include <map>
#include <string>
#include <thread>

#define ASIO_STANDALONE 1

#include <asio.hpp>

namespace opensentinel {

    class threat;
    
    class alert_manager
    {
        public:
        
            /**
             * Constructor
             */
            explicit alert_manager();
        
            /**
             * Starts
             */
            void start();
        
            /**
             * Stops
             */
            void stop();
            
            /**
             * :TODO:
             */
            void on_threat(const threat & threat_data);
        
        private:
        
            /**
             * The timer handler.
             */
            void on_tick();
        
            /**
             * The thread loop.
             */
            void run();
        
            /**
             * The threat alert file to execute.
             */
            std::string m_file_threat_alert;
        
        protected:
        
            /**
             * The state.
             */
            enum
            {
                state_none,
                state_starting,
                state_started,
                state_stopped,
                state_stopping,
            } state_;
        
            /**
             * The std::thread.
             */
            asio::io_service io_service_;
        
            /**
             * The std::thread.
             */
            asio::strand strand_;
        
            /**
             * The std::thread.
             */
            std::thread thread_;
        
            /**
             * The timer.
             */
            asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_;
        
            /**
             * The alert cache.
             */
            std::map<std::string, std::time_t> alert_cache_;
    };
} // namespace opensentinel

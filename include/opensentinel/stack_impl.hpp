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
#include <thread>

#define ASIO_STANDALONE 1

#include <asio.hpp>

namespace opensentinel {

    class alert_manager;
    class icmp_manager;
    class tcp_manager;
    class threat;
    class threat_manager;
    class udp_manager;
    
    class stack_impl
    {
        public:
        
            /**
             * Constructor
             */
            explicit stack_impl();
        
            /**
             * Starts
             */
            void start();
        
            /**
             * Stops
             */
            void stop();
        
            /**
             * Called when a possible threat is detected.
             * @param threat_data The threat.
             */
            void on_threat(const threat & threat_data);
        
            /**
             * The alert_manager.
             */
            std::shared_ptr<alert_manager> & get_alert_manager();
        
        private:
        
            /**
             * The network timer handler.
             */
            void on_tick_network();
        
            /**
             * The network thread loop.
             */
            void network_run();
        
            /**
             * The tcp_manager.
             */
            std::shared_ptr<tcp_manager> m_tcp_manager;
        
            /**
             * The threat_manager.
             */
            std::shared_ptr<threat_manager> m_threat_manager;
        
            /**
             * The alert_manager.
             */
            std::shared_ptr<alert_manager> m_alert_manager;
        
            /**
             * The icmp_manager.
             */
            std::shared_ptr<icmp_manager> m_icmp_manager;
        
            /**
             * The udp_manager.
             */
            std::shared_ptr<udp_manager> m_udp_manager;
        
        protected:
        
            /**
             * Initialize the home (application) directories.
             */
            void initialize_directories();
        
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
             * The network std::thread.
             */
            asio::io_service io_service_network_;
        
            /**
             * The network std::thread.
             */
            asio::strand strand_network_;
        
            /**
             * The network std::thread.
             */
            std::thread thread_network_;
   
            /**
             * The network timer.
             */
            asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_network_;
    };
} // namespace opensentinel

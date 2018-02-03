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
#include <cstdint>
#include <vector>

#define ASIO_STANDALONE 1

#include <asio.hpp>

namespace opensentinel {

    class stack_impl;
    class udp_listener;
    
    class udp_manager
    {
        public:
        
            /**
             * Constructor
             * @param owner The stack_impl.
             * @param ios The asio::io_service.
             * @param s The asio::strand.
             */
            explicit udp_manager(
                stack_impl & owner, asio::io_service & ios, asio::strand & s
            );
        
            /**
             * Starts
             */
            void start();
        
            /**
             * Stops
             */
            void stop();
        
        private:
        
            // ...
        
        protected:
        
            /**
             * The timer handler.
             */
            void on_tick();
        
            /**
             * Opens udp_listener objects.
             * @param port_begin The port to start at.
             * @param port_end The port to end at.
             */
            void open_udp_listener(
                const std::uint16_t & port_begin, const std::uint16_t & port_end
            );
        
            /**
             * Closes udp_listener objects.
             */
            void close_udp_listeners();
    
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
             * The stack_impl.
             */
            stack_impl & stack_impl_;
        
            /**
             * The asio::io_service.
             */
            asio::io_service & io_service_;
        
            /**
             * The asio::strand.
             */
            asio::strand & strand_;
        
            /**
             * The timer.
             */
            asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_;
        
            /**
             * The udp_listener object's
             */
            std::vector< std::weak_ptr<udp_listener> > udp_listeners_;
    };
    
} // opensentinel

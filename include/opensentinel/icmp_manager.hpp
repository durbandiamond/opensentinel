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

    class stack_impl;
    
    class icmp_manager
    {
        public:
        
            /**
             * Constructor
             * @param owner The stack_impl.
             */
            explicit icmp_manager(stack_impl & owner);
        
            /**
             * Starts
             */
            void start();
        
            /**
             * Stops
             */
            void stop();
        
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
             * Starts an IPv4 receive operation.
             */
            void async_receive_ipv4();
        
            /**
             * The receive handler.
             * @param len The length.
             */
            void handle_receive_ipv4(const std::size_t & len);
        
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
             * The asio::ip::icmp::socket.
             */
            asio::ip::icmp::socket socket_ipv4_;
        
            /**
             * The (read) asio::streambuf.
             */
            asio::streambuf read_streambuf_ipv4_;
    };
} // namespace opensentinel

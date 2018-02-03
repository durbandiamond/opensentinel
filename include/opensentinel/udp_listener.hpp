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

#include <cstdint>
#include <deque>
#include <functional>

#define ASIO_STANDALONE 1

#include <asio.hpp>

namespace opensentinel {

    /**
     * Implements a UDP listener.
     */
    class udp_listener : public std::enable_shared_from_this<udp_listener>
    {
        public:
        
            /**
             * Constructor
             * @param ios The asio::io_service.
             */
            explicit udp_listener(asio::io_service & ios);
            
            /**
             * Opens the socket(s) binding to port.
             * @param port The port to bind to.
             */
            void open(const std::uint16_t & port);
            
            /**
             * Closes the socket(s).
             */
            void close();
            
            /**
             * Performs a send to operation.
             * @param ep The destination endpoint.
             * @param buf The buffer to send.
             * @param len The length of bytes to send.
             */
            void send_to(
                const asio::ip::udp::endpoint & ep, const char * buf,
                const std::size_t & len
            );
        
            /**
             * Set the asynchronous receive handler.
             * @param f The std::function.
             */
            void set_on_async_receive_from(
                const std::function<void (
                const asio::ip::udp::endpoint &, const char *,
                const std::size_t &)> & f
            );
        
        private:
        
            /**
             * Handles an asynchronous receive from operation.
             * @param ec The std::error_code.
             * @param len The length of bytes received.
             */
            void handle_async_receive_from(
                const std::error_code & ec, const std::size_t & len
            );
        
            /**
             * Handles an asynchronous send to from operation.
             * @param ec The std::error_code.
             */
            void handle_async_send_to(const std::error_code & ec);
        
            /**
             * The asynchronous receive handler.
             */
            std::function<
                void (const asio::ip::udp::endpoint &, const char *,
                const std::size_t &)
            > m_on_async_receive_from;
        
        protected:
        
            /**
             * The maximum receive buffer length.
             */
            enum { max_length = 65535 };
        
            /**
             * The asio::io_service::stand.
             */
            asio::io_service::strand strand_;
            
            /**
             * The ipv4 socket.
             */
            asio::ip::udp::socket socket_ipv4_;
            
            /**
             * The ipv6 socket.
             */
            asio::ip::udp::socket socket_ipv6_;
            
            /**
             * The remote endpoint.
             */
            asio::ip::udp::endpoint remote_endpoint_;
            
            /**
             * The receive buffer.
             */
            char receive_buffer_[max_length];;
    };
    
} // namespace database

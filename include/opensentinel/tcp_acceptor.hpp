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
#include <functional>
#include <thread>
#include <vector>

#define ASIO_STANDALONE 1

#include <asio.hpp>

namespace opensentinel {

    class tcp_transport;
    
    /**
     * Implements a tcp acceptor.
     */
    class tcp_acceptor
        : public std::enable_shared_from_this<tcp_acceptor>
    {
        public:
        
            /**
             * Constructor
             * @param ios The asio::io_service.
             */
            explicit tcp_acceptor(asio::io_service & ios);
        
            /**
             * Opens the tcp connector given port.
             * @param port The port.
             */
            std::error_code open(const std::uint16_t & port);
        
            /**
             * Closes the acceptor.
             */
            void close();
        
            /**
             * Sets the accept handler
             * @param f The std::function.
             */
            void set_on_accept(
                const std::function<void (std::shared_ptr<tcp_transport>)> & f
            );
        
            /**
             * The local endpoint.
             */
            const asio::ip::tcp::endpoint local_endpoint() const;
        
            /**
             * Runs the test case.
             * @param ios The asio::io_service.
             */
            static int run_test(asio::io_service & ios);

        private:
        
            /**
             * Performs an ipv4 accept operation.
             */
            void do_ipv4_accept();
        
            /**
             * Performs an ipv6 accept operation.
             */
            void do_ipv6_accept();
        
            /**
             * The tick timerhandler.
             */
            void do_tick(const std::uint32_t & seconds);
        
            /**
             * The on accept handler.
             */
            std::function<void (std::shared_ptr<tcp_transport>)> m_on_accept;
        
            /**
             * The tcp_transport's.
             */
            std::vector< std::weak_ptr<tcp_transport> > m_tcp_transports;
        
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
             * The asio::io_service.
             */
            asio::io_service & io_service_;
        
            /**
             * The asio::strand.
             */
            asio::strand strand_;
        
            /**
             * The asio::ip::tcp::acceptor.
             */
            asio::ip::tcp::acceptor acceptor_ipv4_;
        
            /**
             * The asio::ip::tcp::acceptor.
             */
            asio::ip::tcp::acceptor acceptor_ipv6_;
        
            /**
             * The transports timer.
             */
            asio::basic_waitable_timer<
                std::chrono::steady_clock
            > transports_timer_;
    };

} // namespace opensentinel

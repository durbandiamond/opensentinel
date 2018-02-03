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

#include <cassert>
#include <iostream>
#include <stdexcept>

#include <opensentinel/logger.hpp>
#include <opensentinel/udp_listener.hpp>

using namespace opensentinel;

udp_listener::udp_listener(asio::io_service & ios)
    : strand_(ios)
    , socket_ipv4_(ios)
    , socket_ipv6_(ios)
{
    // ...
}

void udp_listener::open(const std::uint16_t & port)
{
    assert(!socket_ipv4_.is_open());
    assert(!socket_ipv6_.is_open());
    
    std::error_code ec;
    
    /**
     * Allocate the ipv4 endpoint.
     */
    asio::ip::udp::endpoint ipv4_endpoint(
        asio::ip::address_v4::any(), port
    );
    
    /**
     * Open the ipv4 socket.
     */
    socket_ipv4_.open(ipv4_endpoint.protocol(), ec);
    
    if (ec)
    {
        throw std::runtime_error(ec.message());
    }
    
    socket_ipv4_.set_option(
        asio::ip::udp::socket::reuse_address(true)
    );
    
    /**
     * Non-blocking IO.
     */
    asio::ip::udp::socket::non_blocking_io non_blocking_io(true);
    
    /**
     * Set the ipv4 socket to non-blocking.
     */
    socket_ipv4_.lowest_layer().io_control(non_blocking_io, ec);
    
    if (ec)
    {
        throw std::runtime_error(ec.message());
    }
    
    /**
     * Bind the ipv4 socket.
     */
    socket_ipv4_.bind(ipv4_endpoint);
    
    auto self(shared_from_this());
    
    /**
     * Start an asynchronous receive from on the ipv4 socket.
     */
    socket_ipv4_.async_receive_from(
        asio::buffer(receive_buffer_), remote_endpoint_, strand_.wrap(
        std::bind(&udp_listener::handle_async_receive_from, self,
        std::placeholders::_1, std::placeholders::_2))
    );
    
    /**
     * Allocate the ipv6 endpoint.
     */
    asio::ip::udp::endpoint ipv6_endpoint(asio::ip::address_v6::any(), port);
    
    /**
     * Open the ipv6 socket.
     */
    socket_ipv6_.open(ipv6_endpoint.protocol(), ec);
    
    if (ec)
    {
        throw std::runtime_error(ec.message());
    }
    
    /**
     * Set the ipv6 socket to non-blocking.
     */
    socket_ipv6_.lowest_layer().io_control(non_blocking_io, ec);
    
    if (ec)
    {
        throw std::runtime_error(ec.message());
    }
    
#if (! defined _MSC_VER)
    /**
     * Set the ipv6 socket to use v6 only.
     */
    socket_ipv6_.set_option(asio::ip::v6_only(true));
#endif // _MSC_VER
    
    /**
     * Bind the ipv6 socket.
     */
    socket_ipv6_.bind(ipv6_endpoint);
    
    /**
     * Start an asynchronous receive from on the ipv6 socket.
     */
    socket_ipv6_.async_receive_from(
        asio::buffer(receive_buffer_), remote_endpoint_, strand_.wrap(
        std::bind(&udp_listener::handle_async_receive_from, self,
        std::placeholders::_1, std::placeholders::_2))
    );
    
    log_info(
        "UDP listener local ipv4 endpoint = " <<
        asio::ip::udp::endpoint(socket_ipv4_.local_endpoint().address(),
        socket_ipv4_.local_endpoint().port()) << "."
    );
    
    log_info(
        "UDP listener local ipv6 endpoint = " <<
        asio::ip::udp::endpoint(socket_ipv6_.local_endpoint().address(),
        socket_ipv6_.local_endpoint().port()) << "."
    );
}

void udp_listener::close()
{
    /**
     * Close the ipv4 socket.
     */
    if (socket_ipv4_.is_open())
    {
        socket_ipv4_.close();
    }
    
    /**
     * Close the ipv6 socket.
     */
    if (socket_ipv6_.is_open())
    {
        socket_ipv6_.close();
    }
}

void udp_listener::send_to(
    const asio::ip::udp::endpoint & ep, const char * buf,
    const std::size_t & len
    )
{
    /**
     * Do not send packets greater than max_length.
     */
    if (len <= max_length)
    {
        if (ep.protocol() == asio::ip::udp::v4())
        {
            if (socket_ipv4_.is_open())
            {
                std::error_code ec;
                
                /**
                 * Perform a blocking send_to.
                 */
                socket_ipv4_.send_to(asio::buffer(buf, len), ep, 0, ec);
                
                if (ec)
                {
                    log_debug("UDP v4 send failed " << ec.message() << ".");
                    
                    if (ec == asio::error::broken_pipe)
                    {
                        auto port = socket_ipv4_.local_endpoint().port();
                        
                        close();
                        
                        open(port);
                        
                        std::error_code ignored_ec;
                        
                        /**
                         * Perform a blocking send_to.
                         */
                        socket_ipv4_.send_to(
                            asio::buffer(buf, len), ep, 0, ignored_ec
                        );
                    }
                }
            }
        }
        else
        {
            if (socket_ipv6_.is_open())
            {
                std::error_code ec;
                
                /**
                 * Perform a blocking send_to.
                 */
                socket_ipv6_.send_to(asio::buffer(buf, len), ep, 0, ec);

                if (ec)
                {
                    log_debug("UDP v6 send failed " << ec.message() << ".");
                    
                    if (ec == asio::error::broken_pipe)
                    {
                        auto port = socket_ipv6_.local_endpoint().port();
                        
                        close();
                        
                        open(port);
                        
                        std::error_code ignored_ec;
                        
                        /**
                         * Perform a blocking send_to.
                         */
                        socket_ipv6_.send_to(
                            asio::buffer(buf, len), ep, 0, ignored_ec
                        );
                    }
                }
            }
        }
    }
}

void udp_listener::set_on_async_receive_from(
    const std::function<void (const asio::ip::udp::endpoint &,
    const char *, const std::size_t &)> & f
    )
{
    m_on_async_receive_from = f;
}

void udp_listener::handle_async_receive_from(
    const std::error_code & ec, const std::size_t & len
    )
{
    if (ec == asio::error::operation_aborted)
    {
        // ...
    }
    else if (ec == asio::error::bad_descriptor)
    {
        // ...
    }
    else if (ec)
    {
        log_debug(
            "UDP listener receive failed, message = " << ec.message() << "."
        );
        
        auto self(shared_from_this());
        
        if (remote_endpoint_.protocol() == asio::ip::udp::v4())
        {
            /**
             * Start an asynchronous receive from on the ipv4 socket.
             */
            socket_ipv4_.async_receive_from(
                asio::buffer(receive_buffer_), remote_endpoint_,
                strand_.wrap(std::bind(
                &udp_listener::handle_async_receive_from,
                self, std::placeholders::_1, std::placeholders::_2))
            );
        }
        else
        {
            /**
             * Start an asynchronous receive from on the ipv6 socket.
             */
            socket_ipv6_.async_receive_from(
                asio::buffer(receive_buffer_), remote_endpoint_,
                strand_.wrap(std::bind(
                &udp_listener::handle_async_receive_from, self,
                std::placeholders::_1, std::placeholders::_2))
            );
        }
    }
    else
    {
        /**
         * Drop packets greater than max_length.
         */
        if (m_on_async_receive_from && (len > 0 && len <= max_length))
        {
            m_on_async_receive_from(
                remote_endpoint_, &receive_buffer_[0], len
            );
        }
        
        auto self(shared_from_this());
        
        if (remote_endpoint_.protocol() == asio::ip::udp::v4())
        {
            /**
             * Start an asynchronous receive from on the ipv4 socket.
             */
            socket_ipv4_.async_receive_from(
                asio::buffer(receive_buffer_), remote_endpoint_,
                strand_.wrap(std::bind(
                &udp_listener::handle_async_receive_from, self,
                std::placeholders::_1, std::placeholders::_2))
            );
        }
        else
        {
            /**
             * Start an asynchronous receive from on the ipv6 socket.
             */
            socket_ipv6_.async_receive_from(
                asio::buffer(receive_buffer_), remote_endpoint_,
                strand_.wrap(std::bind(
                &udp_listener::handle_async_receive_from, self,
                std::placeholders::_1, std::placeholders::_2))
            );
        }
    }
}

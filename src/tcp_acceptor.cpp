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

#include <iostream>
#include <sstream>

#include <opensentinel/logger.hpp>
#include <opensentinel/tcp_acceptor.hpp>
#include <opensentinel/tcp_transport.hpp>

using namespace opensentinel;

tcp_acceptor::tcp_acceptor(asio::io_service & ios)
    : state_(state_none)
    , io_service_(ios)
    , strand_(ios)
    , acceptor_ipv4_(io_service_)
    , acceptor_ipv6_(io_service_)
    , transports_timer_(io_service_)
{
    // ...
}

std::error_code tcp_acceptor::open(const std::uint16_t & port)
{
    assert(!acceptor_ipv4_.is_open());
    assert(!acceptor_ipv6_.is_open());
    
    state_ = state_starting;
    
    log_debug("TCP acceptor is opening with port = " << port);
    
    std::error_code ec;
    
    /**
     * Allocate the ipv4 endpoint.
     */
    asio::ip::tcp::endpoint ipv4_endpoint(
        asio::ip::address_v4::any(), port
    );
    
    /**
     * Open the ipv4 socket.
     */
    acceptor_ipv4_.open(asio::ip::tcp::v4(), ec);
    
    if (ec)
    {
        log_error("ipv4 open failed, message = " << ec.message());
        
        acceptor_ipv4_.close();
        
        return ec;
    }
    
    /**
     * Set option SO_REUSEADDR.
     */
    acceptor_ipv4_.set_option(
        asio::ip::tcp::acceptor::reuse_address(true)
    );
    
    /**
     * Bind the socket.
     */
    acceptor_ipv4_.bind(ipv4_endpoint, ec);
   
    if (ec)
    {
        log_error("ipv4 bind failed, message = " << ec.message());
        
        acceptor_ipv4_.close();
        
        return ec;
    }
    
    /**
     * Listen
     */
    acceptor_ipv4_.listen();
    
    /**
     * Accept
     */
    do_ipv4_accept();
    
    /**
     * Allocate the ipv6 endpoint.
     */
    asio::ip::tcp::endpoint ipv6_endpoint(
        asio::ip::address_v6::any(),
        acceptor_ipv4_.local_endpoint().port()
    );
    
    /**
     * Open the ipv6 socket.
     */
    acceptor_ipv6_.open(asio::ip::tcp::v6(), ec);
    
    if (ec)
    {
        log_error("ipv6 open failed, message = " << ec.message());
        
        acceptor_ipv4_.close();
        acceptor_ipv6_.close();
        
        return ec;
    }
    
#if defined(__linux__) || defined(__APPLE__)
    acceptor_ipv6_.set_option(asio::ip::v6_only(true));
#endif

    /**
     * Set option SO_REUSEADDR.
     */
    acceptor_ipv6_.set_option(
        asio::ip::tcp::acceptor::reuse_address(true)
    );
    
    /**
     * Bind the socket.
     */
    acceptor_ipv6_.bind(ipv6_endpoint, ec);
   
    if (ec)
    {
        log_error("ipv6 bind failed, message = " << ec.message());
        
        acceptor_ipv4_.close();
        acceptor_ipv6_.close();
        
        return ec;
    }
    
    /**
     * Listen
     */
    acceptor_ipv6_.listen();
    
    /**
     * Accept
     */
    do_ipv6_accept();
    
    /**
     * Start the tick timer.
     */
    do_tick(1);

    state_ = state_started;
    
    return ec;
}

void tcp_acceptor::close()
{
    log_info(
        "TCP acceptor is stopping, transports = " <<
        m_tcp_transports.size() << "."
    );
    
    state_ = state_stopping;

    auto self(shared_from_this());
    
    io_service_.post(strand_.wrap([this, self]()
    {
        auto port = local_endpoint().port();
        
        if (acceptor_ipv4_.is_open() == true)
        {
            acceptor_ipv4_.close();
        }
        
        if (acceptor_ipv6_.is_open() == true)
        {
            acceptor_ipv6_.close();
        }

        transports_timer_.cancel();
    
        m_on_accept = std::function<void (std::shared_ptr<tcp_transport>)> ();
        
        for (auto & i : m_tcp_transports)
        {
            if (auto j = i.lock())
            {
                j->stop();
            }
        }
        
        m_tcp_transports.clear();
        
        log_info("TCP acceptor port " << port << " has stopped.");
    
        state_ = state_stopped;
    }));
}

void tcp_acceptor::set_on_accept(
    const std::function<void (std::shared_ptr<tcp_transport>)> & f
    )
{
    m_on_accept = f;
}

const asio::ip::tcp::endpoint tcp_acceptor::local_endpoint() const
{
    return acceptor_ipv4_.is_open() ?
        acceptor_ipv4_.local_endpoint() :
        acceptor_ipv6_.local_endpoint()
    ;
}

void tcp_acceptor::do_ipv4_accept()
{
    if (state_ == state_starting || state_ == state_started)
    {
        auto self(shared_from_this());

        auto t = std::make_shared<tcp_transport> (io_service_);
        
        m_tcp_transports.push_back(t);
        
        acceptor_ipv4_.async_accept(t->socket(), strand_.wrap(
            [this, self, t](std::error_code ec)
        {
            if (acceptor_ipv4_.is_open() == true)
            {
                if (ec)
                {
                    log_error(
                        "TCP acceptor accept failed, message = " <<
                        ec.message() << "."
                    );
                }
                else
                {
                    try
                    {
                        auto remote_endpoint = t->socket().remote_endpoint();

                        /**
                         * Callback
                         */
                        if (m_on_accept)
                        {
                            log_info(
                                "Accepting tcp connection from " <<
                                remote_endpoint
                            );
                            
                            m_on_accept(t);
                        }
                        else
                        {
                            log_info(
                                "Dropping tcp connection from " <<
                                remote_endpoint << " no handler set."
                            );
                        }
                    }
                    catch (std::exception & e)
                    {
                        log_error(
                            "TCP acceptor remote_endpoint, what = " << e.what()
                        );
                    }
                }
                
                do_ipv4_accept();
            }
        }));
    }
}

void tcp_acceptor::do_ipv6_accept()
{
    if (state_ == state_starting || state_ == state_started)
    {
        auto self(shared_from_this());

        auto t = std::make_shared<tcp_transport> (io_service_);
        
        m_tcp_transports.push_back(t);
        
        acceptor_ipv6_.async_accept(t->socket(), strand_.wrap(
            [this, self, t](std::error_code ec)
        {
            if (acceptor_ipv6_.is_open() == true)
            {
                if (ec)
                {
                    log_error(
                        "TCP acceptor accept failed, message = " <<
                        ec.message() << "."
                    );
                }
                else
                {
                    try
                    {
                        asio::ip::tcp::endpoint remote_endpoint =
                            t->socket().remote_endpoint()
                        ;
                        
                        log_debug(
                            "Accepting tcp connection from " << remote_endpoint
                        );
                    
                        /**
                         * Callback
                         */
                        m_on_accept(t);
                    }
                    catch (std::exception & e)
                    {
                        log_error(
                            "TCP acceptor remote_endpoint, what = " << e.what()
                        );
                    }
                }
                
                do_ipv6_accept();
            }
        }));
    }
}

void tcp_acceptor::do_tick(const std::uint32_t & seconds)
{
    if (state_ == state_starting || state_ == state_starting)
    {
        transports_timer_.expires_from_now(std::chrono::seconds(seconds));
        transports_timer_.async_wait(strand_.wrap(
            [this, seconds](std::error_code ec)
        {
            if (ec)
            {
                // ...
            }
            else
            {
                auto it = m_tcp_transports.begin();
                
                while (it != m_tcp_transports.end())
                {
                    if (auto t = it->lock())
                    {
                        ++it;
                    }
                    else
                    {
                        it = m_tcp_transports.erase(it);
                    }
                }
                
                do_tick(seconds);
            }
        }));
    }
}

int tcp_acceptor::run_test(asio::io_service & ios)
{
    auto acceptor = std::make_shared<tcp_acceptor> (ios);
    
    acceptor->set_on_accept(
        [](std::shared_ptr<tcp_transport> transport)
        {
            /**
             * Set the transport on read handler.
             */
            transport->set_on_read(
                [](std::shared_ptr<tcp_transport> t,
                const char * buf, const std::size_t & len)
            {
                std::cout <<
                    "tcp_transport read " << len << " bytes, buffer = " <<
                    buf <<
                std::endl;
            });
    
            transport->start();
        }
    );
    
    std::error_code ret;
    
    try
    {
        enum { port_start = 8080 };
        
        auto port = static_cast<std::uint16_t> (port_start);
        
        while (ret)
        {
            ret = acceptor->open(port);
            
            if (ret)
            {
                port += 2;
            }
            else
            {
                break;
            }
            
            /**
             * Try 50 even ports.
             */
            if (port > port_start + 100)
            {
                break;
            }
        }
        
        std::cout <<
            "tcp_acceptor::run_test opened on port = " << port <<
        std::endl;
    }
    catch (std::exception & e)
    {
        std::cerr << "what = " << e.what() << std::endl;
    }
    
    if (ret)
    {
        std::cerr << "tcp_acceptor::run_test failed" << std::endl;
    }

    return 0;
}

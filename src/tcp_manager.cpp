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

#include <opensentinel/logger.hpp>
#include <opensentinel/stack_impl.hpp>
#include <opensentinel/tcp_acceptor.hpp>
#include <opensentinel/tcp_manager.hpp>
#include <opensentinel/tcp_transport.hpp>
#include <opensentinel/threat.hpp>

using namespace opensentinel;

tcp_manager::tcp_manager(
    stack_impl & owner, asio::io_service & ios, asio::strand & s
    )
    : state_(state_none)
    , stack_impl_(owner)
    , io_service_(ios)
    , strand_(s)
    , timer_(ios)
{
    // ...
}

void tcp_manager::start()
{
    log_info("TCP Manager is starting...");
    
    state_ = state_starting;
    
    /**
     * Starts the timer.
     */
    timer_.expires_from_now(std::chrono::seconds(1));
    timer_.async_wait(strand_.wrap([this](std::error_code ec)
    {
        if (ec)
        {
            // ...
        }
        else
        {
            on_tick();
        }
    }));

    /**
     * Open the tcp_acceptor objects.
     * @note Skip NetBios, bootps and bootpc.
     */
    open_tcp_acceptors(1 /* tcpmux */, 66 /* sql-net */);
    open_tcp_acceptors(69 /* tftp */, 136 /* profile */);
    open_tcp_acceptors(140 /* emfis-data */, 2028 /* dls-monitor */);
    open_tcp_acceptors(8080 /* http-alt */, 8280 /* synapse-nhttp */);

    state_ = state_started;
    
    log_info("TCP Manager has started.");
}

void tcp_manager::stop()
{
    log_info("TCP Manager is stopping...");
    
    state_ = state_stopping;
    
    /**
     * Cancel the timer.
     */
    timer_.cancel();
    
    /**
     * Close all tcp_acceptor object's
     */
    close_tcp_acceptors();
    
    state_ = state_stopped;
    
    log_info("TCP Manager has stopped.");
}

void tcp_manager::on_tick()
{
    auto it1 = tcp_acceptors_.begin();
    
    while (it1 != tcp_acceptors_.end())
    {
        const auto & acceptor = it1->lock();
        
        if (acceptor == nullptr)
        {
            it1 = tcp_acceptors_.erase(it1);
        }
        else
        {
            ++it1;
        }
    }

    /**
     * Starts the timer.
     */
    timer_.expires_from_now(std::chrono::seconds(8));
    timer_.async_wait(strand_.wrap([this](std::error_code ec)
    {
        if (ec)
        {
            // ...
        }
        else
        {
            on_tick();
        }
    }));
}

void tcp_manager::open_tcp_acceptors(
    const std::uint16_t & port_begin, const std::uint16_t & port_end
    )
{
    for (auto i = port_begin; i < (port_end + 1); i++)
    {
        auto acceptor = std::make_shared<tcp_acceptor> (io_service_);
    
        auto ec = acceptor->open(i);
        
        if (ec)
        {
            log_error(
                "TCP manager failed to open acceptor, message = " <<
                ec.message() << "."
            );
            
            /**
             * Check for too many open file descriptors.
             */
            if (ec.value() == 24 || ec.message() == "Too many open files")
            {
                break;
            }
        }
        else
        {
            acceptor->set_on_accept(
                [this](std::shared_ptr<tcp_transport> transport)
                {
                    auto remote_endpoint =
                        transport->socket().remote_endpoint()
                    ;
                    
                    /**
                     * Allocate the threat.
                     */
                    threat threat_data(
                        threat::protocol_tcp, remote_endpoint.address(),
                        remote_endpoint.port(), 0, 0
                    );

                    log_info(
                        "TCP manager has detected a possible threat (TCP Accept) "
                        "from " << remote_endpoint << ", dispatching to "
                        "threat_manager."
                    );
            
                    /**
                     * Callback
                     */
                    stack_impl_.on_threat(threat_data);
                    
                    /**
                     * Set the transport on read handler.
                     */
                    transport->set_on_read(
                        [this](std::shared_ptr<tcp_transport> t,
                        const char * buf, const std::size_t & len)
                    {
                        try
                        {
                            auto remote_endpoint = t->socket().remote_endpoint();

                            /**
                             * Allocate the threat.
                             */
                            threat threat_data(
                                threat::protocol_tcp, remote_endpoint.address(),
                                remote_endpoint.port(), buf, len
                            );
        
                            log_info(
                                "TCP manager has detected a possible threat "
                                "(TCP Read) from " << remote_endpoint <<
                                ", dispatching to threat_manager."
                            );
                            
                            /**
                             * Callback
                             */
                            stack_impl_.on_threat(threat_data);
                        }
                        catch (...)
                        {
                            // ...
                        }
                    });
                    
                    enum { read_write_timeout = 5 };
            
                    transport->set_read_timeout(read_write_timeout);
                    transport->set_write_timeout(read_write_timeout);
                    
                    transport->start();
                }
            );
        
            tcp_acceptors_.push_back(acceptor);
        }
    }
    
    log_info(
        "TCP manager opened " << tcp_acceptors_.size() <<
        " TCP acceptors."
    );
}

void tcp_manager::close_tcp_acceptors()
{
    log_info(
        "TCP manager is closing " << tcp_acceptors_.size() <<
        " TCP acceptors."
    );
    
    /**
     * Close all tcp_acceptor object's
     */
    for (auto & i : tcp_acceptors_)
    {
        if (auto j = i.lock())
        {
            j->close();
        }
    }
    
    /**
     * Clear all tcp_acceptor object's
     */
    tcp_acceptors_.clear();
}

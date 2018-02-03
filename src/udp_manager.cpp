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
#include <opensentinel/threat.hpp>
#include <opensentinel/udp_listener.hpp>
#include <opensentinel/udp_manager.hpp>

using namespace opensentinel;

udp_manager::udp_manager(
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

void udp_manager::start()
{
    log_info("UDP manager is starting...");
    
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
     * Open the udp_listeners objects.
     * @note Skip NetBios, bootps and bootpc.
     */
    open_udp_listener(1 /* tcpmux */, 66 /* sql-net */);
    open_udp_listener(69 /* tftp */, 136 /* profile */);
    open_udp_listener(140 /* emfis-data */, 2028 /* dls-monitor */);
    open_udp_listener(8080 /* http-alt */, 8280 /* synapse-nhttp */);
    
    state_ = state_started;
    
    log_info("UDP manager has started.");
}

void udp_manager::stop()
{
    log_info("UDP manager is stopping...");
    
    state_ = state_stopping;
    
    /**
     * Cancel the timer.
     */
    timer_.cancel();
    
    /**
     * Close all udp_listener object's
     */
    close_udp_listeners();
    
    state_ = state_stopped;
    
    log_info("UDP manager has stopped.");
}

void udp_manager::on_tick()
{
    auto it1 = udp_listeners_.begin();
    
    while (it1 != udp_listeners_.end())
    {
        const auto & acceptor = it1->lock();
        
        if (acceptor == nullptr)
        {
            it1 = udp_listeners_.erase(it1);
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

void udp_manager::open_udp_listener(
    const std::uint16_t & port_begin, const std::uint16_t & port_end
    )
{
    for (auto i = port_begin; i < (port_end + 1); i++)
    {
        auto listener = std::make_shared<udp_listener> (io_service_);
    
        try
        {
            listener->open(i);
        }
        catch (std::exception & e)
        {
            log_error(
                "UDP manager failed to open listener, what = " <<
                e.what() << "."
            );
            
            /**
             * Check for too many open file descriptors.
             */
            if (std::string(e.what()) == "Too many open files")
            {
                break;
            }
        }
        
        /**
         * Listen for UDP packets.x
         */
        listener->set_on_async_receive_from(strand_.wrap([this](
            const asio::ip::udp::endpoint & ep , const char * buf,
            const std::size_t & len
            )
        {
            try
            {
                /**
                 * Allocate the threat.
                 */
                threat threat_data(
                    threat::protocol_udp, ep.address(), ep.port(), buf, len
                );
                
                /**
                 * Set the threat::level_t.
                 */
                threat_data.set_level(threat::level_3);

                log_info(
                    "UDP manager has detected a possible threat "
                    "(UDP Receive) from " << ep <<
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
        }));
        
        udp_listeners_.push_back(listener);
    }
    
    log_info(
        "UDP manager opened " << udp_listeners_.size() <<
        " UDP listeners."
    );
}

void udp_manager::close_udp_listeners()
{
    log_info(
        "UDP manager is closing " << udp_listeners_.size() <<
        " UDP listeners."
    );
    
    /**
     * Close all udp_listener object's
     */
    for (auto & i : udp_listeners_)
    {
        if (auto j = i.lock())
        {
            j->close();
        }
    }
    
    /**
     * Clear all tcp_acceptor object's
     */
    udp_listeners_.clear();
}

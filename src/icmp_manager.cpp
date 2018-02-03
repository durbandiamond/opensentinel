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

#include <opensentinel/icmp.hpp>
#include <opensentinel/icmp_manager.hpp>
#include <opensentinel/ipv4_header.hpp>
#include <opensentinel/logger.hpp>
#include <opensentinel/stack_impl.hpp>
#include <opensentinel/threat.hpp>

using namespace opensentinel;

icmp_manager::icmp_manager(stack_impl & owner)
    : state_(state_none)
    , stack_impl_(owner)
    , strand_(io_service_)
    , timer_(io_service_)
    , socket_ipv4_(io_service_)
{
    // ...
}

void icmp_manager::start()
{
    log_info("ICMP manager is starting...");
    
    state_ = state_starting;
    
    std::error_code ec;
    
    assert(socket_ipv4_.is_open() == false);
    
    socket_ipv4_.open(asio::ip::icmp::v4(), ec);

    if (ec)
    {
        log_error(
            "ICMP manager failed to start, message = " << ec.message() <<
            "."
        );
        
        state_ = state_none;
        
        socket_ipv4_.close();
    }
    else
    {
        async_receive_ipv4();
        
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
    
        thread_ = std::thread(&icmp_manager::run, this);
        
        state_ = state_started;
        
        log_info("ICMP manager has started.");
    }
}

void icmp_manager::stop()
{
    log_info("ICMP manager is stopping...");
    
    state_ = state_stopping;
    
    /**
     * Cancel the timer.
     */
    timer_.cancel();
    
    if (socket_ipv4_.is_open() == true)
    {
        socket_ipv4_.close();
    }
    
    if (thread_.joinable() == true)
    {
        thread_.join();
    }
    
    state_ = state_stopped;
    
    log_info("ICMP manager has stopped.");
}

void icmp_manager::on_tick()
{
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
}

void icmp_manager::run()
{
    while (state_ == state_starting || state_ == state_started)
    {
        try
        {
            io_service_.run();
        }
        catch (std::exception & e)
        {
            log_error(
                "ICMP manager thread caught exception, what = " <<
                e.what() << "."
            );
        }
    }
    
    log_info("ICMP manager thread has stopped.");
}

void icmp_manager::async_receive_ipv4()
{
    if (state_ == state_starting || state_ == state_started)
    {
        read_streambuf_ipv4_.consume(read_streambuf_ipv4_.size());

        asio::ip::icmp::endpoint ep_ipv4;
        
        socket_ipv4_.async_receive_from(
            read_streambuf_ipv4_.prepare(65536), ep_ipv4,
            strand_.wrap(std::bind(&icmp_manager::handle_receive_ipv4, this,
            std::placeholders::_2))
        );
    }
}

void icmp_manager::handle_receive_ipv4(const std::size_t & len)
{
    if (state_ == state_starting || state_ == state_started)
    {
        read_streambuf_ipv4_.commit(len);

        std::istream is(&read_streambuf_ipv4_);

        ipv4_header ipv4_hdr;
        icmp::header icmp_hdr;
        is >> ipv4_hdr >> icmp_hdr;

        log_info(
            "ICMP manager got " << (len - ipv4_hdr.header_length()) <<
            " bytes from " << ipv4_hdr.source_address() << ", seq = " <<
            icmp_hdr.sequence_number() << ", ttl = " <<
            ipv4_hdr.time_to_live() << ", code = " <<
            static_cast<std::int32_t> (icmp_hdr.code()) << ", type = " <<
            icmp_hdr.type_string()
        );
        
        /**
         * Consider a PING to be a threat.
         */
        if (
            icmp_hdr.type() == icmp::header::type_echo_request ||
            icmp_hdr.type() == icmp::header::type_echo_reply
            )
        {
            auto remote_endpoint =
                asio::ip::tcp::endpoint(ipv4_hdr.source_address(), 0)
            ;

            /**
             * Allocate the threat.
             */
            threat threat_data(
                threat::protocol_icmp, remote_endpoint.address(),
                remote_endpoint.port(), 0, 0
            );
            
            /**
             * Set the level to threat::level_3.
             */
            threat_data.set_level(threat::level_3);

            log_info(
                "ICMP manager has detected a possible threat "
                "(ICMP Receive) from " << remote_endpoint <<
                ", dispatching to threat_manager."
            );

            /**
             * Callback
             */
            stack_impl_.on_threat(threat_data);
        }
        
        async_receive_ipv4();
    }
}


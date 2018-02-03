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
#include <stdexcept>

#include <opensentinel/alert_manager.hpp>
#include <opensentinel/icmp_manager.hpp>
#include <opensentinel/filesystem.hpp>
#include <opensentinel/logger.hpp>
#include <opensentinel/stack_impl.hpp>
#include <opensentinel/tcp_manager.hpp>
#include <opensentinel/threat.hpp>
#include <opensentinel/threat_manager.hpp>
#include <opensentinel/udp_manager.hpp>
#include <opensentinel/utility.hpp>

using namespace opensentinel;

stack_impl::stack_impl()
    : state_(state_none)
    , strand_network_(io_service_network_)
    , timer_network_(io_service_network_)
{
    // ...
}

void stack_impl::start()
{
    log_init(filesystem::data_path() + "debug.log");
    
    log_info("Stack is starting...");
    
    state_ = state_starting;
    
    /**
     * Initialize the home (application) directories.
     */
    initialize_directories();
    
    /**
     * We need at least 8096 possibly more file descriptors.
     */
    auto file_descriptor_limit = utility::raise_file_descriptor_limit(
        8096 * 2
    );
    
    log_info(
        "Stack set file descriptor limit to " << file_descriptor_limit << "."
    );
    
    /**
     * Allocate the tcp_manager.
     */
    m_tcp_manager = std::make_shared<tcp_manager> (
        *this, io_service_network_, strand_network_
    );
    
    /**
     * Start the tcp_manager.
     */
    m_tcp_manager->start();
    
    /**
     * Allocate the threat_manager.
     */
    m_threat_manager = std::make_shared<threat_manager> (*this);
    
    /**
     * Start the threat_manager.
     */
    m_threat_manager->start();
    
    /**
     * Allocate the alert_manager.
     */
    m_alert_manager = std::make_shared<alert_manager> ();
    
    /**
     * Start the alert_manager.
     */
    m_alert_manager->start();
    
    try
    {
        /**
         * Allocate the icmp_manager.
         */
        m_icmp_manager = std::make_shared<icmp_manager> (*this);
        
        /**
         * Start the icmp_manager.
         */
        m_icmp_manager->start();
    }
    catch (std::exception & e)
    {
        log_error(
            "Stack failed to start icmp_manager, what = " << e.what() << "."
        );
    }

    /**
     * Allocate the udp_manager.
     */
    m_udp_manager = std::make_shared<udp_manager> (
        *this, io_service_network_, strand_network_
    );
    
    /**
     * Start the udp_manager.
     */
    m_udp_manager->start();

    /**
     * Starts the network timer.
     */
    timer_network_.expires_from_now(std::chrono::seconds(1));
    timer_network_.async_wait(strand_network_.wrap([this](std::error_code ec)
    {
        if (ec)
        {
            // ...
        }
        else
        {
            on_tick_network();
        }
    }));

    thread_network_ = std::thread(&stack_impl::network_run, this);
    
    state_ = state_started;
    
    log_info("Stack has started.");
}

void stack_impl::stop()
{
    log_info("Stack is stopping...");
    
    state_ = state_stopping;

    /**
     * Cancel the network timer.
     */
    timer_network_.cancel();
    
    /**
     * Stop the tcp_manager.
     */
    if (m_tcp_manager != nullptr)
    {
        m_tcp_manager->stop();
    }
    
    /**
     * Stop the threat_manager.
     */
    if (m_threat_manager != nullptr)
    {
        m_threat_manager->stop();
    }
    
    /**
     * Stop the alert_manager.
     */
    if (m_alert_manager != nullptr)
    {
        m_alert_manager->stop();
    }
    
    /**
     * Stop the icmp_manager.
     */
    if (m_icmp_manager != nullptr)
    {
        m_icmp_manager->stop();
    }
    
    /**
     * Stop the udp_manager.
     */
    if (m_udp_manager != nullptr)
    {
        m_udp_manager->stop();
    }
    
    if (thread_network_.joinable() == true)
    {
        thread_network_.join();
    }
    
    m_tcp_manager = nullptr;
    
    state_ = state_stopped;
    
    log_info("Stack has stopped.");
}

void stack_impl::on_threat(const threat & threat_data)
{
    strand_network_.dispatch([this, threat_data]()
    {
        if (m_threat_manager != nullptr)
        {
            m_threat_manager->on_threat(threat_data);
        }
    });
}

std::shared_ptr<alert_manager> & stack_impl::get_alert_manager()
{
    return m_alert_manager;
}

void stack_impl::on_tick_network()
{
    /**
     * Starts the network timer.
     */
    timer_network_.expires_from_now(std::chrono::seconds(1));
    timer_network_.async_wait(strand_network_.wrap([this](std::error_code ec)
    {
        if (ec)
        {
            // ...
        }
        else
        {
            on_tick_network();
        }
    }));
}

void stack_impl::network_run()
{
    while (state_ == state_starting || state_ == state_started)
    {
        try
        {
            io_service_network_.run();
        }
        catch (std::exception & e)
        {
            log_error(
                "Network thread caught exception, what = " << e.what() << "."
            );
        }
    }
    
    log_info("Network thread has stopped.");
}

void stack_impl::initialize_directories()
{
    /**
     * Get the data path.
     */
    auto path_data = filesystem::data_path();
    
    log_info(
        "Stack data path = " << path_data << "."
    );

    filesystem::create_path(path_data);
}

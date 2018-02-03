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

#include <stdexcept>

#include <opensentinel/alert_manager.hpp>
#include <opensentinel/logger.hpp>
#include <opensentinel/stack_impl.hpp>
#include <opensentinel/threat.hpp>
#include <opensentinel/threat_manager.hpp>

using namespace opensentinel;

threat_manager::threat_manager(stack_impl & owner)
    : state_(state_none)
    , stack_impl_(owner)
    , strand_(io_service_)
    , timer_(io_service_)
{
    // ...
}

void threat_manager::start()
{
    log_info("Threat manager is starting...");
    
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

    thread_ = std::thread(&threat_manager::run, this);
    
    state_ = state_started;
    
    log_info("Threat manager has started.");
}

void threat_manager::stop()
{
    log_info("Threat manager is stopping...");
    
    state_ = state_stopping;

    /**
     * Cancel the timer.
     */
    timer_.cancel();
    
    if (thread_.joinable() == true)
    {
        thread_.join();
    }
    
    state_ = state_stopped;
    
    log_info("Threat manager has stopped.");
}

void threat_manager::on_threat(const threat & threat_data)
{
    log_info("Threat manager got threat.");
    
    io_service_.post(strand_.wrap([this, threat_data]()
    {
        /**
         * Print the threat to the console.
         */
        threat_data.print();
        
        /**
         * Check the threat; if the threat::level_t is > 0 send it to the
         * alert_manager.
         */
        if (
            check_threat(*const_cast<threat *> (&threat_data)) == true &&
            threat_data.level() > threat::level_0
            )
        {
            log_info(
                "Threat manager checked threat(" << threat_data.protocol() <<
                ") of level " << threat_data.level() << ", dispatching to "
                "the alert_manager."
            );
            
            /**
             * Inform the alert_manager.
             */
            if (stack_impl_.get_alert_manager() != nullptr)
            {
                stack_impl_.get_alert_manager()->on_threat(threat_data);
            }
        }
        else
        {
            log_info(
                "Threat manager is dropping threat(" <<
                threat_data.protocol() << ") of level " <<
                threat_data.level() << "."
            );
        }
    }));
}

void threat_manager::on_tick()
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

void threat_manager::run()
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
                "Threat manager thread caught exception, what = " <<
                e.what() << "."
            );
        }
    }
    
    log_info("Threat manager thread has stopped.");
}

bool threat_manager::check_threat(threat & val)
{
    const auto & buffer = val.buffer();
    
    if (buffer.size() == 0)
    {
        if (val.level() == threat::level_0)
        {
            val.set_level(threat::level_1);
        }
    }
    else
    {
        /**
         * If we are checking the threat sample buffer from a set of known
         * hostile fingerprints we can escalate the threat::level_t if there is
         * a match.
         * @note This can be a database of samples or in-memory.
         */
        std::vector<char> fingerprint_test;
#if 0
        fingerprint_test.push_back('H');
        fingerprint_test.push_back('T');
        fingerprint_test.push_back('T');
        fingerprint_test.push_back('P');
#else
        fingerprint_test.push_back('F');
        fingerprint_test.push_back('O');
        fingerprint_test.push_back('O');
#endif
        auto it = std::search(
            buffer.begin(), buffer.end(), fingerprint_test.begin(),
            fingerprint_test.end()
        );
    
        if (it != buffer.end())
        {
            val.set_level(threat::level_3);
        }
        else if (buffer.size() > 0)
        {
            val.set_level(threat::level_2);
        }
        else
        {
            val.set_level(threat::level_1);
        }
    }
    
    return val.level() > threat::level_0;
}


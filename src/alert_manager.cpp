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

#include <cstdio>
#include <fstream>

#include <opensentinel/alert.hpp>
#include <opensentinel/alert_manager.hpp>
#include <opensentinel/filesystem.hpp>
#include <opensentinel/logger.hpp>
#include <opensentinel/threat.hpp>

using namespace opensentinel;

alert_manager::alert_manager()
    : m_file_threat_alert("threat_alert.sh")
    , state_(state_none)
    , strand_(io_service_)
    , timer_(io_service_)
{
    // ...
}

void alert_manager::start()
{
    log_info("Alert manager is starting...");
    
    state_ = state_starting;

    /**
     * Check that the threat_alert file exists.
     */
    std::ifstream ifs(filesystem::data_path() + m_file_threat_alert);

    if (ifs.good() == true)
    {
        ifs.close();
    }
    else
    {
        log_info("Alert manager is initializing (sample) threat_alert file.");
        
        /**
         * Write a default threat_alert file.
         */
        std::ofstream ofs(filesystem::data_path() + m_file_threat_alert);
    
        ofs << "#!/bin/bash\n";
        ofs << "echo \"OpenSentinel got threat alert from $1.\"\n";
        ofs << "echo \"Taking action...\"\n";
    }
    
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
    
    thread_ = std::thread(&alert_manager::run, this);
    
    state_ = state_started;
    
    log_info("Alert manager has started.");
}

void alert_manager::stop()
{
    log_info("Alert manager is stopping...");
    
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
    
    log_info("Alert manager has stopped.");
}

void alert_manager::on_threat(const threat & threat_data)
{
    io_service_.post(strand_.wrap([this, threat_data]()
    {
        alert alert_data(threat_data);
        
        /**
         * Check for (recent) duplicate alerts.
         * @note This std::map is protected by our asio::strand.
         */
        if (alert_cache_.count(alert_data.fingerprint()) > 0)
        {
            log_info(
                "Alert manager got duplicate alert fingerprint = " <<
                alert_data.fingerprint() << ", dropping until " <<
                std::time(0) - alert_cache_[alert_data.fingerprint()] <<
                " seconds."
            );

            return;
        }
        else
        {
            alert_cache_[alert_data.fingerprint()] = std::time(0);
        }

        std::thread([this, alert_data]()
        {
            /**
             * Quote the path in case it has spaces.
             */
            auto command =
                "\"" + filesystem::data_path() + m_file_threat_alert + "\" " +
                alert_data.to_string()
            ;
            
            log_info(
                "Alert manager is executing system command = " << command
            );
        
            auto ret = system(command.c_str());
        
            log_info(
                "Alert manager called system command, ret = " << ret << "."
            );
            
        }).detach();
    }));
}

void alert_manager::on_tick()
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
            /**
             * Erase any old alert's.
             * @note This std::map is protected by our asio::strand.
             */
            auto it1 = alert_cache_.begin();
            
            while (it1 != alert_cache_.end())
            {
                if (std::time(0) - it1->second > 60)
                {
                    it1 = alert_cache_.erase(it1);
                }
                else
                {
                    ++it1;
                }
            }
            
            on_tick();
        }
    }));
}

void alert_manager::run()
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
                "Alert manager thread caught exception, what = " <<
                e.what() << "."
            );
        }
    }
    
    log_info("Alert manager thread has stopped.");
}

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
#include <sstream>

#include <opensentinel/logger.hpp>
#include <opensentinel/tcp_transport.hpp>

using namespace opensentinel;

#if (defined USE_TOKEN_BUCKET && USE_TOKEN_BUCKET)
token_bucket tcp_transport::g_token_bucket_read(256000, 384000);
token_bucket tcp_transport::g_token_bucket_write(256000, 384000);
#endif // USE_TOKEN_BUCKET

tcp_transport::tcp_transport(asio::io_service & ios)
    : m_strand(ios)
    , m_state(state_disconnected)
    , m_close_after_writes(false)
    , m_read_timeout(0)
    , m_write_timeout(0)
    , m_interval_last_read(0)
    , m_interval_last_write(0)
    , m_bytes_total_read(0)
    , m_bytes_total_write(0)
    , m_bytes_total_interval_read(0)
    , m_bytes_total_interval_write(0)
    , m_bytes_per_second_read(0)
    , m_bytes_per_second_write(0)
    , io_service_(ios)
    , timer_(ios)
    , connect_timeout_timer_(io_service_)
    , read_timeout_timer_(io_service_)
    , write_timeout_timer_(io_service_)
#if (defined USE_TOKEN_BUCKET && USE_TOKEN_BUCKET)
    , read_retry_timer_(ios)
    , write_retry_timer_(ios)
#endif // USE_TOKEN_BUCKET
{
   m_socket.reset(new asio::ip::tcp::socket(ios));
}

tcp_transport::~tcp_transport()
{
    // ...
}
        
void tcp_transport::start(
    const std::string & hostname, const std::uint16_t & port,
    const std::function<void (std::error_code,
    std::shared_ptr<tcp_transport>)> & f
    )
{
    /**
     * Set the completion handler.
     */
    m_on_complete = f;

    auto self(shared_from_this());
    
    /**
     * Start the tick timer.
     */
    timer_.expires_from_now(std::chrono::seconds(1));
    timer_.async_wait(m_strand.wrap(
        std::bind(&tcp_transport::tick, self, std::placeholders::_1))
    );
    
    connect_timeout_timer_.expires_from_now(std::chrono::seconds(8));
    connect_timeout_timer_.async_wait(m_strand.wrap(
        [this, self](std::error_code ec)
    {
        if (ec)
        {
            // ...
        }
        else
        {
            log_none(
                "TCP transport connect operation timed out after 8 "
                "seconds, closing."
            );
            
            /**
             * Stop
             */
             stop();
        }
    }));
    
    try
    {
        try
        {
            do_connect(
                asio::ip::tcp::endpoint(
                asio::ip::address::from_string(hostname), port)
            );
        }
        catch (...)
        {
            asio::ip::tcp::resolver resolver(io_service_);
            asio::ip::tcp::resolver::query query(
                hostname, std::to_string(port)
            );
            do_connect(resolver.resolve(query));
        }
    }
    catch (std::exception & e)
    {
        log_debug("TCP transport start failed, what = " << e.what());
    }
}

void tcp_transport::start()
{
    auto self(shared_from_this());
    
    /**
     * Start the tick timer.
     */
    timer_.expires_from_now(std::chrono::seconds(1));
    timer_.async_wait(m_strand.wrap(
        std::bind(&tcp_transport::tick, self, std::placeholders::_1))
    );
    
    m_state = state_connected;
        
    do_read();
}
        
void tcp_transport::stop()
{
    if (m_state != state_disconnected)
    {
        /**
         * Set the state to state_disconnected.
         */
        m_state = state_disconnected;
        
        /**
         * Shutdown the socket.
         */
        if (m_socket && m_socket->lowest_layer().is_open())
        {
            std::error_code ec;
            
            m_socket->lowest_layer().shutdown(
                asio::ip::tcp::socket::shutdown_both, ec
            );
            
            if (ec)
            {
                log_debug(
                    "TCP transport socket shutdown error = " <<
                    ec.message() << "."
                );
            }
        }
        
        /**
         * Stop the tick timer.
         */
        timer_.cancel();
        
        connect_timeout_timer_.cancel();
        read_timeout_timer_.cancel();
        write_timeout_timer_.cancel();
        
#if (defined USE_TOKEN_BUCKET && USE_TOKEN_BUCKET)
        read_retry_timer_.cancel();
        write_retry_timer_.cancel();
#endif // USE_TOKEN_BUCKET

        /**
         * Close the socket.
         */
        if (m_socket)
        {
            m_socket->lowest_layer().close();
        }

        m_on_complete = std::function<
            void (std::error_code, std::shared_ptr<tcp_transport>)
        > ();

        m_on_read = std::function<
            void (std::shared_ptr<tcp_transport>, const char *,
            const std::size_t &)
        > ();
    }
}

void tcp_transport::set_on_read(
    const std::function<void (std::shared_ptr<tcp_transport>, const char *,
    const std::size_t &)> & f
    )
{
    m_on_read = f;
}

asio::strand & tcp_transport::strand()
{
    return m_strand;
}

void tcp_transport::write(const char * buf, const std::size_t & len)
{    
    auto self(shared_from_this());
    
    std::vector<char> buffer(buf, buf + len);
    
    if (m_state == state_connected)
    {
        io_service_.post(m_strand.wrap(
            [this, self, buffer]()
        {
            auto write_in_progress = write_queue_.size() > 0;
            
            write_queue_.push_back(buffer);
          
            if (write_in_progress == false)
            {
                do_write(
                    &write_queue_.front()[0], write_queue_.front().size()
                );
            }
        }));
    }
    else
    {
        io_service_.post(m_strand.wrap(
            [this, self, buffer]()
        {
            write_queue_.push_back(buffer);
        }));
    }
}

tcp_transport::state_t & tcp_transport::state()
{
    return m_state;
}

void tcp_transport::set_identifier(const std::string & val)
{
    m_identifier = val;
}

const std::string & tcp_transport::identifier() const
{
    return m_identifier;
}

asio::ip::tcp::socket & tcp_transport::socket()
{
    return *m_socket;
}

void tcp_transport::set_close_after_writes(const bool & flag)
{
    m_close_after_writes = flag;
}

void tcp_transport::set_read_timeout(const std::uint32_t & val)
{
    m_read_timeout = val;
}

void tcp_transport::set_write_timeout(const std::uint32_t & val)
{
    m_write_timeout = val;
}

const std::time_t tcp_transport::time_last_read()
{
    auto time_diff =
        std::chrono::duration_cast<std::chrono::milliseconds> (
        std::chrono::system_clock::now().time_since_epoch()) -
        m_interval_last_read
    ;
    
    if (time_diff.count() == 0)
    {
        return 0;
    }
    
    return
        std::max(static_cast<std::int64_t> (1000), time_diff.count()) / 1000
    ;
}

const std::time_t tcp_transport::time_last_write()
{
    auto time_diff =
        std::chrono::duration_cast<std::chrono::milliseconds> (
        std::chrono::system_clock::now().time_since_epoch()) -
        m_interval_last_write
    ;
    
    if (time_diff.count() == 0)
    {
        return 0;
    }
    
    return
        std::max(static_cast<std::int64_t> (1000), time_diff.count()) / 1000
    ;
}

const std::size_t & tcp_transport::bytes_total_read() const
{
    return m_bytes_total_read;
}

const std::size_t & tcp_transport::bytes_total_write() const
{
    return m_bytes_total_write;
}

const std::size_t & tcp_transport::bytes_per_second_read() const
{
    return m_bytes_per_second_read;
}

const std::size_t & tcp_transport::bytes_per_second_write() const
{
    return m_bytes_per_second_write;
}

#if (defined USE_TOKEN_BUCKET && USE_TOKEN_BUCKET)
token_bucket & tcp_transport::token_bucket_read()
{
    return g_token_bucket_read;
}

token_bucket & tcp_transport::token_bucket_write()
{
    return g_token_bucket_write;
}
#endif // USE_TOKEN_BUCKET

void tcp_transport::do_connect(const asio::ip::tcp::endpoint & ep)
{
    auto self(shared_from_this());
    
    m_state = state_connecting;

    m_socket->lowest_layer().async_connect(ep,
        m_strand.wrap([this, self](std::error_code ec)
    {
        if (ec)
        {
            if (m_on_complete)
            {
                m_on_complete(ec, self);
            }
            
            /**
             * Stop
             */
            stop();
        }
        else
        {
            connect_timeout_timer_.cancel();
            
            m_state = state_connected;
            
            if (m_on_complete)
            {
                m_on_complete(ec, self);
            }
    
            if (write_queue_.size() > 0)
            {
                do_write(
                    &write_queue_.front()[0], write_queue_.front().size()
                );
            }
            
            do_read();
        }
    }));
}

void tcp_transport::do_connect(
    asio::ip::tcp::resolver::iterator endpoint_iterator
    )
{
    auto self(shared_from_this());
    
    m_state = state_connecting;
    
    asio::async_connect(m_socket->lowest_layer(), endpoint_iterator,
        m_strand.wrap([this, self](std::error_code ec,
        asio::ip::tcp::resolver::iterator)
    {
        if (ec)
        {
            if (m_state != state_disconnected)
            {
                /**
                 * Stop
                 */
                 stop();
                
                if (m_on_complete)
                {
                    m_on_complete(ec, self);
                }
            }
        }
        else
        {
            connect_timeout_timer_.cancel();
            
            m_state = state_connected;
            
            if (m_on_complete)
            {
                m_on_complete(ec, self);
            }
    
            if (write_queue_.size() > 0)
            {
                do_write(
                    &write_queue_.front()[0], write_queue_.front().size()
                );
            }
            
            do_read();
        }
    }));
}

void tcp_transport::do_read()
{
    if (m_state == state_connected)
    {
        auto self(shared_from_this());

#if (defined USE_TOKEN_BUCKET && USE_TOKEN_BUCKET)
        auto should_read =
            g_token_bucket_read.is_enabled() &&
            g_token_bucket_read.try_to_consume(sizeof(read_buffer_))
        ;
#else
        auto should_read = true;
#endif // USE_TOKEN_BUCKET

        if (should_read == true)
        {
            if (m_read_timeout > 0)
            {
                read_timeout_timer_.expires_from_now(
                    std::chrono::seconds(m_read_timeout)
                );
                read_timeout_timer_.async_wait(m_strand.wrap(
                    [this, self](std::error_code ec)
                {
                    if (ec)
                    {
                        // ...
                    }
                    else
                    {
                        log_debug("TCP transport receive timed out, closing.");
                        
                        /**
                         * Stop
                         */
                         stop();
                    }
                }));
            }
            
            /**
             * Set the time last read.
             */
            m_interval_last_read =
                std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::system_clock::now().time_since_epoch()
            );
            
            m_socket->async_read_some(asio::buffer(read_buffer_),
                m_strand.wrap([this, self](std::error_code ec,
                std::size_t len)
            {
                if (ec)
                {
                    log_debug(
                        "TCP transport read error, message = " <<
                        ec.message() << ", closing."
                    );

                    /**
                     * Stop.
                     */
                    stop();
                }
                else
                {
                    /**
                     * Update the total bytes read.
                     */
                    m_bytes_total_read += len;
                    
                    /**
                     * Update the total bytes read (during the one second
                     * interval).
                     */
                    m_bytes_total_interval_read += len;
                    
                    /**
                     * Calculate read bandwidth.
                     */
                    
                    auto time_diff =
                        std::chrono::duration_cast<std::chrono::milliseconds> (
                        std::chrono::system_clock::now().time_since_epoch()) -
                        m_interval_last_read
                    ;
                    
                    if (time_diff.count() > 0)
                    {
                        m_bytes_per_second_read =
                            m_bytes_total_interval_read / time_diff.count()
                        ;
       
                        if (time_diff.count() >= 1000)
                        {
                            /**
                             * Set the time last read.
                             */
                            m_interval_last_read =
                                std::chrono::duration_cast<
                                std::chrono::milliseconds> (
                                std::chrono::system_clock::now(
                                ).time_since_epoch()
                            );
                            
                            time_diff =
                                std::chrono::duration_cast<
                                std::chrono::milliseconds> (
                                std::chrono::system_clock::now(
                                ).time_since_epoch()) - m_interval_last_read
                            ;
                            
                            m_bytes_total_interval_read = 0;
                        }
                    }
        
                    read_timeout_timer_.cancel();
                    
                    /**
                     * Callback
                     */
                    if (m_on_read)
                    {
                        try
                        {
                            m_on_read(self, read_buffer_, len);
                        }
                        catch (std::exception & e)
                        {
                            log_error(
                                "TCP transport on_read callback failed, "
                                "what = " << e.what() << "."
                            );
                        }
                    }
                    
                    do_read();
                }
            }));
        }
        else
        {
#if (defined USE_TOKEN_BUCKET && USE_TOKEN_BUCKET)

            log_debug("TCP transport will retry reading.");
            
            read_retry_timer_.expires_from_now(std::chrono::seconds(1));
            read_retry_timer_.async_wait(m_strand.wrap(
                [this, self](std::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    do_read();
                }
            }));
            
#endif // USE_TOKEN_BUCKET
        }
    }
}

void tcp_transport::do_write(const char * buf, const std::size_t & len)
{
    if (m_state == state_connected)
    {
        auto self(shared_from_this());

#if (defined USE_TOKEN_BUCKET && USE_TOKEN_BUCKET)
        auto should_write =
            g_token_bucket_write.is_enabled() &&
            g_token_bucket_write.try_to_consume(len)
        ;
#else
        auto should_write = true;
#endif // USE_TOKEN_BUCKET

        if (should_write == true)
        {
            if (m_write_timeout > 0)
            {
                write_timeout_timer_.expires_from_now(
                    std::chrono::seconds(m_write_timeout)
                );
                write_timeout_timer_.async_wait(m_strand.wrap(
                    [this, self](std::error_code ec)
                {
                    if (ec)
                    {
                        // ...
                    }
                    else
                    {
                        log_debug("TCP transport write timed out, closing.");
                        
                        /**
                         * Stop
                         */
                         stop();
                    }
                }));
            }

            /**
             * Set the time last write.
             */
            m_interval_last_write =
                std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::system_clock::now().time_since_epoch()
            );
            
            asio::async_write(*m_socket, asio::buffer(buf, len),
                m_strand.wrap([this, self](std::error_code ec,
                std::size_t bytes_transferred)
            {
                if (ec)
                {
                    /**
                     * Stop
                     */
                     stop();
                }
                else
                {
                    /**
                     * Update the total bytes written.
                     */
                    m_bytes_total_write += bytes_transferred;
                    
                    /**
                     * Update the total bytes written (during the one second
                     * interval).
                     */
                    m_bytes_total_interval_write += bytes_transferred;
            
                    /**
                     * Calculate write bandwidth.
                     */
                    
                    auto time_diff =
                        std::chrono::duration_cast<std::chrono::milliseconds> (
                        std::chrono::system_clock::now().time_since_epoch()) -
                        m_interval_last_write
                    ;
                    
                    if (time_diff.count() > 0)
                    {
                        m_bytes_per_second_write =
                            m_bytes_total_interval_write / time_diff.count()
                        ;
       
                        if (time_diff.count() >= 1000)
                        {
                            /**
                             * Set the time last write.
                             */
                            m_interval_last_write =
                                std::chrono::duration_cast<
                                std::chrono::milliseconds> (
                                std::chrono::system_clock::now(
                                ).time_since_epoch()
                            );
                            
                            time_diff =
                                std::chrono::duration_cast<
                                std::chrono::milliseconds> (
                                std::chrono::system_clock::now(
                                ).time_since_epoch()
                                ) - m_interval_last_write
                            ;
                            
                            m_bytes_total_interval_write = 0;
                        }
                    }
                    
                    write_timeout_timer_.cancel();
                    
                    write_queue_.pop_front();
                    
                    if (write_queue_.size() == 0)
                    {
                        if (m_close_after_writes)
                        {
                            log_debug(
                                "TCP transport write queue is empty, closing."
                            );
                            
                            /**
                             * Stop
                             */
                             stop();
                        }
                    }
                    else
                    {
                        do_write(
                            &write_queue_.front()[0],
                            write_queue_.front().size()
                        );
                    }
                }
            }));
        }
        else
        {
#if (defined USE_TOKEN_BUCKET && USE_TOKEN_BUCKET)

            log_debug("TCP transport will retry writing, len = " << len << ".");
            
            write_retry_timer_.expires_from_now(std::chrono::seconds(1));
            write_retry_timer_.async_wait(m_strand.wrap(
                [this, self](std::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    if (write_queue_.size() > 0)
                    {
                        /**
                         * Break the packet into chunks until there is none
                         * left to write.
                         */
                        auto bytes = write_queue_.front();
                        
                        /**
                         * Get the rate.
                         */
                        auto rate = g_token_bucket_write.rate();
                        
                        if (bytes.size() > rate)
                        {
                            bytes.resize(rate);
                            
                            write_queue_.front().erase(
                                write_queue_.front().begin(),
                                write_queue_.front().begin() + rate
                            );
                        }
                    
                        write_queue_.push_front(bytes);
                        
                        do_write(
                            &write_queue_.front()[0],
                            write_queue_.front().size()
                        );
                    }
                }
            }));
#endif // USE_TOKEN_BUCKET
        }
    }
}

void tcp_transport::tick(const std::error_code & ec)
{
    if (ec)
    {
        // ...
    }
    else
    {
        if (m_state == state_connecting || m_state == state_connected)
        {
            log_debug(
                "TCP Transport wrote " << m_bytes_total_write <<
                " total bytes, bytes per second = " <<
                m_bytes_per_second_write << ", last write = " <<
                time_last_write() << "."
            );
            
            /**
             * Calculate write bandwidth.
             */
            
            auto time_diff =
                std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::system_clock::now().time_since_epoch()) -
                m_interval_last_write
            ;
            
            if (time_diff.count() > 0)
            {
                m_bytes_per_second_write =
                    m_bytes_total_interval_write / time_diff.count()
                ;
            }
            
            log_debug(
                "TCP Transport read " << m_bytes_total_read <<
                " total bytes, bytes per second = " <<
                m_bytes_per_second_read << ", last read = " <<
                time_last_read() << "."
            );
            
            /**
             * Calculate read bandwidth.
             */
            
            time_diff =
                std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::system_clock::now().time_since_epoch()) -
                m_interval_last_read
            ;
            
            if (time_diff.count() > 0)
            {
                m_bytes_per_second_read =
                    m_bytes_total_interval_read / time_diff.count()
                ;
            }

            auto self(shared_from_this());
            
            /**
             * Calculate bandwidth over an 8 second moving window.
             */
            timer_.expires_from_now(std::chrono::seconds(8));
            timer_.async_wait(m_strand.wrap(
                std::bind(&tcp_transport::tick, self, std::placeholders::_1))
            );
        }
    }
}

int tcp_transport::run_test()
{
    asio::io_service ios;
    
    std::shared_ptr<tcp_transport> t =
        std::make_shared<tcp_transport>(ios)
    ;
    
    /**
     * Set the transport on read handler.
     */
    t->set_on_read(
        [](std::shared_ptr<tcp_transport> t,
        const char * buf, const std::size_t & len)
    {
        std::cout <<
            "tcp_transport read " << len << " bytes, buffer = " <<
            buf <<
        std::endl;
    });
    
    t->start("google.com", 80,
        [](std::error_code ec, std::shared_ptr<tcp_transport> t)
    {
        if (ec)
        {
            std::cerr <<
                "tcp_transport connect failed, message = " <<
                ec.message() <<
            std::endl;
        }
        else
        {
            std::cout <<
                "tcp_transport connect success" <<
            std::endl;
            
            std::stringstream ss;
            
            ss << "GET" << " "  << "/" << " HTTP/1.1\r\n";
            ss << "Host: " << "google.com" << "\r\n";
            ss << "Accept: */*\r\n";
            ss << "Connection: close\r\n";
            ss << "\r\n";
            
            /**
             * Preform the write operation.
             */
            t->write(ss.str().data(), ss.str().size());
        }
    });
    
    ios.run();

    return 0;
}

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

#define USE_TOKEN_BUCKET 0

#include <cstdint>
#include <chrono>
#include <deque>
#include <memory>

#define ASIO_STANDALONE 1

#include <asio.hpp>

#if (defined USE_TOKEN_BUCKET && USE_TOKEN_BUCKET)
#include <opensentinel/token_bucket.hpp>
#endif // USE_TOKEN_BUCKET

namespace opensentinel {

    /**
     * Implements a tcp transport.
     */
    class tcp_transport
        : public std::enable_shared_from_this<tcp_transport>
    {
        public:
        
            /**
             * The states.
             */
            typedef enum
            {
                state_disconnected,
                state_connecting,
                state_connected,
            } state_t;

            /**
             * Constructor
             * @param ios The asio::io_service.
             */
            tcp_transport(asio::io_service & ios);
        
            /**
             * Destructor
             */
            ~tcp_transport();

            /**
             * Starts the transport (outgoing).
             * f The completion handler.
             */
            void start(
                const std::string & hostname, const std::uint16_t & port,
                const std::function<void (std::error_code,
                std::shared_ptr<tcp_transport>)> & f
            );
        
            /**
             * Starts the transport (incoming).
             */
            void start();
        
            /**
             * Stops the transport.
             */
            void stop();
        
            /**
             * Sets the on read handler.
             * @param f the std::function.
             */
            void set_on_read(
                const std::function<void (std::shared_ptr<tcp_transport>,
                const char *, const std::size_t &)> & f
            );
        
            /**
             * Performs a write operation.
             * @param buf The buffer.
             * @param len The length.
             */
            void write(const char * buf, const std::size_t & len);
        
            /**
             * The asio::strand.
             */
            asio::strand & strand();
        
            /**
             * The state.
             */
            state_t & state();
        
            /**
             * Sets the identifier.
             * @param val The value.
             */
            void set_identifier(const std::string & val);
        
            /**
             * The identifier.
             */
            const std::string & identifier() const;
        
            /**
             * The socket.
             */
            asio::ip::tcp::socket & socket();

            /**
             * If true the conneciton will close as soon as it's write queue is
             * exhausted.
             * @param flag The flag.
             */
            void set_close_after_writes(const bool & flag);
        
            /**
             * Sets the read timeout.
             * @param val The value.
             */
            void set_read_timeout(const std::uint32_t & val);
        
            /**
             * Sets the write timeout.
             * @param val The value.
             */
            void set_write_timeout(const std::uint32_t & val);
        
            /**
             * The time of the last read.
             */
            const std::time_t time_last_read();
        
            /**
             * The time of the last write.
             */
            const std::time_t time_last_write();
        
            /**
             * The total bytes read.
             */
            const std::size_t & bytes_total_read() const;
        
            /**
             * The total bytes written.
             */
            const std::size_t & bytes_total_write() const;
        
            /**
             * The number of bytes read per second.
             */
            const std::size_t & bytes_per_second_read() const;
        
            /**
             * The number of bytes write per second.
             */
            const std::size_t & bytes_per_second_write() const;
        
#if (defined USE_TOKEN_BUCKET && USE_TOKEN_BUCKET)
            /**
             * The write token_bucket.
             */
            static token_bucket & token_bucket_read();
        
            /**
             * The read token_bucket.
             */
            static token_bucket & token_bucket_write();
#endif // USE_TOKEN_BUCKET
            /**
             * Runs the test case.
             */
            static int run_test();
        
        private:
        
            /**
             * do_connect
             */
            void do_connect(const asio::ip::tcp::endpoint & ep);
        
            /**
             * do_connect
             */
            void do_connect(asio::ip::tcp::resolver::iterator);
        
            /**
             * do_read
             */
            void do_read();
        
            /**
             * do_write
             */
            void do_write(const char * buf, const std::size_t & len);
        
            /**
             * The timer handler.
             * @param ec The std::error_code.
             */
            void tick(const std::error_code & ec);
        
            /**
             * The asio::strand.
             */
            asio::strand m_strand;
        
            /**
             * The identifier.
             */
            std::string m_identifier;
        
            /**
             * The state.
             */
            state_t m_state;

            /**
             * The asio::ip::tcp::socket.
             */
            std::shared_ptr<asio::ip::tcp::socket> m_socket;

            /**
             * If true the conneciton will close as soon as it's write queue is
             * exhausted.
             */
            bool m_close_after_writes;
        
            /**
             * The read timeout.
             */
            std::uint32_t m_read_timeout;
        
            /**
             * The write timeout.
             */
            std::uint32_t m_write_timeout;
  
            /**
             * The time of the last read.
             */
            std::chrono::milliseconds m_interval_last_read;
        
            /**
             * The time of the last write.
             */
            std::chrono::milliseconds m_interval_last_write;
        
            /**
             * The total bytes read.
             */
            std::size_t m_bytes_total_read;
        
            /**
             * The total bytes written.
             */
            std::size_t m_bytes_total_write;
        
            /**
             * The total bytes (interval) read.
             * @note Interval is one second (bytes per second).
             */
            std::size_t m_bytes_total_interval_read;
        
            /**
             * The total bytes (interval) written.
             * @note Interval is one second (bytes per second).
             */
            std::size_t m_bytes_total_interval_write;
        
            /**
             * The number of bytes read per second.
             */
            std::size_t m_bytes_per_second_read;
        
            /**
             * The number of bytes write per second.
             */
            std::size_t m_bytes_per_second_write;
    
            /**
             * The completion handler.
             */
            std::function<
                void (std::error_code, std::shared_ptr<tcp_transport>)
            > m_on_complete;
        
            /**
             * The read handler.
             */
            std::function<
                void (std::shared_ptr<tcp_transport>, const char *,
                const std::size_t &)
            > m_on_read;
        
        protected:
        
            /**
             * The asio::io_service.
             */
            asio::io_service & io_service_;

            /**
             * The timer.
             */
            asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_;
        
            /**
             * The connect timeout timer.
             */
            asio::basic_waitable_timer<
                std::chrono::steady_clock
            > connect_timeout_timer_;
        
            /**
             * The read timeout timer.
             */
            asio::basic_waitable_timer<
                std::chrono::steady_clock
            > read_timeout_timer_;
        
            /**
             * The write timeout timer.
             */
            asio::basic_waitable_timer<
                std::chrono::steady_clock
            > write_timeout_timer_;
        
            /**
             * The write queue.
             */
            std::deque< std::vector<char> > write_queue_;

            /**
             * The read buffer.
             */
            char read_buffer_[8192];

#if (defined USE_TOKEN_BUCKET && USE_TOKEN_BUCKET)
            /**
             * The write token_bucket.
             */
            static token_bucket g_token_bucket_read;
        
            /**
             * The read token_bucket.
             */
            static token_bucket g_token_bucket_write;
        
            /**
             * The read retry timer.
             */
            asio::basic_waitable_timer<
                std::chrono::steady_clock
            > read_retry_timer_;
        
            /**
             * The write retry timer.
             */
            asio::basic_waitable_timer<
                std::chrono::steady_clock
            > write_retry_timer_;
#endif // USE_TOKEN_BUCKET
    };
    
} // namespace opensentinel


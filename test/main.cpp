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

#define ASIO_STANDALONE 1

#include <asio.hpp>

#include <opensentinel/stack.hpp>

#define PERFORM_TESTS 0

#if (defined PERFORM_TESTS && PERFORM_TESTS)
#include <opensentinel/tcp_acceptor.hpp>
#include <opensentinel/tcp_transport.hpp>
#endif // PERFORM_TESTS
int main(int argc, const char * argv[])
{
#if (defined PERFORM_TESTS && PERFORM_TESTS)
    int ret = 0;
    
    /**
     * The asio::io_service.
     */
    asio::io_service ios_tcp_acceptor;
    
    ret |= opensentinel::tcp_acceptor::run_test(ios_tcp_acceptor);
    
    ios_tcp_acceptor.run();
    
    ret |= opensentinel::tcp_transport::run_test();
    
    return ret;
#endif // PERFORM_TESTS
    
    /**
     * Allocate the opensentinel::stack.
     */
    opensentinel::stack opensentinel_stack;

    /**
     * Start the opensentinel::stack.
     */
    opensentinel_stack.start();
    
    /**
     * The asio::io_service that waits on the asio::signal_set.
     */
    asio::io_service ios;
    
    /**
     * Set asio::signal_set.
     */
    asio::signal_set signals(ios, SIGINT, SIGTERM);
    
    /**
     * Wait for termination.
     */
    signals.async_wait(std::bind(&asio::io_service::stop, &ios));
    
    /**
     * Run the asio::io_service.
     */
    ios.run();
    
    /**
     * Stop the opensentinel::stack.
     */
    opensentinel_stack.stop();
    
    return 0;
}

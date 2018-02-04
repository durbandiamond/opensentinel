# Open Sentinel

Open Sentinel mitigates chance by extraordinarily decreasing the time a hacker goes undetected in secure networks.

The framework, made out of a passive device associated with each subnet, screens network traffic for suspicious action's such as ICMP (Ping) Scans, TCP Half-Open Scans, TCP Connect Scans, UDP Scans and so on, producing an alert and conveying countermeasures when activated.

It is coded in C++11 and runs on most platforms and devices.

## Building

Although Boost is not used it is required for the Boost.Build system (bjam). As C++17 is being supported on more compilers we will be adopting std::filesystem, etc instead of using Boost moving forward. Asio will continue to be used until C++20 which will include native support.

 * Linux - git pull and run ./build_linux.sh

 * MacOS - git pull and run ./build_macos.sh

## Using

Open Sentinel uses the `system` call to execute a user defined script when a threat is detected. The examples directory contains a script that works with the [pushd](https://pushed.co) service.

Open Sentinel MUST be run as root on Posix compliant systems and Administrator on Windows systems.

To test your Open Sentinel setup simply point your favorite `LAN scanner` at it or send a UDP packet(`echo -n "hello" >/dev/udp/192.168.1.16/8100`) or connect with your `web browser` to one of the passive ports such as 8100.

## Collaboration

All contributions, big or small, are welcome. You are welcome to contribute to this project with whatever level of contribution you are comfortable with. We have no expectations for the amount or frequency of contributions from anyone.

We want to share ownership and responsibility with the community where possible. To help with this we hand out *write access* when we deem pull requests consistently of sufficient quality.

## License

See the [LICENSE](LICENSE) file. Usage of any API keys found in the source is not allowed for other purposes than described in the source code and/or its documentation. You must at all times use your own API keys.

## Website

[opensentinel.org](http://opensentinel.org)



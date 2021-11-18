#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

using tcp = boost::asio::ip::tcp;

void log(const boost::system::error_code& ec) {
    std::cout << '[' << std::setw(14) << std::this_thread::get_id() << "] "
        << (ec ? "Error: " : "OK")
        << (ec ? ec.message() : "")
        << '\n';
}

void onConnect(const boost::system::error_code& ec) {
    log(ec);
}

int main() {
    std::cout << '[' << std::setw(14) << std::this_thread::get_id() << "] main\n";

    boost::asio::io_context ioc { };
    tcp::socket socket { boost::asio::make_strand(ioc) };

    constexpr std::size_t nThreads { 4 };

    boost::system::error_code ec;
    tcp::resolver resolver { ioc };
    auto resolveIt { resolver.resolve("www.google.com", "80", ec) };
    if (ec) {
        log(ec);
        return -1;
    }
    for (std::size_t idx { 0 }; idx < nThreads; ++idx)
        socket.async_connect(*resolveIt, onConnect);

    std::vector<std::thread> threads { };
    threads.reserve(nThreads);
    for (std::size_t idx { 0 }; idx < nThreads; ++idx) {
        threads.emplace_back([&ioc]() {
            ioc.run();
        });
    }
    for (std::size_t idx { 0 }; idx < nThreads; ++idx)
        threads[idx].join();

    return 0;
}
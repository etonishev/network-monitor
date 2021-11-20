#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

using tcp = boost::asio::ip::tcp;

#include <boost/beast.hpp>



void log(const boost::system::error_code& ec) {

    std::cerr << (ec ? "Error: " : "OK")
        << (ec ? ec.message() : "")
        << '\n';

}

int main() {

    const std::string host = "ltnm.learncppthroughprojects.com";
    const std::string port = "80";

    boost::asio::io_context ioc {};
    tcp::socket socket { ioc };

    boost::system::error_code ec {};
    tcp::resolver resolver { ioc };

    auto resolveIt { resolver.resolve(host, port, ec) };
    if (ec) {
        log(ec);
        return -1;
    }

    socket.connect(*resolveIt, ec);
    if (ec) {
        log(ec);
        return -1;
    }

    boost::beast::websocket::stream<boost::beast::tcp_stream> ws { std::move(socket) };
    ws.handshake(host, "/echo", ec);
    if (ec) {
        log(ec);
        return -1;
    }
    ws.text(true);
    const std::string message { "Hello, world!" };
    ws.write(boost::asio::buffer(message.c_str(), message.size()), ec);
    if (ec) {
        log(ec);
        return -1;
    }

    boost::beast::flat_buffer buffer;
    ws.read(buffer, ec);
    if (ec) {
        log(ec);
        return -1;
    }

    std::cout << "Response: " << boost::beast::make_printable(buffer.data()) << '\n';

    return 0;

}
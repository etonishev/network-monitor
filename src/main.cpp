#include <iomanip>
#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

using tcp = boost::asio::ip::tcp;
using error_code = boost::system::error_code;

#include <boost/beast.hpp>

namespace beast = boost::beast;
namespace websocket = beast::websocket;


void log(const std::string& where, const error_code& ec) {

    std::cerr << '[' << std::setw(20) << where << "] "
        << (ec ? "Error: " : "OK")
        << (ec ? ec.message() : "")
        << '\n';

}

void onReceive(
    // --> Start of shared data
    beast::flat_buffer& rBuffer,
    // <-- End of shared data
    const error_code& ec
)
{
    if (ec) {
        log("onReceive", ec);
        return;
    }

    std::cout << "ECHO: " << beast::make_printable(rBuffer.data()) << '\n';
}

void onSend(
    // --> Start of shared data
    websocket::stream<beast::tcp_stream>& ws,
    beast::flat_buffer& rBuffer,
    // <-- End of shared data
    const error_code& ec
)
{
    if (ec) {
        log("onSend", ec);
        return;
    }

    ws.async_read(rBuffer, [&rBuffer](auto ec, auto nBytes) {
        onReceive(rBuffer, ec);
    });
}

void onHandshake(
    // --> Start of shared data
    websocket::stream<beast::tcp_stream>& ws,
    const boost::asio::const_buffer& wBuffer,
    beast::flat_buffer& rBuffer,
    // <-- End of shared data
    const error_code& ec
)
{
    if (ec) {
        log("onHandshake", ec);
        return;
    }

    ws.text(true);

    ws.async_write(wBuffer, [&ws, &rBuffer](auto ec, auto nBytes) {
        onSend(ws, rBuffer, ec);
    });
}

void onConnect(
    // --> Start of shared data
    websocket::stream<beast::tcp_stream>& ws,
    const std::string& host,
    const std::string& target,
    const boost::asio::const_buffer& wBuffer,
    beast::flat_buffer& rBuffer,
    // <-- End of shared data
    const error_code& ec
)
{
    if (ec) {
        log("onConnect", ec);
        return;
    }

    ws.async_handshake(host, target, [&ws, &wBuffer, &rBuffer](auto ec) {
        onHandshake(ws, wBuffer, rBuffer, ec);
    });
}

void onResolve(
    // --> Start of shared data
    websocket::stream<beast::tcp_stream>& ws,
    const std::string& host,
    const std::string& target,
    const boost::asio::const_buffer& wBuffer,
    beast::flat_buffer& rBuffer,
    // <-- End of shared data
    const error_code& ec,
    tcp::resolver::results_type results
)
{
    if (ec) {
        log("onResolve", ec);
        return;
    }

    ws.next_layer().async_connect(*results, [&ws, &host, &target, &wBuffer, &rBuffer](auto ec) {
        onConnect(ws, host, target, wBuffer, rBuffer, ec);
    });
}




int main() {

    const std::string host { "ltnm.learncppthroughprojects.com" };
    const std::string port { "80" };
    const std::string target { "/echo" };
    const std::string message { "Hello, world!" };

    boost::asio::io_context ioc {};
    websocket::stream<beast::tcp_stream> ws { ioc };

    boost::asio::const_buffer wBuffer { message.data(), message.length() };
    beast::flat_buffer rBuffer;

    tcp::resolver resolver { ioc };
    resolver.async_resolve(host, port, [&ws, &host, &target, &wBuffer, &rBuffer](auto ec, auto results) {
        onResolve(ws, host, target, wBuffer, rBuffer, ec, results);
    });

    ioc.run();

    return 0;

}
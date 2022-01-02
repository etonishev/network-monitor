#include <string>
#include "boost\asio.hpp"
#include "boost\beast.hpp"



namespace NetworkMonitor {

class WebSocketClient {
public:
    WebSocketClient(
        const std::string& url,
        const std::string& endpoint,
        const std::string& port,
        boost::asio::io_context& ioc
    );

    ~WebSocketClient();

    void Connect(
        std::function<void (boost::system::error_code)> onConnect = nullptr,
        std::function<void (boost::system::error_code, std::string&&)> onMessage = nullptr,
        std::function<void (boost::system::error_code)> onDisconnect = nullptr
    );

    void Send(
        const std::string& message,
        std::function<void (boost::system::error_code)> onSend = nullptr
    );

    void Close(
        std::function<void (boost::system::error_code)> onClose = nullptr
    );

private:
    void OnResolve(const boost::system::error_code& ec, const boost::asio::ip::tcp::resolver::iterator& resolverIt);
    void OnConnect(const boost::system::error_code& ec);
    void OnHandshake(const boost::system::error_code& ec);
    void OnRead(const boost::system::error_code& ec, std::size_t nBytes);
    void ListenToIncomingMessage(const boost::system::error_code& ec);

private:
    std::string m_url {};
    std::string m_endpoint {};
    std::string m_port {};
    bool m_closed { false };

    boost::asio::ip::tcp::resolver m_resolver;
    boost::beast::websocket::stream<boost::beast::tcp_stream> m_ws;

    boost::beast::flat_buffer m_buffer {};

    std::function<void (const boost::system::error_code& ec)> m_onConnect { nullptr };
    std::function<void (const boost::system::error_code& ec)> m_onDisconnect { nullptr };
    std::function<void (const boost::system::error_code& ec, std::string&& message)> m_onMessage { nullptr };
};

} // namespace NetworkMonitor

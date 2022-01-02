#include <iostream>
#include <iomanip>

#include "websocket-client.h"

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

static void log(const std::string& where, const boost::system::error_code& ec) {
    std::cerr << '[' << std::setw(20) << where << "] "
        << (ec ? "Error: " : "OK")
        << (ec ? ec.message() : "")
        << '\n';
}

NetworkMonitor::WebSocketClient::WebSocketClient
(
    const std::string& url,
    const std::string& endpoint,
    const std::string& port,
    boost::asio::io_context& ioc
) : m_url { url }, 
    m_endpoint { endpoint }, 
    m_port { port },
    m_resolver { boost::asio::make_strand(ioc) }, 
    m_ws { boost::asio::make_strand(ioc) } 
{}

NetworkMonitor::WebSocketClient::~WebSocketClient
(
) 
{}

void NetworkMonitor::WebSocketClient::Connect
(
    std::function<void (boost::system::error_code)> onConnect,
    std::function<void (boost::system::error_code, std::string&&)> onMessage,
    std::function<void (boost::system::error_code)> onDisconnect
) {
    m_onConnect = onConnect;
    m_onDisconnect = onDisconnect;
    m_onMessage = onMessage;

    m_closed = false;
    m_resolver.async_resolve(m_url, m_port, [this](const auto& ec, const auto& resultrs) {
        OnResolve(ec, resultrs);
    });
}

void NetworkMonitor::WebSocketClient::Send
(
    const std::string& message,
    std::function<void (boost::system::error_code)> onSend
) {
    m_ws.async_write(boost::asio::buffer(message), [onSend](const auto& ec, const auto&) {
        if (onSend) {
            onSend(ec);
        }
    });
}

void NetworkMonitor::WebSocketClient::Close
(
    std::function<void (boost::system::error_code)> onClose
) {
    m_closed = true;
    m_ws.async_close(websocket::close_code::normal, [onClose](const auto& ec) {
        if (onClose) {
            onClose(ec);
        }
    });
}

void NetworkMonitor::WebSocketClient::OnResolve(
    const boost::system::error_code& ec,
    const boost::asio::ip::tcp::resolver::iterator& resolverIt
) {
    if (ec) {
        log("OnResolve", ec);
        if (m_onConnect) {
            m_onConnect(ec);
        }
        return;
    }

    m_ws.next_layer().expires_after(std::chrono::seconds(5));

    m_ws.next_layer().async_connect(*resolverIt, [this](const auto& ec) {
        OnConnect(ec);
    });
}

void NetworkMonitor::WebSocketClient::OnConnect(
    const boost::system::error_code& ec 
) {
    if (ec) {
        log("OnConnect", ec);
        if (m_onConnect) {
            m_onConnect(ec);
        }
        return;
    }

    m_ws.next_layer().expires_never();
    m_ws.set_option(websocket::stream_base::timeout::suggested(
        boost::beast::role_type::client
    ));

    m_ws.async_handshake(m_url, m_endpoint, [this](const auto& ec) {
        OnHandshake(ec);
    });
}

void NetworkMonitor::WebSocketClient::OnHandshake(
    const boost::system::error_code& ec
) {
    if (ec) {
        log("OnHandshake", ec);
        if (m_onConnect) {
            m_onConnect(ec);
        }
        return;
    }

    m_ws.text(true);
    ListenToIncomingMessage(ec);

    if (m_onConnect) {
        m_onConnect(ec);
    }
}

void NetworkMonitor::WebSocketClient::ListenToIncomingMessage(
    const boost::system::error_code& ec 
) {
    if (ec == boost::asio::error::operation_aborted) {
        if (m_onDisconnect && !m_closed) {
            m_onDisconnect(ec);
        }

        return;
    }

    m_ws.async_read(m_buffer, [this](const auto& ec, const auto& nBytes) {
        OnRead(ec, nBytes);
        ListenToIncomingMessage(ec);
    });
}

void NetworkMonitor::WebSocketClient::OnRead(
    const boost::system::error_code& ec,
    std::size_t nBytes
) {
    if (ec) {
        return;
    }

    std::string message{boost::beast::buffers_to_string(m_buffer.data())};
    m_buffer.consume(nBytes);

    if (m_onMessage) {
        m_onMessage(ec, std::move(message));
    }
}
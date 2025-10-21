#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class WebSocketClient {
public:
    WebSocketClient(const std::string& host, const std::string& port)
        : resolver_(ioc_), ws_(ioc_) {
        // Resolve the host and port
        auto const results = resolver_.resolve(host, port);
        // Connect to the resolved endpoint
        auto ep = net::connect(ws_.next_layer(), results);
        // Update the host_ string with the resolved host and port
        host_ = host + ':' + std::to_string(ep.port());
        // Perform the WebSocket handshake
        ws_.handshake(host_, "/");
    }

    ~WebSocketClient() {
        // Close the WebSocket connection
        ws_.close(websocket::close_code::normal);
    }

    void send_string(const std::string& message) {
        // Send the message over the WebSocket connection
        ws_.write(net::buffer(message));
    }

private:
    net::io_context ioc_;
    tcp::resolver resolver_;
    websocket::stream<tcp::socket> ws_;
    std::string host_;
};

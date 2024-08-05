#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>
#include <string>
#include <thread>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;   // from <boost/asio/ip/tcp.hpp>


// Accepts incoming connections and launches the sessions
class listener
{
public:
    // Echoes back all received WebSocket messages
    class session
    {
    public:
        // Define a function pointer type
        typedef void (*CB_Receive)(session* pSession, std::string content);

        // Take ownership of the socket
        session(tcp::socket socket, CB_Receive fncReceive = 0x0);
        ~session();

        void stop();
        void send(std::string content);
        void setCBReceive(CB_Receive fncReceive);

    protected:
        void on_accept(beast::error_code ec);
        void do_read();
        void on_read(beast::error_code ec, std::size_t bytes_transferred);
        void on_write(beast::error_code ec, std::size_t bytes_transferred);
        void fail(beast::error_code ec, char const* what);

    private:
        websocket::stream<beast::tcp_stream>* ws_;
        beast::flat_buffer buffer_;
        CB_Receive onReceive;
    };

public:
    typedef void (*CB_Accept)(session* pSession);

    listener(CB_Accept fncAccept = 0x0);
    ~listener();

    std::string run(int defaultPort);
    void setCBAccept(CB_Accept fnc);

    int getPort() { return port_; }

protected:
    std::string tryPort(int port);
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
    std::string fail(beast::error_code ec, char const* what);

private:
    net::io_context ioc_;
    tcp::acceptor acceptor_;
    tcp::endpoint endpoint_;
    int port_;
    std::vector<session*> sessions_;
    std::vector<std::thread> vThreads;
    CB_Accept onAccept;
};
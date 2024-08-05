#define STRICT
#include "WebSocket.h"

// Take ownership of the socket
listener::session::session(tcp::socket socket, CB_Receive fncReceive)
    : ws_(0x0)
    , onReceive(fncReceive)
{
    ws_ = new websocket::stream<beast::tcp_stream>(std::move(socket));
    // Accept the WebSocket handshake
    ws_->async_accept(
        beast::bind_front_handler(&session::on_accept, this)
    );
}

listener::session::~session()
{
    this->stop();
}

void listener::session::stop()
{
    if (ws_ != 0x0)
    {
        delete ws_;
        ws_ = 0x0;
    }
}

void listener::session::send(std::string content)
{
    // Prepare a buffer with the specified message
    boost::beast::flat_buffer send_buffer;
    boost::beast::ostream(send_buffer) << content;
    // Echo the message
    ws_->text(true);
    ws_->async_write(
        send_buffer.data(),
        beast::bind_front_handler(&session::on_write, this)
    );
}

void listener::session::setCBReceive(CB_Receive fncReceive)
{
    this->onReceive = fncReceive;
}

void listener::session::on_accept(beast::error_code ec)
{
    if (ec) return fail(ec, "accept");

    do_read();
}

void listener::session::do_read()
{
    // Read a message into our buffer
    ws_->async_read(
        buffer_,
        beast::bind_front_handler(&session::on_read, this)
    );
}

void listener::session::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    // This indicates that the session was closed
    if (ec == websocket::error::closed)
    {
        return;
    }
    else
    {
        if (ec) fail(ec, "read");
        // Convert buffer to string
        std::string received_message = beast::buffers_to_string(buffer_.data());
        if (onReceive != 0x0) onReceive(this, received_message);
    }
}

void listener::session::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec) return fail(ec, "write");
    // Clear the buffer
    buffer_.consume(buffer_.size());

    do_read();
}

void listener::session::fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

//================================================================

listener::listener(CB_Accept fncAccept)
    : acceptor_(tcp::acceptor(net::make_strand(ioc_)))
    , port_(0)
    , onAccept(fncAccept)
{
}

listener::~listener()
{
    acceptor_.close();
    ioc_.stop();
    // Block until all the threads exit
    if (vThreads.size() > 0)
    {
        for (auto& t : vThreads)
            t.join();
        vThreads.clear();
    }
}

std::string listener::tryPort(int port)
{
    std::string errMsg;
    boost::asio::ip::port_type tmpPort = port;
    tcp::endpoint tmpEndpoint = tcp::endpoint{ tcp::v4(), tmpPort };
    tcp::acceptor tmpAcceptor = tcp::acceptor(net::make_strand(ioc_));
    try
    {
        beast::error_code ec;
        // Open the acceptor
        tmpAcceptor.open(tmpEndpoint.protocol(), ec);
        if (ec) {
            std::string err = "open:" + ec.what();
            throw std::exception(err.c_str());
        }

        // Allow address reuse
        tmpAcceptor.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) {
            std::string err = "set_option:" + ec.what();
            throw std::exception(err.c_str());
        }

        // Bind to the server address
        tmpAcceptor.bind(tmpEndpoint, ec);
        if (ec) {
            std::string err = "bind:" + ec.what();
            throw std::exception(err.c_str());
        }

        // Start listening for connections
        tmpAcceptor.listen(net::socket_base::max_listen_connections, ec);
        if (ec) {
            std::string err = "listen:" + ec.what();
            throw std::exception(err.c_str());
        }

        this->port_ = port;
        this->endpoint_ = tmpEndpoint;
        this->acceptor_ = std::move(tmpAcceptor);
    }
    catch (std::exception ex)
    {
        errMsg = ex.what();
        tmpAcceptor.close();
    }
    return errMsg;
}

std::string listener::run(int defaultPort)
{
    std::string errMsg;

    try
    {
        if (vThreads.size() > 0)
        {
            throw std::exception("already started...");
        }

        int port = defaultPort;
        int retryCnt = 100;
        while (retryCnt > 0)
        {
            std::string err = tryPort(port);
            if (err.length() > 0)
            {
                retryCnt--;
                // try another port
                port++;
            }
            else
            {
                break;
            }
        }
        // found no usable port...
        if (retryCnt == 0 || this->port_ == 0)
        {
            throw std::exception("no usable port");
        }

        do_accept();
        // Run the I/O service on the requested number of threads
        vThreads.clear();
        vThreads.reserve(1);
        for (int i = 0; i < 1; ++i)
        {
            net::io_context& ioc = ioc_;
            vThreads.emplace_back(
                [&ioc] {
                    ioc.run();
                });
        }
    }
    catch (std::exception ex)
    {
        errMsg = ex.what();
    }
    return errMsg;
}

void listener::setCBAccept(CB_Accept fnc)
{
    this->onAccept = fnc;
}

void listener::do_accept()
{
    // The new connection gets its own strand
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(&listener::on_accept, this)
    );
}

void listener::on_accept(beast::error_code ec, tcp::socket socket)
{
    if (ec)
    {
        fail(ec, "accept");
    }
    else
    {
        // Create the session and run it
        session* newSession = new session(std::move(socket));
        sessions_.push_back(newSession);
        if (onAccept != 0x0) onAccept(newSession);
    }

    // Accept another connection
    do_accept();
}

std::string listener::fail(beast::error_code ec, char const* what)
{
    std::string err = what + std::string(": ") + ec.what();
    //std::cerr << err << "\n";
    return err;
}
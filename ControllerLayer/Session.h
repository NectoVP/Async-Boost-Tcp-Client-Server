#pragma once
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>

#include "../ServiceLayer/Server.h"
#include "ErrorHandler.h"
#include "RequestHandler.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>
{
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    std::shared_ptr<std::string const> doc_root_;
    http::request<http::string_body> req_;
    std::shared_ptr<Server> server;
    std::shared_ptr<std::mutex> server_mutex;
    RequestHandler requestHandler;
public:
    Session(tcp::socket&& socket, std::shared_ptr<std::string const> const& doc_root, std::shared_ptr<Server>& server
        , std::shared_ptr<std::mutex>& server_mutex);

    void run();

    void do_read();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    void send_response(http::message_generator&& msg);
    void on_write(bool keep_alive, beast::error_code ec, std::size_t bytes_transferred);

    void do_close();
};
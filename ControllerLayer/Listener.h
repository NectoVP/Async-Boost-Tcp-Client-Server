#pragma once
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>

#include "../ServiceLayer/Server.h"
#include "ErrorHandler.h"
#include "Session.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class Listener : public std::enable_shared_from_this<Listener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<std::string const> doc_root_;
    std::shared_ptr<Server> server;
    std::shared_ptr<std::mutex> server_mutex;

public:
    Listener(net::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<std::string const> const& doc_root, std::shared_ptr<Server>& server);
    void run();
private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
};
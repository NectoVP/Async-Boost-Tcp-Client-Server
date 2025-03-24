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

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

beast::string_view mime_type(beast::string_view path) {
    using beast::iequals;
    auto const ext = [&path] {
        auto const pos = path.rfind(".");
        if(pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}

std::string path_cat(beast::string_view base, beast::string_view path) {
    if(base.empty())
        return std::string(path);
    std::string result(base);
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    return result;
}

//buy?itemId=1&itemCount=1&sessionId=1
//remove?itemId=1&itemCount=1&sessionId=1
//make_order?orderSum=100&sessionId=1

std::unordered_map<std::string, size_t> operations = { {"buy", 0}, {"remove", 1}, {"make_order", 2} };

std::unordered_map<std::string, size_t> split_str(std::string& s) {
    std::unordered_map<std::string, size_t> res;
    size_t question_mark_pos = s.find('?');
    res.insert(std::make_pair("method", operations[s.substr(1, question_mark_pos - 1)]));

    std::cout << s.substr(1, question_mark_pos - 1) << std::endl;

    int next_equation_mark = s.find('=', question_mark_pos);
    int next_ampersanda = s.find('&', question_mark_pos);
    next_ampersanda = next_ampersanda == -1 ? s.size() : next_ampersanda; 
    while(next_equation_mark != s.size()) {
        res.insert(std::make_pair(s.substr(question_mark_pos + 1, next_equation_mark - question_mark_pos - 1)
            , std::stoi(s.substr(next_equation_mark + 1, next_ampersanda - next_equation_mark - 1))));
        question_mark_pos = next_ampersanda;
        next_equation_mark = s.find('=', std::min(next_ampersanda, (int)s.size() - 1));
        if(next_equation_mark == -1)
            return res;
        next_ampersanda = s.find('&', next_equation_mark);
        next_ampersanda = next_ampersanda == -1 ? s.size() : next_ampersanda; 
    }
    return res;
}

// Return a response for the given request.
//
// The concrete type of the response message (which depends on the
// request), is type-erased in message_generator.
//template <class Body, class Allocator>
//void my_handle_request(beast::string_view doc_root, http::request<Body, http::basic_fields<Allocator>>&& req
//    , std::shared_ptr<Server>& server, std::shared_ptr<std::mutex>& server_mutex, std::shared_ptr<session>& session) {
//    
//    std::string s = req.target();
//    auto input = split_str(s);
//    
//    std::unique_lock<std::mutex> uniq_lock(*server_mutex);
//    if(input["method"] == 0) {
//        auto buy_callback = [&req, &session]() {
//            http::response<http::string_body> res{http::status::ok, req.version()};
//            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
//            res.set(http::field::content_type, "text/html");
//            res.keep_alive(req.keep_alive());
//            res.body() = std::string("item was bought correctly");
//            res.prepare_payload();
//            session->send_response(res);
//        };
//        server->Buy(input["itemId"], input["itemCount"], input["sessionId"], buy_callback);
//    }
//    else if(input["method"] == 1) {
//        server->Remove(input["itemId"], input["itemCount"], input["sessionId"]);
//    }
//    else if(input["method"] == 2) {
//        server->MakeOrder(input["orderSum"], input["sessionId"]);
//    }
//    uniq_lock.unlock();
//    return;
//
    //// Returns a bad request response
    //auto const bad_request = [&req](beast::string_view why) {
    //    http::response<http::string_body> res{http::status::bad_request, req.version()};
    //    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    //    res.set(http::field::content_type, "text/html");
    //    res.keep_alive(req.keep_alive());
    //    res.body() = std::string(why);
    //    res.prepare_payload();
    //    return res;
    //};
//
    //// Returns a not found response
    //auto const not_found = [&req](beast::string_view target) {
    //    http::response<http::string_body> res{http::status::not_found, req.version()};
    //    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    //    res.set(http::field::content_type, "text/html");
    //    res.keep_alive(req.keep_alive());
    //    res.body() = "The resource '" + std::string(target) + "' was not found.";
    //    res.prepare_payload();
    //    return res;
    //};
//
    //// Returns a server error response
    //auto const server_error = [&req](beast::string_view what) {
    //    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
    //    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    //    res.set(http::field::content_type, "text/html");
    //    res.keep_alive(req.keep_alive());
    //    res.body() = "An error occurred: '" + std::string(what) + "'";
    //    res.prepare_payload();
    //    return res;
    //};
//
    ////std::cout << req.body() << "tut";
//
    //// Make sure we can handle the method
    //if(req.method() != http::verb::get && req.method() != http::verb::head)
    //    return bad_request("Unknown HTTP-method");
//
    //// Request path must be absolute and not contain "..".
    //if(req.target().empty() || req.target()[0] != '/' || req.target().find("..") != beast::string_view::npos)
    //    return bad_request("Illegal request-target");
//
    //// Build the path to the requested file
    //std::string path = path_cat(doc_root, req.target());
    //if(req.target().back() == '/')
    //    path.append("index.html");
//
    //// Attempt to open the file
    //beast::error_code ec;
    //http::file_body::value_type body;
    //body.open(path.c_str(), beast::file_mode::scan, ec);
//
    //// Handle the case where the file doesn't exist
    //if(ec == beast::errc::no_such_file_or_directory)
    //    return not_found(req.target());
//
    //// Handle an unknown error
    //if(ec)
    //    return server_error(ec.message());
//
    //// Cache the size since we need it after the move
    //auto const size = body.size();
//
    //// Respond to HEAD request
    //if(req.method() == http::verb::head)
    //{
    //    http::response<http::empty_body> res{http::status::ok, req.version()};
    //    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    //    res.set(http::field::content_type, mime_type(path));
    //    res.content_length(size);
    //    res.keep_alive(req.keep_alive());
    //    return res;
    //}
//
    //// Respond to GET request
    //http::response<http::file_body> res{
    //    std::piecewise_construct,
    //    std::make_tuple(std::move(body)),
    //    std::make_tuple(http::status::ok, req.version())};
    //res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    //res.set(http::field::content_type, mime_type(path));
    //res.content_length(size);
    //res.keep_alive(req.keep_alive());
    //return res;
//}

void fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

class session : public std::enable_shared_from_this<session>
{
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    std::shared_ptr<std::string const> doc_root_;
    http::request<http::string_body> req_;
    std::shared_ptr<Server> server;
    std::shared_ptr<std::mutex> server_mutex;

public:
    session(tcp::socket&& socket, std::shared_ptr<std::string const> const& doc_root, std::shared_ptr<Server>& server
        , std::shared_ptr<std::mutex>& server_mutex) 
            : stream_(std::move(socket))
            , doc_root_(doc_root)
            , server(server)
            , server_mutex(server_mutex) {}

    template <class Body, class Allocator>
    void my_handle_request(beast::string_view doc_root, http::request<Body, http::basic_fields<Allocator>>&& req
        , std::shared_ptr<Server>& server, std::shared_ptr<std::mutex>& server_mutex) {
        
        std::string s = req.target();
        auto input = split_str(s);
        
        std::unique_lock<std::mutex> uniq_lock(*server_mutex);
        
        std::function<void(std::string&&, std::string&&)> callback = [session = shared_from_this()](std::string&& msg, std::string&& status) {
            http::response<http::string_body> res{http::status::ok, 10};
            if(status == "fail")
                res.result(http::status::bad_request);
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.body() = msg;
            res.prepare_payload();
            session->send_response(http::message_generator(std::move(res)));
        };

        if(input["method"] == 0) {
            server->Buy(input["itemId"], input["itemCount"], input["sessionId"], std::make_shared<std::function<void(std::string&&, std::string&&)>>(std::move(callback)));
        }
        else if(input["method"] == 1) {
            server->Remove(input["itemId"], input["itemCount"], input["sessionId"], std::make_shared<std::function<void(std::string&&, std::string&&)>>(std::move(callback)));
        }
        else if(input["method"] == 2) {
            server->MakeOrder(input["orderSum"], input["sessionId"], std::make_shared<std::function<void(std::string&&, std::string&&)>>(std::move(callback)));
        }
        uniq_lock.unlock();
        return;
    }

    void run() {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(stream_.get_executor(), beast::bind_front_handler(&session::do_read, shared_from_this()));
    }

    void do_read() {
        req_ = {};
        stream_.expires_after(std::chrono::seconds(30));
        http::async_read(stream_, buffer_, req_, beast::bind_front_handler(&session::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if(ec == http::error::end_of_stream)
            return do_close();

        if(ec)
            return fail(ec, "read");
        auto temp_session = shared_from_this();
        my_handle_request(*doc_root_, std::move(req_), server, server_mutex);
    }

    void send_response(http::message_generator&& msg)
    {
        bool keep_alive = msg.keep_alive();
        beast::async_write(stream_, std::move(msg), beast::bind_front_handler(&session::on_write, shared_from_this(), keep_alive));
    }

    void on_write(bool keep_alive, beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        if(! keep_alive)
        {
            return do_close();
        }

        do_read();
    }

    void do_close() {
        beast::error_code ec;
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
    }
};

class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<std::string const> doc_root_;
    std::shared_ptr<Server> server;
    std::shared_ptr<std::mutex> server_mutex;

public:
    listener(net::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<std::string const> const& doc_root, std::shared_ptr<Server>& server)
        : ioc_(ioc)
        , acceptor_(net::make_strand(ioc))
        , doc_root_(doc_root)
        , server(server)
        , server_mutex(std::make_shared<std::mutex>())
    {
        beast::error_code ec;

        acceptor_.open(endpoint.protocol(), ec);
        if(ec) {
            fail(ec, "open");
            return;
        }

        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec) {
            fail(ec, "set_option");
            return;
        }

        acceptor_.bind(endpoint, ec);
        if(ec) {
            fail(ec, "bind");
            return;
        }

        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if(ec) {
            fail(ec, "listen");
            return;
        }
    }

    void run() {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(net::make_strand(ioc_), beast::bind_front_handler(&listener::on_accept, shared_from_this()));
    }

    void on_accept(beast::error_code ec, tcp::socket socket)
    {
        if(ec) {
            fail(ec, "accept");
            return;
        }
        else {
            std::make_shared<session>(std::move(socket),doc_root_, server, server_mutex)->run();
        }

        do_accept();
    }
};
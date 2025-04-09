#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>

#include <gtest/gtest.h>

#include "../ServiceLayer/Server.h"
#include "../DataLayer/ItemHolder.h"
#include "../ControllerLayer/Listener.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class NetHelperTestClass {
public:
    std::string body;
    std::string status;
    void callback_with_save(std::string body, std::string status) {
        mtx.lock();
        this->body = body;
        this->status = status;
        mtx.unlock();
    };
    std::mutex mtx;
};
    

void fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

class Client : public std::enable_shared_from_this<Client>
{
    tcp::resolver resolver_;
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;

    nlohmann::json buy_json;
    nlohmann::json remove_json;
    nlohmann::json make_order_json;
public:
    std::shared_ptr<NetHelperTestClass> helper;
    std::function<void(std::string, std::string)> checker;
public:
    explicit Client(net::io_context& ioc) 
        : resolver_(net::make_strand(ioc))
        , stream_(net::make_strand(ioc))
        , helper(std::make_shared<NetHelperTestClass>())
        , checker(std::bind(&NetHelperTestClass::callback_with_save, helper, std::placeholders::_1, std::placeholders::_2)) {
        std::ifstream buy_file("/home/nectovp/Code/cpp/mpp/tests/test_buy.json");
        std::ifstream remove_file("/home/nectovp/Code/cpp/mpp/tests/test_remove.json");
        std::ifstream make_order_file("/home/nectovp/Code/cpp/mpp/tests/test_make_order.json");
    
        if(!buy_file.is_open()) {
            std::cout << "file with items description was not opened";
        }    

        buy_file >> buy_json;
        remove_file >> remove_json;
        make_order_file >> make_order_json;

        buy_file.close();
        remove_file.close();
        make_order_file.close();
    }

    void run(char const* host, char const* port, char const* target, char const* type, int version, int idx) {
        req_.version(version);
        req_.target(target);
        req_.set(http::field::content_type, "text/html");
        req_.method(http::verb::get);
        if(std::string(type) == "buy") {
            req_.method(http::verb::post);
            req_.body() = buy_json[idx].dump() + "\n";
        } else if(std::string(type) == "make_order") {
            req_.method(http::verb::post);
            req_.body() = make_order_json[idx].dump() + "\n";
        } else if(std::string(type) == "remove") {
            req_.method(http::verb::delete_);
            req_.body() = remove_json[idx].dump() + "\n";
        }
        req_.set(http::field::host, host);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req_.prepare_payload();
        resolver_.async_resolve(host, port, beast::bind_front_handler(&Client::on_resolve, shared_from_this()));
    }

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if(ec)
            return fail(ec, "resolve");

        stream_.expires_after(std::chrono::seconds(30));
        stream_.async_connect(results, beast::bind_front_handler(&Client::on_connect, shared_from_this()));
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
        if(ec)
            return fail(ec, "connect");

        stream_.expires_after(std::chrono::seconds(30));
        http::async_write(stream_, req_, beast::bind_front_handler(&Client::on_write, shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");
        http::async_read(stream_, buffer_, res_, beast::bind_front_handler(&Client::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        checker(std::string(res_.body()), std::to_string(res_.result_int()));
        stream_.socket().shutdown(tcp::socket::shutdown_both, ec);
        if(ec && ec != beast::errc::not_connected)
            return fail(ec, "shutdown");
    }
};

struct ClientTest : public testing::Test {
    const char* host = "0.0.0.0";
    const char* port = "8080";
    int version = 10;

    net::io_context ioc;
    
    ClientTest() : ioc() {}
};

TEST_F(ClientTest, BasicBuy) {
    auto client(std::make_shared<Client>(ioc));
    client->run(host, port, "/buy", "buy", 10, 0);
    ioc.run();
    ASSERT_EQ(client->helper->body, "item was bought correctly");
    ASSERT_EQ(client->helper->status, "200");
}

TEST_F(ClientTest, BuyManyItems) {
    {
        auto client(std::make_shared<Client>(ioc));
        client->run(host, port, "/buy", "buy", 10, 0);
        ioc.run();
        ASSERT_EQ(client->helper->body, "item was bought correctly");
        ASSERT_EQ(client->helper->status, "200");
    }
    ioc.stop();
    ioc.reset();
    {
        auto client(std::make_shared<Client>(ioc));
        client->run(host, port, "/buy", "buy", 10, 1);
        ioc.run();
        ASSERT_EQ(client->helper->body, "item was bought correctly");
        ASSERT_EQ(client->helper->status, "200");
    }
}

TEST_F(ClientTest, BuyError) {
    auto client(std::make_shared<Client>(ioc));
    client->run(host, port, "/buyy", "buy", 10, 0);
    ioc.run();
    ASSERT_EQ(client->helper->body, "The resource '/buyy' was not found.");
    ASSERT_EQ(client->helper->status, "404");
}

TEST_F(ClientTest, BasicRemove) {
    {
        auto client(std::make_shared<Client>(ioc));
        client->run(host, port, "/buy", "buy", 10, 0);
        ioc.run();
        ASSERT_EQ(client->helper->body, "item was bought correctly");
        ASSERT_EQ(client->helper->status, "200");
    }
    ioc.stop();
    ioc.reset();
    {
        auto client(std::make_shared<Client>(ioc));
        client->run(host, port, "/remove", "remove", 10, 0);
        ioc.run();
        ASSERT_EQ(client->helper->body, "item was removed correctly");
        ASSERT_EQ(client->helper->status, "200");
    }
}

TEST_F(ClientTest, RemoveError) {
    auto client(std::make_shared<Client>(ioc));
    client->run(host, port, "/removve", "remove", 10, 0);
    ioc.run();
    ASSERT_EQ(client->helper->body, "The resource '/removve' was not found.");
    ASSERT_EQ(client->helper->status, "404");
}

TEST_F(ClientTest, GetDesc) {
    auto client(std::make_shared<Client>(ioc));
    client->run(host, port, "/get_description", "get", 10, 0);
    ioc.run();
    ASSERT_EQ(client->helper->body.size(), 1472);
    ASSERT_EQ(client->helper->status, "200");
}

TEST_F(ClientTest, GetDescError) {
    auto client(std::make_shared<Client>(ioc));
    client->run(host, port, "/get_descriptionn", "get", 10, 0);
    ioc.run();
    ASSERT_EQ(client->helper->body, "The resource '/get_descriptionn' was not found.");
    ASSERT_EQ(client->helper->status, "404");
}


TEST_F(ClientTest, BuyTooManyItems) {
    auto client(std::make_shared<Client>(ioc));
    client->run(host, port, "/buy", "buy", 10, 3);
    ioc.run();
    ASSERT_EQ(client->helper->body, "you cannot buy this many items :1");
    ASSERT_EQ(client->helper->status, "500");
}

TEST_F(ClientTest, MakeOrder) {
    {
        auto client(std::make_shared<Client>(ioc));
        client->run(host, port, "/buy", "buy", 10, 2);
        ioc.run();
        ASSERT_EQ(client->helper->body, "item was bought correctly");
        ASSERT_EQ(client->helper->status, "200");
    }
    ioc.stop();
    ioc.reset();
    {
        auto client(std::make_shared<Client>(ioc));
        client->run(host, port, "/make_order", "make_order", 10, 2);
        ioc.run();
        ASSERT_EQ(client->helper->body, "order 4 is ready");
        ASSERT_EQ(client->helper->status, "200");
    }
}

TEST_F(ClientTest, MakeOrderPaymentNotSufficient) {
    {
        auto client(std::make_shared<Client>(ioc));
        client->run(host, port, "/buy", "buy", 10, 2);
        ioc.run();
        ASSERT_EQ(client->helper->body, "item was bought correctly");
        ASSERT_EQ(client->helper->status, "200");
    }
    ioc.stop();
    ioc.reset();
    {
        auto client(std::make_shared<Client>(ioc));
        client->run(host, port, "/make_order", "make_order", 10, 1);
        ioc.run();
        ASSERT_EQ(client->helper->body, "payment provided is not sufficient 150 1");
        ASSERT_EQ(client->helper->status, "402");
    }
}
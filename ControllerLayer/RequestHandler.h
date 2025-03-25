#pragma once
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>

#include "../ServiceLayer/Server.h"
#include "ErrorHandler.h"
#include "../DataLayer/json.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class RequestHandler {
public:
    RequestHandler() {}

    template <class Body>
    void handle_request(beast::string_view doc_root, http::request<Body>&& req, std::shared_ptr<Server>& server
            , std::shared_ptr<std::mutex>& server_mutex, std::function<void(http::message_generator&& msg)> responce_func) {
        
        auto const not_found = [send_response = responce_func](std::string& target) {
            http::response<http::string_body> res{http::status::not_found, 10};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.body() = "The resource '" + std::string(target) + "' was not found.";
            res.prepare_payload();
            send_response(http::message_generator(std::move(res)));
        };

        auto const unsupported_method = [send_response = responce_func](std::string why) {
            http::response<http::string_body> res{http::status::method_not_allowed, 10};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.body() = why;
            res.prepare_payload();
            send_response(http::message_generator(std::move(res)));
        };
        
        auto const invalid_params = [send_response = responce_func](std::string why) {
            http::response<http::string_body> res{http::status::unprocessable_entity, 10};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.body() = why;
            res.prepare_payload();
            send_response(http::message_generator(std::move(res)));
        };

        std::function<void(std::string&&, std::string&&)> callback = [send_response = responce_func](std::string&& msg, std::string&& status) {
            http::response<http::string_body> res{http::status::ok, 10};
            if(status == "internal_error")
                res.result(http::status::internal_server_error);
            if(status == "payment_failure")
                res.result(http::status::payment_required);
            
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.body() = msg;
            res.prepare_payload();
            send_response(http::message_generator(std::move(res)));
        };

        std::string target = req.target();
        std::unique_lock<std::mutex> uniq_lock(*server_mutex);

        if(req.method() == http::verb::get) {
            if(target == "/get_description")
                server->GetAllItemDescription(std::move(callback));
            else
                not_found(target);
            return;
        }
        //std::cout <<"\ntut\n" << std::string(req.body()) << ' ' << 
        //    std::string(req.target()) << std::endl << req << std::endl;
        
        nlohmann::json js = nlohmann::json::parse(std::string(req.body()));
        if(!check_params(js)) {
            invalid_params("invalid body parameters");
            return;
        }

        if(req.method() == http::verb::post) {
            if(target == "/buy")
                server->Buy(js["id"].template get<size_t>(), js["count"].template get<size_t>(), js["sessionId"].template get<size_t>(), std::move(callback));
            else if(target == "/make_order")
                server->MakeOrder(js["order_sum"].template get<size_t>(), js["sessionId"].template get<size_t>(), std::move(callback));
            else
                not_found(target);
            return;
        }
        std::cout << std::string(req.body());
        if(req.method() == http::verb::delete_) {
            std::cout << std::string(req.body());
            if(target == "/remove")
                server->Remove(js["id"].template get<size_t>(), js["count"].template get<size_t>(), js["sessionId"].template get<size_t>(), std::move(callback));
            else
                not_found(target);
            return;
        }

        unsupported_method("unsupported_method");
        return;
    }

private:
    bool check_params(nlohmann::json& js) {
        if(js.contains("id") && !js["id"].is_number_unsigned())
            return false;
        if(js.contains("count") && !js["count"].is_number_unsigned())
            return false;
        if(js.contains("sessionId") && !js["sessionId"].is_number_unsigned())
            return false;
        if(js.contains("order_sum") && !js["order_sum"].is_number_unsigned())
            return false;
        return true;
    }
};
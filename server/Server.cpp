#include "Server.h"
#include <boost/asio.hpp>
#include <thread>

bool operator==(const SessionId& first, const SessionId& second) {
    return first.id == second.id;
}

size_t SessionIdHash::operator()(const SessionId& sessionId) const {
    return std::hash<size_t>()(sessionId.id);
}

std::future<void> Server::Buy(size_t productId, size_t productCount, size_t sessionId) {
    std::future<void> f = boost::asio::post(threadPool,
        std::packaged_task<void()>(
            [server = shared_from_this(), productId, productCount, sessionId]() {
                std::unique_lock<std::shared_mutex> uniq_lock(server->bying_removing_mutex);
                (*server->boughtItems)[SessionId(sessionId)][productId] = productCount;
                return;
            }
        )
    );

    return f;
}

std::future<int> Server::Remove(size_t productId, size_t sessionId) {
    std::future<int> f = boost::asio::post(threadPool,
        std::packaged_task<int()>(
            [server = shared_from_this(), productId, sessionId]() {
                std::unique_lock<std::shared_mutex> uniq_lock(server->bying_removing_mutex);
                if((*server->boughtItems).find(SessionId(sessionId)) != (*server->boughtItems).end() 
                        && (*server->boughtItems)[SessionId(sessionId)].find(productId) != (*server->boughtItems)[SessionId(sessionId)].end())        
                    (*server->boughtItems)[SessionId(sessionId)].erase(productId);
                else {
                    std::cout << "item was not present in map in time of deletion\n";
                    return -1;
                }
                return 0;
            }
        )
    );

    return f;
    
}

std::future<bool> Server::Pay(size_t order_sum, size_t sessionId) {
    std::future<bool> f = boost::asio::post(threadPool,
        std::packaged_task<bool()>(
            [server = shared_from_this(), order_sum, sessionId]() {
                size_t temp_sum = 0;
                std::shared_lock<std::shared_mutex> shar_lock(server->bying_removing_mutex);
                auto item_costs = server->itemHolder->GetItems();
                for(auto& [itemId, amount]: (*server->boughtItems)[sessionId])
                    temp_sum += (*item_costs)[itemId].cost * amount;
                if(temp_sum != order_sum) {
                    std::cout << "payment provided is not sufficient\n";
                    return false;
                }
                return true;
            }
        )
    );

    return f;
}

std::future<void> Server::MakeOrder(size_t sessionId) {
    std::future<void> f = boost::asio::post(threadPool,
        std::packaged_task<void()>(
            [server = shared_from_this(), sessionId]() {
                for(auto& [itemId, amount]: (*server->boughtItems)[sessionId]) {
                    (*server->kitchenWorker).MakeProduct(amount * (*server->itemHolder->GetItems())[itemId.id].cooking_time);
                }
                std::unique_lock<std::shared_mutex> uniq_lock(server->bying_removing_mutex);
                (*server->boughtItems).erase(SessionId(sessionId));
                std::cout << "order " << sessionId << " is ready\n";
            }
        )
    );

    return f;
}

void Server::Wait(){
    threadPool.wait();
}

void Server::Join(){
    threadPool.join();
}

void Server::Stop(){
    threadPool.stop();
}

Server::~Server() {
    threadPool.stop();
}
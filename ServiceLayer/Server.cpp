#include <boost/asio.hpp>
#include <thread>

#include "Server.h"

std::future<void> Server::Buy(size_t itemId, size_t itemCount, size_t sessionId) {
    std::future<void> f = boost::asio::post(threadPool,
        std::packaged_task<void()>(
            [server = shared_from_this(), itemId, itemCount, sessionId]() {
                std::unique_lock<std::shared_mutex> uniq_lock(server->bying_removing_mutex);
                if((*server->itemHolder->GetItemsAmount())[itemId] >= itemCount) {
                    (*server->itemHolder->GetItemsAmount())[itemId] -= itemCount;
                    (*server->boughtItems)[sessionId][itemId] += itemCount;
                } else {
                    std::cout << "you cannot buy so many items";
                }
                return;
            }
        )
    );

    return f;
}

std::future<int> Server::Remove(size_t itemId, size_t sessionId) {
    std::future<int> f = boost::asio::post(threadPool,
        std::packaged_task<int()>(
            [server = shared_from_this(), itemId, sessionId]() {
                std::unique_lock<std::shared_mutex> uniq_lock(server->bying_removing_mutex);
                if((*server->boughtItems).find(sessionId) != (*server->boughtItems).end() 
                        && (*server->boughtItems)[sessionId].find(itemId) != (*server->boughtItems)[sessionId].end()) {       
                    (*server->itemHolder->GetItemsAmount())[itemId] += (*server->boughtItems)[sessionId][itemId];
                    (*server->boughtItems)[sessionId].erase(itemId);
                } else {
                    std::cout << "item was not present in map in time of deletion\n";
                    return -1;
                }
                return 0;
            }
        )
    );

    return f;
    
}

std::future<void> Server::MakeOrder(size_t order_sum, size_t sessionId) {
    std::future<void> f = boost::asio::post(threadPool,
        std::packaged_task<void()>(
            [server = shared_from_this(), order_sum, sessionId]() {
                std::shared_lock<std::shared_mutex> shar_lock(server->bying_removing_mutex);
                if(!server->CheckOrderCost(order_sum, sessionId)) {
                    std::cout << "payment provided is not sufficient\n";
                    return;
                }
                for(auto& [itemId, amount]: (*server->boughtItems)[sessionId]) {
                    (*server->kitchenWorker).MakeProduct(amount * (*server->itemHolder->GetItemsDescription())[itemId].cooking_time);
                }
                shar_lock.unlock();
                std::unique_lock<std::shared_mutex> uniq_lock(server->bying_removing_mutex);
                (*server->boughtItems).erase(SessionId(sessionId));
                std::cout << "order " << sessionId << " is ready\n";
            }
        )
    );

    return f;
}

bool Server::CheckOrderCost(size_t order_sum, size_t sessionId) {
    size_t temp_sum = 0;
    auto item_costs = itemHolder->GetItemsDescription();
    for(auto& [itemId, amount]: (*boughtItems)[sessionId])
        temp_sum += (*item_costs)[itemId].cost * amount;
    return temp_sum == order_sum;
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
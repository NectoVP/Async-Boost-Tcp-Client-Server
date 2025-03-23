#include <boost/asio.hpp>
#include <thread>

constexpr bool DEBUG_PRINT = false;

#include "Server.h"

std::future<void> Server::Buy(size_t itemId, size_t itemCount, size_t sessionId) {
    CheckSessionIdSize(sessionId);
    std::shared_lock<std::shared_mutex> temp_lock(resize_atomics_mutex);
    ++(*allAtomicsCount)[sessionId];
    temp_lock.unlock();

    std::future<void> f = boost::asio::post(threadPool,
        std::packaged_task<void()>(
            [server = shared_from_this(), itemId, itemCount, sessionId]() {
                std::unique_lock<std::mutex> uniq_lock(server->bying_removing_mutex);
                if(DEBUG_PRINT) std::cout << "buy method: " << itemId << ' ' << sessionId << std::endl;
                if((*server->itemHolder->GetItemsAmount())[itemId] >= itemCount) {
                    (*server->itemHolder->GetItemsAmount())[itemId] -= itemCount;
                    (*server->boughtItems)[sessionId][itemId] += itemCount;
                } else {
                    std::cout << "you cannot buy so many items";
                }
                std::shared_lock<std::shared_mutex> shar_lock(server->resize_atomics_mutex);
                --(*server->allAtomicsCount)[sessionId];
                if ((*server->allAtomicsCount)[sessionId].load() == 0)
                {
                    (*server->allAtomicsBools)[sessionId] = true;
                    (*server->allAtomicsBools)[sessionId].notify_all();
                }
               return;
           }
       )
    );
    return f;
}

std::future<void> Server::Remove(size_t itemId, size_t itemCount, size_t sessionId) {
    CheckSessionIdSize(sessionId);
    std::shared_lock<std::shared_mutex> temp_lock(resize_atomics_mutex);
    ++(*allAtomicsCount)[sessionId];
    temp_lock.unlock();

    std::future<void> f = boost::asio::post(threadPool,
        std::packaged_task<void()>(
            [server = shared_from_this(), itemId, itemCount, sessionId]() {
                std::unique_lock<std::mutex> uniq_lock(server->bying_removing_mutex);
                if(DEBUG_PRINT) std::cout << "remove method: " << itemId << ' ' << sessionId << std::endl;
                
                (*server->itemHolder->GetItemsAmount())[itemId] += itemCount;
                (*server->boughtItems)[sessionId][itemId] -= itemCount;
                
                std::shared_lock<std::shared_mutex> shar_lock(server->resize_atomics_mutex);
                --(*server->allAtomicsCount)[sessionId];
                if ((*server->allAtomicsCount)[sessionId].load() == 0)
                {
                    (*server->allAtomicsBools)[sessionId] = true;
                    (*server->allAtomicsBools)[sessionId].notify_all();
                }
                return;
            }
        )
    );
    return f;
}

std::future<void> Server::MakeOrder(size_t order_sum, size_t sessionId) {
    if(DEBUG_PRINT) std::cout << "tut";
    CheckSessionIdSize(sessionId);
    std::future<void> f = boost::asio::post(threadPool,
        std::packaged_task<void()>(
            [server = shared_from_this(), order_sum, sessionId]() {
                std::shared_lock<std::shared_mutex> temp_lock(server->resize_atomics_mutex);
                (*server->allAtomicsBools)[sessionId].wait(false);
                temp_lock.unlock();
                if(DEBUG_PRINT) std::cout << "make method " << (*server->allAtomicsBools)[sessionId].load() << ' ' << sessionId << std::endl;
                
                std::unique_lock<std::mutex> uniq_lock(server->bying_removing_mutex);
                size_t total_cooking_time = 0;
                if(!server->CheckOrderCost(order_sum, sessionId, total_cooking_time)) {
                    std::cout << "payment provided is not sufficient " << order_sum << " " << sessionId << '\n';
                    return;
                }
                (*server->boughtItems).erase(sessionId);
                uniq_lock.unlock();
                (*server->kitchenWorker).MakeProduct(total_cooking_time);
                std::cout << "order " << sessionId << " is ready\n";
            }
        )
    );
    return f;
}

bool Server::CheckOrderCost(size_t order_sum, size_t sessionId, size_t& total_cooking_time) {
    size_t temp_sum = 0;
    auto item_costs = itemHolder->GetItemsDescription();
    for(auto& [itemId, amount]: (*boughtItems)[sessionId]) {
        temp_sum += (*item_costs)[itemId].cost * amount;
        total_cooking_time += amount * (*itemHolder->GetItemsDescription())[itemId].cooking_time;
    }
    return temp_sum == order_sum;
}

void Server::CheckSessionIdSize(size_t sessionId) {
    std::unique_lock<std::shared_mutex> uniq_lock(resize_atomics_mutex);
    if(sessionId >= allAtomicsBools->size()) {
        std::vector<std::atomic<bool>> new_allAtomicsBools(sessionId * 2);
        std::vector<std::atomic<size_t>> new_allAtomicsCount(sessionId * 2);
        for(int i = 0; i < allAtomicsBools->size(); ++i) {
            new_allAtomicsBools[i] = (*allAtomicsBools)[i].load();
            new_allAtomicsCount[i] = (*allAtomicsCount)[i].load();
        }
        allAtomicsBools->swap(new_allAtomicsBools);
        allAtomicsCount->swap(new_allAtomicsCount);
    }
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
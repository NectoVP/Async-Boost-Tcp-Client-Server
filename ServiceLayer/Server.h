#pragma once
#include <iostream>
#include <deque>
#include <future>
#include <shared_mutex>

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/packaged_task.hpp>
#include <boost/lockfree/queue.hpp>

#include "../DataLayer/ItemHolder.h"
#include "KitchenWorker.h"

typedef size_t SessionId;

class Server : public std::enable_shared_from_this<Server> {
public:
    Server(const std::shared_ptr<ItemHolder>& itemHolder, const std::shared_ptr<KitchenWorker>& kitchenWorker) : 
        threadPool(std::thread::hardware_concurrency())
        , bying_removing_mutex()
        , itemHolder(itemHolder)
        , kitchenWorker(kitchenWorker)
        , boughtItems(std::make_shared<std::unordered_map<SessionId, std::unordered_map<ItemId, size_t>>>()) {}
    
    std::future<void> Buy(size_t itemId, size_t itemCount, size_t sessionId);
    std::future<int> Remove(size_t itemId, size_t sessionId);
    std::future<void> MakeOrder(size_t order_sum, size_t sessionId);

    bool CheckOrderCost(size_t order_sum, size_t sessionId);

    void Wait();
    void Join();
    void Stop();
    ~Server();

    std::shared_ptr<std::unordered_map<SessionId, std::unordered_map<ItemId, size_t>>> GetBoughtItemsTesTing() {
        return boughtItems;
    }

private:
    boost::asio::thread_pool threadPool;
    std::shared_mutex bying_removing_mutex;
    std::shared_mutex sessionThreadMutex;
    std::shared_ptr<std::unordered_map<SessionId, std::vector<std::thread>>> sessionThreads;
    std::shared_ptr<ItemHolder> itemHolder;
    std::shared_ptr<KitchenWorker> kitchenWorker;
    std::shared_ptr<std::unordered_map<SessionId, std::unordered_map<ItemId, size_t>>> boughtItems;  
};
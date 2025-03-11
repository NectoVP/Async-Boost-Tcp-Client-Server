#pragma once
#include <iostream>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/packaged_task.hpp>
#include <boost/lockfree/queue.hpp>
#include <deque>
#include <future>
#include <shared_mutex>
#include "ItemHolder.h"
#include "KitchenWorker.h"

struct SessionId{
    SessionId(size_t id) : id(id) {}
    size_t id;
};

bool operator==(const SessionId& first, const SessionId& second);

struct SessionIdHash {
    size_t operator()(const SessionId& sessionId) const;
};

class Server : public std::enable_shared_from_this<Server> {
public:
    Server(const std::shared_ptr<ItemHolder>& itemHolder, const std::shared_ptr<KitchenWorker>& kitchenWorker) : 
        threadPool(std::thread::hardware_concurrency())
        , bying_removing_mutex()
        , itemHolder(itemHolder)
        , kitchenWorker(kitchenWorker)
        , boughtItems(std::make_shared<std::unordered_map<SessionId, std::unordered_map<ItemId, size_t, ItemIdHash>, SessionIdHash>>()) {}
    
    std::future<void> Buy(size_t productId, size_t productCount, size_t sessionId);
    std::future<int> Remove(size_t productId, size_t sessionId);
    std::future<bool> Pay(size_t order_sum, size_t sessionId);
    std::future<void> MakeOrder(size_t sessionId);

    void Wait();
    void Join();
    void Stop();
    ~Server();

    std::shared_ptr<std::unordered_map<SessionId, std::unordered_map<ItemId, size_t, ItemIdHash>, SessionIdHash>> GetBoughtItemsTesTing() {
        return boughtItems;
    }

private:
    boost::asio::thread_pool threadPool;
    std::shared_mutex bying_removing_mutex;
    std::shared_ptr<ItemHolder> itemHolder;
    std::shared_ptr<KitchenWorker> kitchenWorker;
    std::shared_ptr<std::unordered_map<SessionId, std::unordered_map<ItemId, size_t, ItemIdHash>, SessionIdHash>> boughtItems;  
};
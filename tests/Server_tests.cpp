#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <gtest/gtest.h>
#include "../server/Server.h"

struct ServerTest : public testing::Test {
    ServerTest() : itemHolder(std::make_shared<ItemHolder>("/home/nectovp/Code/cpp/mpp/"))
        , kitchenWorker(std::make_shared<KitchenWorker>())
        , server(std::make_shared<Server>(itemHolder, kitchenWorker)) { }
    
    std::shared_ptr<ItemHolder> itemHolder;
    std::shared_ptr<KitchenWorker> kitchenWorker;
    std::shared_ptr<Server> server;
};


TEST_F(ServerTest, ItemPresentSingleSession) {
    auto f = server->Buy(1, 1, 1);
    f.wait();
    ASSERT_FALSE((*server->GetBoughtItemsTesTing())[SessionId(1)].find(ItemId(1)) == (*server->GetBoughtItemsTesTing())[SessionId(1)].end());
}

TEST_F(ServerTest, ItemPresentMultipleSessions) {
    auto a = server->Buy(1, 1, 1);
    auto b = server->Buy(2, 1, 2);
    auto c = server->Buy(4, 1, 3);
    
    a.wait();
    b.wait();
    c.wait();

    ASSERT_FALSE((*server->GetBoughtItemsTesTing())[SessionId(1)].find(ItemId(1)) == (*server->GetBoughtItemsTesTing())[SessionId(1)].end());
    ASSERT_FALSE((*server->GetBoughtItemsTesTing())[SessionId(2)].find(ItemId(2)) == (*server->GetBoughtItemsTesTing())[SessionId(2)].end());
    ASSERT_FALSE((*server->GetBoughtItemsTesTing())[SessionId(3)].find(ItemId(4)) == (*server->GetBoughtItemsTesTing())[SessionId(3)].end());
}

TEST_F(ServerTest, ItemCountSingleSession) {
    auto f = server->Buy(1, 1, 1);
    f.wait();
    ASSERT_EQ((*server->GetBoughtItemsTesTing())[SessionId(1)][ItemId(1)], 1);
}

TEST_F(ServerTest, ItemCountMultipleSessions) {
    auto a = server->Buy(1, 1, 1);
    auto b = server->Buy(5, 3, 2);
    auto c = server->Buy(1, 4, 2);

    a.wait();
    b.wait();
    c.wait();
    
    ASSERT_EQ((*server->GetBoughtItemsTesTing())[SessionId(1)][ItemId(1)], 1);
    ASSERT_EQ((*server->GetBoughtItemsTesTing())[SessionId(2)][ItemId(5)], 3);
    ASSERT_EQ((*server->GetBoughtItemsTesTing())[SessionId(2)][ItemId(1)], 4);
}

TEST_F(ServerTest, ItemUpdate) {
    auto a = server->Buy(1, 1, 1);
    a.wait();
    ASSERT_EQ((*server->GetBoughtItemsTesTing())[SessionId(1)][ItemId(1)], 1);
    auto b = server->Buy(1, 2, 1);
    b.wait();
    ASSERT_EQ((*server->GetBoughtItemsTesTing())[SessionId(1)][ItemId(1)], 2);
}

TEST_F(ServerTest, ItemRemoveSingleSession) {
    auto a = server->Buy(1, 1, 1);
    auto b = server->Buy(2, 1, 1);
    a.wait();
    b.wait();
    auto c = server->Remove(2, 1);
    c.wait();

    ASSERT_EQ(c.get(), 0);
    ASSERT_EQ((*server->GetBoughtItemsTesTing())[SessionId(1)][ItemId(1)], 1);
    ASSERT_EQ((*server->GetBoughtItemsTesTing())[SessionId(1)].find(ItemId(2)), (*server->GetBoughtItemsTesTing())[SessionId(1)].end());
}

TEST_F(ServerTest, ItemRemoveMultipleSessions) {
    auto a = server->Buy(1, 1, 1);
    auto b = server->Buy(2, 1, 2);
    a.wait();
    b.wait();
    auto c = server->Remove(1, 1);
    auto d = server->Remove(2, 2);
    c.wait();
    d.wait();
    
    ASSERT_EQ(c.get(), 0);
    ASSERT_EQ(d.get(), 0);
    ASSERT_EQ((*server->GetBoughtItemsTesTing())[SessionId(2)].find(ItemId(2)), (*server->GetBoughtItemsTesTing())[SessionId(2)].end());
    ASSERT_EQ((*server->GetBoughtItemsTesTing())[SessionId(1)].find(ItemId(1)), (*server->GetBoughtItemsTesTing())[SessionId(1)].end());
}


TEST_F(ServerTest, ItemRemoveError) {
    auto a = server->Remove(1, 1);
    a.wait();

    ASSERT_EQ(a.get(), -1);
    ASSERT_EQ((*server->GetBoughtItemsTesTing()).find(SessionId(1)), (*server->GetBoughtItemsTesTing()).end());
}

TEST_F(ServerTest, ItemDoubleRemoveError) {
    auto a = server->Buy(1, 1, 1);
    a.wait();
    auto b = server->Remove(1, 1);
    b.wait();
    auto c = server->Remove(1, 1);
    c.wait();

    ASSERT_EQ(b.get(), 0);
    ASSERT_EQ(c.get(), -1);
    ASSERT_NE((*server->GetBoughtItemsTesTing()).find(SessionId(1)), (*server->GetBoughtItemsTesTing()).end());
    ASSERT_EQ((*server->GetBoughtItemsTesTing())[SessionId(1)].find(ItemId(1)), (*server->GetBoughtItemsTesTing())[SessionId(1)].end());
}

TEST_F(ServerTest, ItemPaySingleSession) {
    auto a = server->Buy(1, 1, 1);
    a.wait();
    auto b = server->Buy(2, 1, 1);
    b.wait();
    auto c = server->Buy(3, 2, 1);
    c.wait();    
    auto d = server->Remove(1, 1);
    d.wait();
    
    auto e = server->Pay(500, 1);
    e.wait();

    ASSERT_EQ(e.get(), true);
    ASSERT_EQ(d.get(), 0);
}


TEST_F(ServerTest, ItemOverPaySingleSession) {
    auto a = server->Buy(1, 1, 1);
    a.wait();
    auto b = server->Buy(2, 1, 1);
    b.wait();
    auto c = server->Buy(3, 2, 1);
    c.wait();    
    auto d = server->Remove(1, 1);
    d.wait();
    
    auto e = server->Pay(600, 1);
    e.wait();

    ASSERT_EQ(e.get(), false);
    ASSERT_EQ(d.get(), 0);
}

TEST_F(ServerTest, ItemUnderPaySingleSession) {
    auto a = server->Buy(1, 1, 1);
    a.wait();
    auto b = server->Buy(2, 1, 1);
    b.wait();
    auto c = server->Buy(3, 2, 1);
    c.wait();    
    auto d = server->Remove(1, 1);
    d.wait();
    
    auto e = server->Pay(400, 1);
    e.wait();

    ASSERT_EQ(e.get(), false);
    ASSERT_EQ(d.get(), 0);
}

TEST_F(ServerTest, CompleteOrderSingleSession) {
    auto a = server->Buy(1, 1, 1);
    a.wait();
    auto b = server->Buy(2, 1, 1);
    b.wait();
    auto c = server->Buy(3, 2, 1);
    c.wait();    
    auto d = server->Remove(1, 1);
    d.wait();
    
    auto e = server->Pay(500, 1);
    e.wait();
    auto f = server->MakeOrder(500, 1);
    f.wait();

    ASSERT_EQ(e.get(), true);
    ASSERT_EQ(d.get(), 0);
    ASSERT_EQ((*server->GetBoughtItemsTesTing()).find(SessionId(1)), (*server->GetBoughtItemsTesTing()).end());
}
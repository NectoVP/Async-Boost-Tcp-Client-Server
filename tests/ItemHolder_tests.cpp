#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <gtest/gtest.h>
#include "../DataLayer/ItemHolder.h"

struct ItemHolderTest : public testing::Test {
    ItemHolderTest() : itemHolder(std::make_shared<ItemHolder>("/home/nectovp/Code/cpp/mpp/")) { }
    std::shared_ptr<ItemHolder> itemHolder;
};

TEST_F(ItemHolderTest, ItemCount) {
    ASSERT_EQ(itemHolder->GetItemsDescription()->size(), 20);
}

TEST_F(ItemHolderTest, ItemCost) {
    ASSERT_EQ((*itemHolder->GetItemsDescription())[ItemId(3)].cost, 150);
}

TEST_F(ItemHolderTest, ItemCookingTime) {
    ASSERT_EQ((*itemHolder->GetItemsDescription())[ItemId(6)].cooking_time, 200);
}
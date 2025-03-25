#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <gtest/gtest.h>
#include "../DataLayer/ItemHolder.h"

struct ItemHolderTest : public testing::Test {
    ItemHolderTest() : itemHolder(std::make_shared<ItemHolder>("/home/nectovp/Code/cpp/mpp/")) { }
    std::shared_ptr<ItemHolder> itemHolder;
};

TEST_F(ItemHolderTest, ItemDescCount) {
    ASSERT_EQ(itemHolder->GetItemsDescription()->size(), 20);
}

TEST_F(ItemHolderTest, ItemDescCost) {
    ASSERT_EQ((*itemHolder->GetItemsDescription())[ItemId(3)].cost, 350);
}

TEST_F(ItemHolderTest, ItemDescCookingTime) {
    ASSERT_EQ((*itemHolder->GetItemsDescription())[ItemId(6)].cooking_time, 250);
}

TEST_F(ItemHolderTest, ItemAmountCount) {
    ASSERT_EQ(itemHolder->GetItemsAmount()->size(), 20);
}

TEST_F(ItemHolderTest, ItemAmountInitialValue) {
    ASSERT_EQ((*itemHolder->GetItemsAmount())[ItemId(4)], 10);
}
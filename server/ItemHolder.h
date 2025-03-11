#pragma once
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <utility>

struct Item {
    Item() : cost(0), cooking_time(0) {}
    Item(size_t cost, size_t cooking_time) : cost(cost), cooking_time(cooking_time) {}
    size_t cost;
    size_t cooking_time;
};

struct ItemId{
    ItemId() : id(0) {}
    ItemId(size_t id) : id(id) {}
    ItemId(const ItemId& other) : id(other.id) {}
    int id;
};

bool operator==(const ItemId& first, const ItemId& second);

struct ItemIdHash {
    size_t operator()(const ItemId& itemId) const;
};

class ItemHolder {
public:
    ItemHolder(const std::string& path);

    std::shared_ptr<std::unordered_map<ItemId, Item, ItemIdHash>> GetItems() {
        return items;
    }

private:
    std::shared_ptr<std::unordered_map<ItemId, Item, ItemIdHash>> items;
};
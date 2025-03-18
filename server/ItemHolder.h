#pragma once
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <utility>

#include "json.hpp"

struct Item {
    Item() : cost(0), cooking_time(0) {}
    Item(size_t cost, size_t cooking_time, std::string& name, std::string& pic_url) : cost(cost)
        , cooking_time(cooking_time), name(name), pic_url(pic_url) {}
    Item(size_t cost, size_t cooking_time, std::string&& name, std::string&& pic_url) : cost(cost)
        , cooking_time(cooking_time), name(std::move(name)), pic_url(std::move(pic_url)) {}
    size_t cost;
    size_t cooking_time;
    std::string name;
    std::string pic_url;
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

    std::shared_ptr<std::unordered_map<ItemId, Item, ItemIdHash>> GetItemsDescription() {
        return items_description;
    }
    std::shared_ptr<std::unordered_map<ItemId, size_t, ItemIdHash>> GetItemsAmount() {
        return items_amount;
    }

private:
    std::shared_ptr<std::unordered_map<ItemId, Item, ItemIdHash>> items_description;
    std::shared_ptr<std::unordered_map<ItemId, size_t, ItemIdHash>> items_amount;
};
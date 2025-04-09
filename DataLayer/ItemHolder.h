#pragma once
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <utility>

#include "json.hpp"

typedef size_t ItemId;

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

class ItemHolder {
public:
    ItemHolder(const std::string& path, bool prod = false);

    std::shared_ptr<std::unordered_map<ItemId, Item>> GetItemsDescription() {
        return items_description;
    }
    std::shared_ptr<std::unordered_map<ItemId, size_t>> GetItemsAmount() {
        return items_amount;
    }
    std::shared_ptr<std::string> GetStringJson() {
        return item_description_json;
    }

private:
    std::shared_ptr<std::unordered_map<ItemId, Item>> items_description;
    std::shared_ptr<std::unordered_map<ItemId, size_t>> items_amount;
    std::shared_ptr<std::string> item_description_json;

    bool prod;
};
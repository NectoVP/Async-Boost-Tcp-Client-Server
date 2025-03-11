#include "ItemHolder.h"

bool operator==(const ItemId& first, const ItemId& second) {
    return first.id == second.id;
}

size_t ItemIdHash::operator()(const ItemId& itemId) const {
    return std::hash<size_t>()(itemId.id);
}

ItemHolder::ItemHolder(const std::string& path) {
    std::ifstream input_file(path + "server/items_desc.txt");
    if(!input_file.is_open()) {
        std::cout << "file with items cost was not opened";
    }

    std::string s;

    items = std::make_shared<std::unordered_map<ItemId, Item, ItemIdHash>>();

    while (std::getline(input_file, s)) {
        size_t split_pos = s.find(' ');
        size_t split_pos_second = s.find(' ', split_pos + 1);
        items->insert({
            ItemId(std::stoi(s.substr(0,split_pos))), 
            Item(std::stoi(s.substr(split_pos + 1, split_pos_second - split_pos - 1)), std::stoi(s.substr(split_pos_second + 1)))
        });
    }
    input_file.close();
}
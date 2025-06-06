#include "ItemHolder.h"

using json = nlohmann::json;

ItemHolder::ItemHolder(const std::string& path, const std::string& locale) {
    this->locale = locale;
    
    std::ifstream desc_file(path + "DataLayer/items_desc_prod.json");
    if(!desc_file.is_open()) {
        std::cout << "file with items description was not opened";
    }
    std::ifstream amount_file(path + "DataLayer/items_initial_count_prod.json");
    if(!amount_file.is_open()) {
        std::cout << "file with items description was not opened";
    }

    json item_desc_json_file;
    desc_file >> item_desc_json_file;
    json item_amount_json_file;
    amount_file >> item_amount_json_file;

    items_description = std::make_shared<std::unordered_map<ItemId, Item>>();
    for(const auto& i : item_desc_json_file) {
        items_description->insert({
            ItemId(i["id"].template get<size_t>()),
            Item(
                i["cost"].template get<size_t>(), 
                i["cooking_time"].template get<size_t>(), 
                i[locale + "_name"].template get<std::string>(),
                i["pic_url"].template get<std::string>()
            )
            }
        );
    }

    items_amount = std::make_shared<std::unordered_map<ItemId, size_t>>();
    for(const auto& i : item_amount_json_file) {
        items_amount->insert({
            ItemId(i["id"].template get<size_t>()),
            i["amount"].template get<size_t>(), 
            }
        );
    }
    
    item_description_json = std::make_shared<std::string>(item_desc_json_file.dump());

    desc_file.close();
    amount_file.close();
}
//
// Created by diago on 4/18/2024.
//

#include "base.hpp"

#include <utility>

std::string
lurch::object::generate_id() {

    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<> distribution(0, 15);

    static const char* character_set = "0123456789abcdef";
    std::string guid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";

    for(auto& ch : guid) {
        if(ch == 'x')
            ch = character_set[distribution(generator)];

        else if(ch == 'y')
            ch = character_set[distribution(generator)];
    }

    return guid;
}

lurch::object::~object() {
    if(!id.empty()) {
        io::info("deleting object with GUID: " + id);
    } else {
        io::failure("an object with no GUID is being deleted. Possible undefined behaviour.");
    }
}

lurch::owner::owner(std::optional<std::weak_ptr<owner>> parent, instance* root) : parent(std::move(parent)), root(root) {}
lurch::leaf::leaf(std::optional<std::weak_ptr<owner>> parent, instance *root) : parent(std::move(parent)), root(root) {}
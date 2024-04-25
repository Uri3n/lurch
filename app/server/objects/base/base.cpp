//
// Created by diago on 4/18/2024.
//

#include "base.hpp"

std::string lurch::object::generate_id() {

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
        std::cout << "deleting object with GUID: " << id << "\n\n";
    }
}

lurch::object::object()
    : id(lurch::object::generate_id()) {}

lurch::owner::owner(owner* parent, instance* root) : parent(parent), root(root) {
    this->lock = std::make_unique<std::mutex>();
}

lurch::owner::owner(instance *root) : root(root) {
    this->lock = std::make_unique<std::mutex>();
}

lurch::leaf::leaf(lurch::owner* parent, instance *root)
    : parent(parent), root(root) {}
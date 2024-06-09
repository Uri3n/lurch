//
// Created by diago on 2024-05-16.
//

#include <components.hpp>


std::string
lurch::instance::database::generate_token(const size_t length) {

    //
    // Not good. Gonna change this heavily later
    //

    static const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result;
    result.reserve(length);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);

    for (int i = 0; i < length; ++i) {
        result += charset[distribution(gen)];
    }

    return crow::utility::base64encode(result, result.size());
}


uint32_t
lurch::instance::database::hash_password(const std::string &password) {

    static constexpr uint32_t initial_seed = 7; //can be altered if needed
    uint32_t hash = 0;
    size_t index = 0;

    while(index != password.size()) {
        hash += password[index++];
        hash += hash << initial_seed;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;

    return hash;
}

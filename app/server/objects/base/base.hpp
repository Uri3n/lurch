//
// Created by diago on 4/18/2024.
//

#ifndef BASE_HPP
#define BASE_HPP
#include <memory>
#include <vector>
#include "../../util/common.hpp"
#include <iomanip>
#include <iostream>
#include <random>
#include <mutex>

namespace lurch {
    class instance;

    class object {
    public:
        std::string id;

        static std::string generate_id();
        virtual ~object();
        object();
    };

    class owner : public object {
    public:
        owner* parent;
        instance* root;
        std::vector<std::unique_ptr<object>> children;
        std::unique_ptr<std::mutex> lock = nullptr;

        virtual void recieve(const lurch::command& cmd) = 0;

        ~owner() override = default;
        owner(owner* parent, instance* root);
        owner(instance* root);
    };

    class leaf : public object {
    public:
        owner* parent;
        instance* root;

        virtual void recieve(const lurch::command& cmd) = 0;
        ~leaf() override = default;
        leaf(owner* parent, instance* root);
    };

} // lurch::objects

#endif //BASE_HPP

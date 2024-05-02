//
// Created by diago on 4/18/2024.
//

#ifndef BASE_HPP
#define BASE_HPP
#include <memory>
#include <vector>
#include "../../util/common.hpp"
#include "../../util/io.hpp"
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
        object() = default;
    };

    class owner : public object {
    public:
        std::optional<std::weak_ptr<owner>> parent;
        instance* root;
        std::vector<std::shared_ptr<object>> children;

        virtual std::string recieve(const lurch::command& cmd) = 0;
        virtual ~owner() = default;
        owner(std::optional<std::weak_ptr<owner>> parent, instance* root);
    };

    class leaf : public object {
    public:
        std::optional<std::weak_ptr<owner>> parent;
        instance* root;

        virtual std::string recieve(const lurch::command& cmd) = 0;
        virtual ~leaf() = default;
        leaf(std::optional<std::weak_ptr<owner>> parent, instance* root);
    };

} // lurch::objects

#endif //BASE_HPP

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
        access_level access = access_level::LOW;

        static std::string generate_id();
        virtual ~object();
        object() = default;
    };

    class owner : public object {
    public:
        std::optional<std::weak_ptr<owner>> parent;
        instance* inst;
        std::vector<std::shared_ptr<object>> children;

        result<bool> create_child(object_index index, object_type type, const std::string& alias);
        result<bool> delete_child(const std::string& guid);

        virtual std::string recieve(const command& cmd) = 0;
        virtual std::string upload(const std::string& file, const std::string& extension) = 0;
        virtual ~owner() = default;

        owner(std::optional<std::weak_ptr<owner>> parent, instance* inst);
    };

    class leaf : public object {
    public:
        std::optional<std::weak_ptr<owner>> parent;
        instance* inst;

        virtual std::string recieve(const command& cmd) = 0;
        virtual std::string upload(const std::string& file, const std::string& extension) = 0;
        virtual ~leaf() = default;

        leaf(std::optional<std::weak_ptr<owner>> parent, instance* inst);
    };

} // lurch::objects

#endif //BASE_HPP

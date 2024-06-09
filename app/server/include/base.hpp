//
// Created by diago on 4/18/2024.
//

#ifndef BASE_HPP
#define BASE_HPP
#include <memory>
#include <vector>
#include <common.hpp>
#include <filesystem>
#include <io.hpp>
#include <iomanip>
#include <iostream>
#include <random>
#include <mutex>
#include <queue>
#include <iostream>

#define AGENT_DELIMITING_CHAR '!'
#define OBJECT_EMPTY_RESPONSE ""

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

        virtual result<std::string>             receive(reciever_context& ctx) = 0;
        virtual result<std::filesystem::path>   upload(const std::string& file, const std::string& extension) = 0;

        virtual ~owner() = default;
        owner(std::optional<std::weak_ptr<owner>> parent, instance* inst);
    };

    class leaf : public object {
    public:
        std::optional<std::weak_ptr<owner>> parent;
        instance* inst;

        virtual result<std::string>             receive(reciever_context& ctx) = 0;
        virtual result<std::filesystem::path>   upload(const std::string& file, const std::string& extension) = 0;

        virtual ~leaf() = default;
        leaf(std::optional<std::weak_ptr<owner>> parent, instance* inst);
    };

} // lurch::objects

#endif //BASE_HPP

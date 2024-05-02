//
// Created by diago on 2024-04-24.
//

#ifndef INSTANCE_HPP
#define INSTANCE_HPP
#include "../vendor/sqlite/sqlite_modern_cpp.h"
#include "../util/common.hpp"
#include "../util/io.hpp"
#include "../objects/base/base.hpp"
#include "../objects/agent/agent.hpp"
#include "../objects/root/root.hpp"
#include "../objects/group/group.hpp"
#include <crow.h>
#include <optional>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <filesystem>
#include <thread>
#include <atomic>


namespace lurch {
using array_of_children = std::vector<std::tuple<std::string, std::string, lurch::object_type, lurch::object_index>>;

class instance {
    class database {
        private:
            std::mutex mtx;
            std::unique_ptr<sqlite::database> db = nullptr;

        public:
            instance* inst = nullptr;

            bool match_user(const std::string& username, const std::string& password);
            static inline uint32_t hash_password(const std::string &password);

            result<bool> store_object(const std::string& GUID, const std::optional<std::string> parent, const std::string alias, object_type type, object_index index);
            result<bool> store_user(const std::string& username, const std::string& password);

            result<std::string> query_root_guid();
            result<object_type> query_object_type(const std::string& guid);
            result<array_of_children> query_object_children(const std::string& guid);
            size_t object_count();

            result<bool> initialize(instance* inst, const std::optional<std::string >& initial_user, const std::optional<std::string>& initial_password);
            result<bool> restore_objects();
            void restore_objects_r(std::shared_ptr<owner>, size_t& total_restored);

            database() = default;
            ~database() = default;
    };

    class router {
        private:
            crow::SimpleApp app;
            struct {
                std::vector<crow::websocket::connection*> connections;
                std::mutex lock;
            } websockets;

        public:
            instance* inst = nullptr;

            void add_ws_connection(crow::websocket::connection* conn);
            void remove_ws_connection(crow::websocket::connection* conn);
            void send_ws_data(const std::string& data, const bool is_binary);
            void send_ws_text(const std::string& data);
            void send_ws_binary(const std::string& data);

            void run(std::string addr, uint16_t port);
            static result<std::pair<std::string, std::string>> hdr_extract_credentials(const crow::request& req);

            router() = default;
            ~router() = default;
    };

    class object_tree {
        private:
            uint32_t max_object_count = 100;
            std::atomic_uint32_t curr_object_count = 0;

        public:
            instance* inst = nullptr;
            std::recursive_mutex tree_lock;
            std::shared_ptr<object> root = nullptr;

            void set_max_object_count(const uint32_t count);
            void increment_object_count();

            result<bool> create_child(const std::string& parent_guid, object_index index);
            std::shared_ptr<object> create_object(object_index index, const std::optional<std::string> guid, const std::optional<std::weak_ptr<owner>> parent);

            object_tree() = default;
            ~object_tree() = default;
    };

public:
    std::thread server_thread;
    database db;
    router routing;
    object_tree tree;

    void begin();
    instance() = default;
    ~instance() = default;
};

} // lurch

#endif //INSTANCE_HPP

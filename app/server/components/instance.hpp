//
// Created by diago on 2024-04-24.
//

#ifndef INSTANCE_HPP
#define INSTANCE_HPP
#include "../vendor/sqlite/sqlite_modern_cpp.h"
#include "../util/common.hpp"
#include "../util/io.hpp"
#include "../objects/base/base.hpp"
#include "../objects/root/root.hpp"
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
class instance {
    class database {
        private:
            std::mutex mtx;
            std::unique_ptr<sqlite::database> db = nullptr;

        public:
            instance* inst = nullptr;

            bool match_user(const std::string& username, const std::string& password);
            static inline uint32_t hash_password(const std::string &password);
            result<bool> initialize(instance* inst, const std::optional<std::string >& initial_user, const std::optional<std::string>& initial_password);

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
            inline void send_ws_text(const std::string& data);
            inline void send_ws_binary(const std::string& data);

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
            std::shared_mutex tree_lock;
            std::unique_ptr<lurch::object> root = nullptr;

            inline void set_max_object_count(const uint32_t count);
            inline void increment_object_count();

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

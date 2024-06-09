//
// Created by diago on 2024-04-24.
//

#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "../vendor/sqlite/sqlite_modern_cpp.h"
#include "../util/common.hpp"
#include "../util/argument_parser.hpp"
#include "../util/io.hpp"
#include "../objects/base/base.hpp"
#include "../objects/agent/baphomet/baphomet.hpp"
#include "../objects/root/generic_root/root.hpp"
#include "../objects/group/group.hpp"
#include "../objects/external/chatroom/chatroom.hpp"

#include <crow.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <cstdint>
#include <memory>
#include <string_view>
#include <mutex>
#include <array>
#include <optional>
#include <functional>
#include <filesystem>
#include <thread>
#include <optional>
#include <atomic>

#if defined(_MSVC_STL_VERSION) && defined(_MSVC_LANG)
    #if _MSVC_LANG > 202002L
        #define LURCH_USE_STACKTRACE
        #include <stacktrace>
    #endif
#endif


namespace lurch {

using object_data = std::tuple<std::string, std::string, object_type, object_index>;
using array_of_children = std::vector<std::tuple<std::string, std::string, object_type, object_index>>;
using object_message = std::tuple<std::string, std::string, std::string>;

class instance {
    class database {
        private:
            std::mutex mtx;
            std::mutex fileman_mtx;
            std::unique_ptr<sqlite::database> db = nullptr;

        public:
            instance* inst = nullptr;

            result<access_level> match_user(const std::string& username, const std::string& password);
            bool match_token(const std::string& token, access_level required_access);

            static uint32_t hash_password(const std::string &password);
            static std::string generate_token(size_t length = 25);

            result<std::filesystem::path> fileman_create(const std::string_view& raw_contents, const std::string& extension, const std::string& guid, bool is_binary);
            result<std::stringstream> fileman_get_raw(const std::string& name, const std::string& guid);
            result<std::vector<std::filesystem::path>> fileman_get_file_list(const std::string& guid);

            bool fileman_delete_file(const std::string& name, const std::string& guid);
            bool fileman_delete_all_files(const std::string& guid);

            void fileman_wipe(const std::string& guid);
            void fileman_wipe_all();

            result<bool> store_object(const std::string& guid, std::optional<std::string> parent, std::string alias, object_type type, object_index index);
            result<bool> store_user(const std::string& username, const std::string& password, access_level access);
            result<bool> store_message(const std::string& guid, const std::string& sender,  const std::string& body);
            result<bool> store_token(const std::string& token, access_level access, const std::optional<std::string>& alias = std::nullopt, uint64_t expiration_time = 12);

            result<bool> delete_object(const std::string& guid);
            result<bool> delete_user(const std::string& username);
            void delete_old_tokens();

            result<std::string> query_root_guid();
            result<std::pair<std::string, access_level>> query_token_context(const std::string& token);
            result<object_type> query_object_type(const std::string& guid);
            result<array_of_children> query_object_children(const std::string& guid);
            result<object_data> query_object_data(const std::string& guid);
            result<std::vector<object_message>> query_object_messages(const std::string& guid, int message_index);
            result<std::vector<full_token_data>> query_full_token_list();

            result<bool> initialize(instance* inst, const std::optional<std::string >& initial_user, const std::optional<std::string>& initial_password);
            result<bool> restore_objects();
            void restore_objects_r(std::shared_ptr<owner>, size_t& total_restored);
            size_t object_count();

            database() = default;
            ~database() = default;
    };

    class router {
        private:
            struct {
                std::vector<std::pair<crow::websocket::connection*, std::optional<access_level>>> connections;
                std::mutex lock;
            } websockets;

            [[nodiscard]] bool verify_token(const crow::request& req, access_level required_access) const;

        public:
            crow::SimpleApp app;
            instance* inst = nullptr;

            static result<std::pair<std::string, std::string>> hdr_extract_credentials(const crow::request& req);
            static result<std::string> hdr_extract_token(const crow::request& req);
            static std::string file_template(const std::string& uri_path, const std::string& file_name, const std::string& extension);

            void add_ws_connection(crow::websocket::connection* conn);
            void remove_ws_connection(crow::websocket::connection* conn);
            bool verify_ws_user(crow::websocket::connection* conn, const std::string& data);

            void send_ws_data(const std::string& data, bool is_binary,std::optional<access_level> required_access);
            void send_ws_object_message_update(const std::string& body, const std::string& sender, std::string recipient, access_level required_access);
            void send_ws_object_create_update(const std::string& guid, std::string parent, const std::string& alias, object_type type);
            void send_ws_object_delete_update(const std::string& guid);
            void send_ws_notification(const std::string& message, ws_notification_intent intent);

            /* handler functions should NOT call crow::response::end, or set the response code. */
            bool handler_verify(const crow::request& req, crow::response& res) const;
            bool handler_objects_send(std::string GUID, const crow::request& req, crow::response& res, const token_context& tok) const;
            bool handler_objects_getdata(std::string GUID, crow::response& res) const;
            bool handler_objects_getchildren(std::string GUID, crow::response& res) const;
            bool handler_objects_getmessages(std::string GUID, int message_index, crow::response& res) const;
            bool handler_objects_upload(std::string GUID, const std::string& user_alias, const std::string& file_type, const crow::request& req, crow::response& res, access_level user_access);

            void run(std::string addr, uint16_t port, const std::optional<std::string>& ssl_cert, const std::optional<std::string>& ssl_key);

            router() = default;
            ~router() = default;
    };

    class object_tree {
        private:
            uint32_t max_object_count = 100;
            std::atomic_uint32_t curr_object_count = 0;

            static search_ctx send_message_r(const std::shared_ptr<object>& current, const std::string& guid, reciever_context& reciever_ctx);
            static std::pair<result<std::filesystem::path>, bool> upload_file_r(const std::shared_ptr<object>&, const std::string& guid, const std::string& file, const std::string& file_type, access_level access);
            static std::pair<result<access_level>, bool> lookup_access_level_r(const std::shared_ptr<object>& current, const std::string& guid);

        public:

            instance* inst = nullptr;
            std::recursive_mutex tree_lock;
            std::shared_ptr<object> root = nullptr;

            void set_max_object_count(uint32_t count);
            uint32_t get_max_object_count() const;
            void increment_object_count();
            result<access_level> lookup_access_level(std::string& guid) const;

            std::shared_ptr<object> create_object(object_index index, std::optional<std::string> guid, std::optional<std::weak_ptr<owner>> parent);
            search_ctx send_message(const std::string& guid, const std::string& cmd_raw, reciever_context& reciever_ctx);
            result<std::filesystem::path> upload_file(const std::string& guid, const std::string& file, const std::string& file_type, access_level access);

            object_tree() = default;
            ~object_tree() = default;
    };

    class event_log {
        private:
            std::mutex log_mtx;
            std::ofstream log_file;
        public:

            instance* inst = nullptr;

            result<bool> init(const std::string& file_name);
            void write(const std::string& message, log_type type, log_noise noise);
            static std::string format_log_message(log_type type, const std::string& message);

            ~event_log();
            event_log() = default;
    };

public:

    std::mutex mtx;
    std::condition_variable shutdown_condition;
    bool shutdown = false;

    database    db;
    router      routing;
    object_tree tree;
    event_log   log;

    static void handle_uncaught_exception();
    static bool generate_self_signed_cert(const std::string& certfile_path, const std::string& keyfile_path, long certificate_version);
    static result<config_data> init_config_data();

    void post_message_interaction(const std::string& sender, const std::string& object, const std::optional<std::string>& response, const std::string& message_content, access_level required_access);
    void begin();
    void await_shutdown();
    void set_shutdown_condition();

    instance() = default;
    ~instance() = default;
};

} // lurch

#endif //INSTANCE_HPP

//
// Created by diago on 2024-06-18.
//

#ifndef LISTENERS_HPP
#define LISTENERS_HPP
#include <components.hpp>

//
// Support for other listener types can be added later,
// if this project ends up actually going there
//

namespace lurch {
    class listener {
        public:
            std::string object_guid;
            instance*   inst;

            virtual ~listener() = default;
            explicit listener(const std::string& object_guid, instance* inst)
                : object_guid(object_guid), inst(inst) {}
    };

    class http_listener final : public listener {
        public:
            std::future<void> future;
            crow::SimpleApp   app;

            result<bool> start(std::string address,
                uint16_t port,
                const std::optional<std::string>& certfile,
                const std::optional<std::string>& keyfile
            );

            ~http_listener() override;
            http_listener(const std::string &object_guid, instance* inst)
                : listener(object_guid, inst) {
            }
    };
}

#endif //LISTENERS_HPP

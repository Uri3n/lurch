//
// Created by diago on 2024-05-01.
//

#ifndef GROUP_HPP
#define GROUP_HPP
#include <base.hpp>

namespace lurch {
class group final : public owner {
public:

    result<std::filesystem::path> upload(const std::string &file, const std::string &extension) override;
    result<std::string> receive(reciever_context& ctx) override;

    explicit group(const std::optional<std::weak_ptr<owner>> &parent, instance * const root)
        : owner(parent, root) {
    }

    ~group() override = default;
};

} // lurch

#endif //GROUP_HPP
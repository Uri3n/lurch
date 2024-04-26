//
// Created by diago on 2024-04-24.
//

#include "instance.hpp"

inline void lurch::instance::object_tree::set_max_object_count(const uint32_t count) {
    this->max_object_count = count;
}

inline void lurch::instance::object_tree::increment_object_count() {
    this->curr_object_count++;
}

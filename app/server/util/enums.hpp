//
// Created by diago on 2024-06-05.
//

#ifndef ENUMS_HPP
#define ENUMS_HPP

namespace lurch {

    //
    // Enums stored in the SQL tables must be larger than 16 bits, to
    // be compatible with the library used. Anything else should be 16 bits or less.
    //

    enum class log_type : uint16_t {
        INFO,           // generic log type. Neutral.
        SUCCESS,        // an operation was successfully completed.
        ERROR_MINOR,    // an error has occurred.
        ERROR_CRITICAL  // a critical error has occurred. This should ideally shut down the server.
    };

    enum class log_noise : uint16_t {
        QUIET,      // The message will be written to the console.
        REGULAR,    // The message will be written to the console, and to the log file.
        NOISY       // the message will be written to the console, log file, and the client will be notified.
    };

    enum class access_level : int32_t {
        LOW,     // token has low access. Most endpoints are locked, and objects greater than this level cannot be accessed.
        MEDIUM,  // token has medium access. All endpoints can be accessed. objects greater than this level cannot be accessed.
        HIGH     // token has high access. All permissions from access_level::MEDIUM carry over, and objects like ROOT can be accessed.
    };

    enum class object_type : int64_t {
        NONE,       // default.
        GROUP,
        AGENT,
        EXTERNAL,   // objects like chatrooms should have this.
        ROOT,
        GENERIC,
        CUSTOM      // unimplimented for now, we can do something with this later.
    };

    enum class object_index : int64_t {
        BAPHOMET,
        GENERIC_GROUP,
        GENERIC_CHATROOM,
        GENERIC_ROOT,
    };

    enum class ws_notification_intent : uint16_t {
        NEUTRAL,    // client notification has a grey color.
        BAD,        // client notification has a red/pinkish color.
        GOOD        // client notification has a blue color.
    };
}

#endif //ENUMS_HPP

#pragma once

#include <array>
#include <vector>
#include <cstddef>
#include <string>
#include <unordered_map>
#include "define.hpp"
#include <zmqpp/zmqpp.hpp>

SAIGON_NAMESPACE_BEGIN
class kvmsg
{
    using properties = std::unordered_map<std::string, std::string>;

    //  Keys are short strings
    inline static constexpr int KVMSG_KEY_MAX = 255;

    //  Message is formatted on wire as 4 frames:
    //  frame 0: getKey (0MQ string)
    //  frame 1: getSequence (8 bytes, network order)
    //  frame 2: uuid (blob, 16 bytes)
    //  frame 3: properties (0MQ string)
    //  frame 4: body (blob)
    inline static constexpr int FRAME_KEY       = 0;
    inline static constexpr int FRAME_SEQ       = 1;
    inline static constexpr int FRAME_UUID      = 2;
    inline static constexpr int FRAME_PROPS     = 3;
    inline static constexpr int FRAME_BODY      = 4;
    inline static constexpr int KVMSG_FRAMES    = 5;

    // Length
    inline static constexpr int FRAME_SEQ_LEN = 8;

    // Presence indicators for each frame
    std::array<bool, KVMSG_FRAMES> m_present;
    
    //Corresponding 0MQ message frames, if any
    std::array<std::string, KVMSG_FRAMES> m_frame;

    // List of properties, as name=value strings
    properties m_props;

public:
    kvmsg(size_t seq);
    
    void set_sequence(size_t seq);
    size_t get_sequence() const;

    std::string get_key() const;
    void set_key(std::string_view key);

    std::string get_body() const;
    void set_body(std::string_view body);
    size_t size() const;

    std::string get_UUID() const;
    void set_UUID();

    std::string get_prop(std::string const& name) const;
    void set_prop(std::string const& name, std::string const& val);

private:
    void encode_props();
    void decode_props();
};
SAIGON_NAMESPACE_END
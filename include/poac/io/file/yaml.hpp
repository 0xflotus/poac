#ifndef POAC_IO_FILE_YAML_HPP
#define POAC_IO_FILE_YAML_HPP

#include <iostream>
#include <string>
#include <map>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <yaml-cpp/yaml.h>

#include "../../core/exception.hpp"


namespace poac::io::file::yaml {
    namespace detail {
        struct wrapper {
            YAML::Node node;
            explicit wrapper(const YAML::Node& n) {
                // Argument-dependent lookup (ADL)
                node = Clone(n);
            }
            template <typename Key>
            wrapper& operator->*(const Key& key) {
                this->node = node[key];
                return *this;
            }
        };
        template <typename T>
        T get(const YAML::Node& node) {
            return node.as<T>();
        }
        template <typename T, typename ...Keys>
        T get(const YAML::Node& node, Keys&&... keys) {
            return get<T>((wrapper(node) ->* ... ->* keys).node);
        }
    }

    template <typename T, typename ...Args>
    boost::optional<T>
    get(const YAML::Node& node, Args&&... args) {
        try {
            return detail::get<T>(node, args...);
        }
        catch (const YAML::BadConversion& e) {
            return boost::none;
        }
    }
    template <typename ...Args>
    bool get(const YAML::Node& node, Args&&... args) {
        try {
            return detail::get<bool>(node, args...);
        }
        catch (const YAML::BadConversion& e) {
            return false;
        }
    }


    // Private member accessor
    template <class T, T V>
    struct accessor {
        static constexpr T m_isValid = V;
    };
    template <typename T>
    using bastion = accessor<T, &YAML::Node::m_isValid>;
    // using access_t = accessor<YAMLNode_t, &YAML::Node::m_isValid>;
    // -> error: 'm_isValid' is a private member of 'YAML::Node'
    using YAMLNode_t = bool YAML::Node::*;
    using access = bastion<YAMLNode_t>;


    template <typename Head>
    boost::optional<const char*>
    read(const YAML::Node& node, Head&& head) {
        if (!(node[head].*access::m_isValid)) {
            return head;
        }
        else {
            return boost::none;
        }
    }
    template <typename Head, typename ...Tail>
    boost::optional<const char*>
    read(const YAML::Node& node, Head&& head, Tail&&... tail) {
        if (!(node[head].*access::m_isValid)) {
            return head;
        }
        else {
            return read(node, tail...);
        }
    }


    template <typename... Args>
    static std::map<std::string, YAML::Node>
    get_by_width(const YAML::Node& node, const Args&... args) {
        namespace except = core::exception;
        if (const auto result = read(node, args...)) {
            throw except::error(
                    "Required key `" + std::string(*result) +
                    "` does not exist in poac.yml.\n"
                    "Please refer to https://docs.poac.pm");
        }
        else {
            std::map<std::string, YAML::Node> mp;
            ((mp[args] = node[args]), ...);
            return mp;
        }
    }
    template <typename... Args>
    static boost::optional<std::map<std::string, YAML::Node>>
    get_by_width_opt(const YAML::Node& node, const Args&... args) {
        namespace except = core::exception;
        if (const auto result = read(node, args...)) {
            return boost::none;
        }
        else {
            std::map<std::string, YAML::Node> mp;
            ((mp[args] = node[args]), ...);
            return mp;
        }
    }


    boost::optional<std::string>
    exists_config(const boost::filesystem::path& base)
    {
        namespace fs = boost::filesystem;
        if (const auto yml = base / "poac.yml"; fs::exists(yml)) {
            return yml.string();
        }
        else if (const auto yaml = base / "poac.yaml"; fs::exists(yaml)) {
            return yaml.string();
        }
        else {
            return boost::none;
        }
    }
    boost::optional<std::string>
    exists_config() {
        namespace fs = boost::filesystem;
        return exists_config(fs::current_path());
    }
    boost::optional<YAML::Node>
    load(const std::string& filename) {
        try { return YAML::LoadFile(filename); }
        catch (...) { return boost::none; }
    }


    YAML::Node load_config() {
        namespace except = core::exception;
        if (const auto op_filename = exists_config()) {
            if (const auto op_node = load(*op_filename)) {
                return *op_node;
            }
            else {
                throw except::error("Could not load poac.yml");
            }
        }
        else {
            throw except::error(
                    "poac.yml does not exists.\n"
                    "Please execute $ poac init or $ poac new $PROJNAME.");
        }
    }
    template <typename ...Args>
    static auto load_config(Args ...args) {
        return get_by_width(load_config(), args...);
    }
    template <typename ...Args>
    static auto load_config_opt(Args ...args) {
        return get_by_width_opt(load_config(), args...);
    }
} // end namespace
#endif // !POAC_IO_FILE_YAML_HPP

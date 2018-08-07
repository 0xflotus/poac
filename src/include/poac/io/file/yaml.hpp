#ifndef POAC_IO_FILE_YAML_HPP
#define POAC_IO_FILE_YAML_HPP

#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <yaml-cpp/yaml.h>

#include "../../io/cli.hpp"


namespace poac::io::file::yaml {
    bool _exists(const boost::filesystem::path& dir) {
        namespace fs = boost::filesystem;
        // If both are present, select the former.
        return fs::exists(dir / fs::path("poac.yml")) ||
               fs::exists(dir / fs::path("poac.yaml"));
    }
    [[maybe_unused]] bool exists(const boost::filesystem::path& dir) {
        return _exists(dir);
    }
    bool exists(/* current directory */) {
        return _exists(boost::filesystem::path("."));
    }

    bool notfound_handle() {
        const bool ret = exists();
        if (!ret) {
            std::cerr << io::cli::red
                      << "ERROR: poac.yml is not found"
                      << io::cli::reset
                      << std::endl;
        }
        return ret;
    }

    YAML::Node get_node() {
        return YAML::LoadFile("./poac.yml");
    }
    YAML::Node get_node(const std::string& name) {
        return YAML::LoadFile("./poac.yml")[name];
    }
} // end namespace
#endif // !POAC_IO_FILE_YAML_HPP

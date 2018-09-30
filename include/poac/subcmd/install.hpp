// Beta: It correspond to installation only from GitHub
//       Do not resolve dependencies
#ifndef POAC_SUBCMD_INSTALL_HPP
#define POAC_SUBCMD_INSTALL_HPP

#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <tuple>
#include <map>
#include <functional>
#include <utility>
#include <tuple>
#include <fstream>
#include <regex>
#include <chrono>
//#include <csignal> trap "rm to_TFRecord.py" 0

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/timer/timer.hpp>
#include <yaml-cpp/yaml.h>

#include "../io.hpp"
#include "../core/exception.hpp"
#include "../sources.hpp"
#include "../util.hpp"


// Version 1 will allow $ poac install <Nothing> only.
//   Parse poac.yml
// Version 2 will also allow $ poac install [<pkg-names>].
//   Parse arguments.

// TODO: 妙なprogressを出さないoptionが欲しい．CIや，Docker用
// TODO: --simple-progressみたいな -> elm-package installのoutputに寄せる
// TODO: --quiteも必要．
namespace poac::subcmd { struct install {
    static const std::string summary() { return "Beta: Install packages."; }
    static const std::string options() { return "<Nothing>"; }

    template <typename VS, typename = std::enable_if_t<std::is_rvalue_reference_v<VS&&>>>
    void operator()(VS&& argv) { _main(std::move(argv)); }


    template <typename Async>
    int installing(int* index_now, Async& async_funcs) {
        // 0/num packages installed|
        // |0/num packages installed
        std::cout << io::cli::left(50) << io::cli::up(1);
        std::cout << io::cli::up(async_funcs.size());

        int count = 0;
        for (auto& [info, func] : async_funcs) {
            const auto rel_time = std::chrono::milliseconds(0);
            const std::string status = func.wait_for(rel_time);

            std::cout << io::cli::right(1) << "\b";
            if (func.is_done()) {
                io::cli::set_left(50);
                std::cout << status;
                ++count;
            }
            else {
                io::cli::set_left(41);
                std::cout << " " + io::cli::at_spinner(*index_now) +
                             "  " + status;
            }
            std::cout << info
                      << io::cli::left(100)
                      << io::cli::down(1);
        }
        std::cout << std::endl
                  << io::cli::right(1)
                  << '\b'
                  << count
                  << std::flush;
        return count;
    }

    // Copy
    // ./poac.yml         pkgname/poac.yml
    //  deps: pkgname ->
    static bool _configure(const std::string& pkgname, const YAML::Node& deps) {
        const auto from_path =
                io::file::path::poac_cache_dir / pkgname;
        std::ofstream ofs((from_path / "poac.yml").string());
        ofs << deps;
        return EXIT_SUCCESS;
    }
    static bool _copy(const std::string& pkgname) {
        const auto from_path =
                io::file::path::poac_cache_dir / pkgname;
        const auto to_path =
                io::file::path::current_deps_dir /
                util::package::cache_to_current(pkgname);
        // Copy package to ./deps
        return io::file::path::recursive_copy(from_path, to_path);
    }
    static bool _copy_spec(const std::string& pkgname, const std::string& pkgname2) {
        const auto from_path =
                io::file::path::poac_cache_dir / pkgname;
        const auto to_path =
                io::file::path::current_deps_dir / pkgname2;
        // Copy package to ./deps
        return io::file::path::recursive_copy(from_path, to_path);
    }
    static bool _placeholder() { return EXIT_SUCCESS; }

    auto cache_func_pack(
        const std::string& pkgname,
        const std::string& source )
    {
        const std::string from_string = " (from " + source + ")";

        // If it exists in cache and it is not in the current directory copy it to the current.
        util::step_funcs_with_status step_funcs;

        step_funcs.statuses.push_back("Copying" + from_string);
        const std::function<bool()> f = std::bind(&_copy, pkgname);
        step_funcs.funcs.push_back(f);

        step_funcs.error_msg = io::cli::to_red(" ×  Install failed");
        step_funcs.finish_msg = io::cli::to_green(" ✔  Installed!" + from_string);
        return step_funcs;
    }
    auto cache_spec_func_pack(
            const std::string& name,
            const std::string& pkgname,
            const std::string& source )
    {
        const std::string from_string = " (from " + source + ")";

        // If it exists in cache and it is not in the current directory copy it to the current.
        util::step_funcs_with_status step_funcs;

        step_funcs.statuses.push_back("Copying" + from_string);
        const std::function<bool()> f = std::bind(&_copy_spec, pkgname, name);
        step_funcs.funcs.push_back(f);

        step_funcs.error_msg = io::cli::to_red(" ×  Install failed");
        step_funcs.finish_msg = io::cli::to_green(" ✔  Installed!" + from_string);
        return step_funcs;
    }
    auto github_func_pack(
        const std::string& name,
        const YAML::Node& deps,
        const std::string& version,
        const std::string& pkgname )
    {
        const std::string url = sources::github::resolve(name);
        const auto dest =
                io::file::path::poac_cache_dir /
                util::package::github_conv_pkgname(name, version);
        const std::string from_string = " (from github)";

        std::map<std::string, std::string> opts;
        opts.insert(io::network::opt_depth(1));
        opts.insert(io::network::opt_branch(version));

        util::step_funcs_with_status step_funcs;
        {
            step_funcs.statuses.push_back("Cloning" + from_string);
            const std::function<bool()> f = std::bind(&io::network::clone, url, dest, opts);
            step_funcs.funcs.push_back(f);
        }
        {
            step_funcs.statuses.push_back("Configuring" + from_string);
            const std::function<bool()> f = std::bind(&_configure, pkgname, YAML::Clone(deps));
            step_funcs.funcs.push_back(f);
        }
        {
            step_funcs.statuses.push_back("Copying" + from_string);
            const std::function<bool()> f = std::bind(&_copy, pkgname);
            step_funcs.funcs.push_back(f);
        }
        step_funcs.error_msg = io::cli::to_red(" ×  Install failed");
        step_funcs.finish_msg = io::cli::to_green(" ✔  Installed!" + from_string);
        return step_funcs;
    }
    auto poac_func_pack(
        const std::string& name,
        const std::string& version )
    {
        namespace tb = io::file::tarball;

        const std::string url = sources::poac::resolve(name, version);
        const std::string pkgname = sources::poac::pkgname(name, version);
        const auto pkg_dir = io::file::path::poac_cache_dir / pkgname;
        const auto tar_dir = pkg_dir.string() + ".tar.gz";
        const std::string from_string = " (from poac)";

        util::step_funcs_with_status step_funcs;
        {
            step_funcs.statuses.push_back("Downloading" + from_string);
            const std::function<bool()> f = std::bind(&io::network::get_file, url, tar_dir);
            step_funcs.funcs.push_back(f);
        }
        {
            step_funcs.statuses.push_back("Extracting" + from_string);
            const std::function<bool()> f = std::bind(&tb::extract_spec_rm_file, tar_dir, pkg_dir);
            step_funcs.funcs.push_back(f);
        }
        {
            step_funcs.statuses.push_back("Copying" + from_string);
            const std::function<bool()> f = std::bind(&_copy_spec, pkgname, name);
            step_funcs.funcs.push_back(f);
        }
        step_funcs.error_msg = io::cli::to_red(" ×  Install failed");
        step_funcs.finish_msg = io::cli::to_green(" ✔  Installed!" + from_string);
        return step_funcs;
    }
    auto notfound_func_pack()
    {
        util::step_funcs_with_status step_funcs;

        step_funcs.statuses.push_back("Notfound");
        const std::function<bool()> f = std::bind(&_placeholder);
        step_funcs.funcs.push_back(f);

        step_funcs.finish_msg = io::cli::to_red(" ×  Not found");
        return step_funcs;
    }

    auto create_func_pack(
        const std::string& name,
        const YAML::Node& deps,
        const std::string& version,
        const std::string& pkgname,
        const std::string& src )
    {
        if (sources::cache::resolve(pkgname))
            return cache_func_pack(pkgname, src);
        else if (const std::string pkgname2 = sources::poac::pkgname(name, version); sources::cache::resolve(pkgname2))
            return cache_spec_func_pack(name, pkgname2, src); // TODO: poac経由でインストールしたのがcacheに存在する
        else if (src == "github")
            return github_func_pack(name, deps, version, pkgname);
        else if (src == "poac")
            return poac_func_pack(name, version);
        else
            return notfound_func_pack();
    }

    template <typename Async>
    void dependencies(Async* async_funcs, const YAML::Node& node) {
        namespace except = core::exception;

        int already_count = 0;
        // Even if a package of the same name is written, it is excluded.
        // However, it can not deal with duplication of other information (e.g. version etc.).
        for (const auto& [name, next_node] : node.as<std::map<std::string, YAML::Node>>()) {
            // hello_world: 0.2.1
            // itr->first: itr->second
            std::string src = util::package::get_source(next_node);
            const std::string version = util::package::get_version(next_node, src);
            const std::string pkgname = util::package::github_conv_pkgname(name, version);

            // TODO: もし，poacからのインストールならこっちの名前になっている
            if (sources::current::resolve(pkgname) || io::file::path::validate_dir(io::file::path::current_deps_dir / name))
                ++already_count;
            else
                async_funcs->emplace(
                    name + ": " + version,
                    create_func_pack(name, next_node, version, pkgname, src)
                );
        }
        if (async_funcs->empty()) {
            if (already_count > 0)
                throw except::warn("Already up-to-date");
            else
                throw except::invalid_second_arg("install");
        }
    }

    template <typename Async>
    void start_funcs(Async& async_funcs) {
        namespace fs = boost::filesystem;
        fs::create_directories(io::file::path::current_deps_dir);
        // TODO: hardware concurrency
        for (auto& [_info, func] : async_funcs) {
            (void)_info; // Avoid unused warning
            func.start();
        }
    }

    void check_arguments(const std::vector<std::string>& argv) {
        namespace except = core::exception;
        if (!argv.empty()) throw except::invalid_second_arg("install");
    }

    template <typename VS, typename = std::enable_if_t<std::is_rvalue_reference_v<VS&&>>>
    void _main(VS&& argv) {
        namespace fs = boost::filesystem;

        // Start timer
        boost::timer::cpu_timer timer;


        check_arguments(argv);
        fs::create_directories(io::file::path::poac_cache_dir);
        const auto node = io::file::yaml::load_setting_file("deps");

        std::map<std::string, util::step_funcs_with_status> async_funcs;
        dependencies(&async_funcs, node.at("deps"));
        start_funcs(async_funcs);

        const int deps_num = static_cast<int>(async_funcs.size());

        std::cout << "Some new packages are needed.\n\n";
        for (int i = 0; i < deps_num; ++i)
            std::cout << std::endl;
        std::cout << std::endl
                  << "0/" << deps_num << " packages installed";


        for (int i = 0; installing(&i, async_funcs) != deps_num; ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));


        std::cout << io::cli::clr_line
                  << io::cli::left(30)
                  << "Elapsed time: "
                  << util::pretty_time::to(timer.format(6, "%w"))
                  << std::endl
                  << io::cli::bold
                  << " ==> Installation finished successfully!"
                  << std::endl;
    }
};} // end namespace
#endif // !POAC_SUBCMD_INSTALL_HPP

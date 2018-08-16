#ifndef POAC_SUBCMD_RUN_HPP
#define POAC_SUBCMD_RUN_HPP

#include <iostream>
#include <string>
#include <regex>

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

#include "../core/except.hpp"
#include "../io/file.hpp"
#include "../io/cli.hpp"
#include "../util/command.hpp"
#include "../util/package.hpp"
#include "./build.hpp"


namespace poac::subcmd { struct run {
    static const std::string summary() { return "Beta: Build project and exec it."; }
    static const std::string options() { return "[-v | --verbose | -- [program args]]"; }

    template <typename VS>
    void operator()(VS&& argv) { _main(argv); }
    template <typename VS>
    void _main(VS&& argv) {
        namespace fs     = boost::filesystem;
        namespace except = core::except;

        check_arguments(argv);

        std::vector<std::string> program_args;
        // poac run -v -- -h build
        if (const auto result = std::find(argv.begin(), argv.end(), "--"); result != argv.end()) {
            // -h build
            program_args = std::vector<std::string>(result+1, argv.end());
            // -v
            subcmd::build()(std::vector<std::string>(argv.begin(), result));
        }
        else {
            subcmd::build()(argv);
        }

        const std::string executable = fs::relative(io::file::path::current_build_bin_dir / "poac").string();
        util::command cmd(executable);
        for (const auto& s : program_args) {
            cmd += s;
        }

        std::cout << io::cli::green << "Running: " << io::cli::reset
                  << "`" + executable + "`"
                  << std::endl;
        if (const auto ret = cmd.run())
            std::cout << *ret;
        else // TODO: errorの時も文字列が欲しい．
            std::cout << "poac returned 1" << std::endl;
    }

    void check_arguments([[maybe_unused]] const std::vector<std::string>& argv) {
        namespace except = core::except;

//        if (argv.size() >= 2)
//            throw except::invalid_second_arg("run");
    }
};} // end namespace
#endif // !POAC_SUBCMD_RUN_HPP

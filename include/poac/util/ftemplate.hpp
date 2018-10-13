#ifndef POAC_UTIL_FTEMPLATE_HPP
#define POAC_UTIL_FTEMPLATE_HPP

#include <string>


namespace poac::util::ftemplate {
    const std::string _gitignore(
        "#\n"
        "# poac\n"
        "#\n"
        "deps\n"
        "_build\n"
    );
    const std::string main_cpp(
        "#include <iostream>\n"
        "\n"
        "int main(int argc, char** argv) {\n"
        "    std::cout << \"Hello, world!\" << std::endl;\n"
        "}\n"
    );
    std::string poac_yml(const std::string& project_name) {
        return "name: " + project_name + "\n"
               "version: 0.1.0\n"
               "cpp_version: 17\n"
               "description: \"**TODO: Add description**\"\n"
               "owners:\n"
               "  - \"Your ID\"\n"
               "build:\n"
               "  system: poac\n"
               "  bin: true\n";
    }
    std::string README_md(const std::string& project_name) {
        return "# " + project_name + "\n"
               "**TODO: Add description**\n"
               "\n"
               "---\n"
               "This project uses [poac](https://github.com/poacpm/poac).\n"
               "\n"
               "For more information on poac please see below:\n"
               "* https://poac.pm\n"
               "* https://github.com/poacpm\n"
               "* https://github.com/poacpm/poac#readme\n"
               "\n"
               "## Build\n"
               "\n"
               "```bash\n"
               "$ poac build # or run\n"
               "```\n"
               "\n"
               "## Installation\n"
               "\n"
               "To install `" + project_name + "`, add it to the dependency list of `poac.yml`:\n"
               "\n"
               "```yaml\n"
               "deps:\n"
               "  " + project_name + ": \"0.1.0\"\n"
               "```\n"
               "\n"
               "Execute the following command:\n"
               "`poac install`\n"
               ;
    }
    // TODO: 0.6.0 >=, doc
    // "\n"
    // "Documentation can be generated with doc.poac.pm\n"
    // "and published on [HexDocs](https://doc.poac.pm). Once published, the docs can\n"
    // "be found at https://doc.poac.pm/"+ project_name + ".\n"
} // end namespace
#endif // !POAC_UTIL_FTEMPLATE_HPP

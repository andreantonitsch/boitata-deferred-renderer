#pragma once

#include <map>
#include <string>

namespace boitatah::utils{

    class ShaderLoader{

        private:
            static std::map<std::string, std::string> partial_shaders;

            static void add_partial_shader(std::string& filename);
            static void replace_includes(std::string& shader);
            static void replace_include(std::string& key, std::string& content, std::string& shader);

            static void compile(std::string& filename);

            

    };



};
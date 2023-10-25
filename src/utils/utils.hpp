#ifndef BOITATAH_UTILS_HPP
#define BOITATAH_UTILS_HPP
#include <fstream>
#include <vector>
#include <string>

namespace boitatah::utils{
    static std::vector<char> readFile(const std::string& filename)
    {
        // ate starts from the back of the file
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file : " + filename);
        }

        // tellg == current header positon
        size_t fileSize = (size_t)file.tellg();

        std::vector<char> buffer(fileSize);
        file.seekg(0);                      // goes back to the beginning
        file.read(buffer.data(), fileSize); // fill buffer

        file.close();

        return buffer;
    }
}


#endif //BOITATAH_UTILS_HPP
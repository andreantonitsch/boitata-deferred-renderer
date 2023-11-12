#ifndef BOITATAH_UTILS_HPP
#define BOITATAH_UTILS_HPP
#include <fstream>
#include <vector>
#include <string>

// #include <format>
#include <chrono>

namespace boitatah::utils
{
    static std::vector<char> readFile(const std::string &filename)
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
    
    /// @brief Averaged timewatch
    struct Timewatch
    {
        int roll_size;
        int current;
        std::vector<std::chrono::microseconds> rolling;
        std::chrono::microseconds total;
        std::chrono::high_resolution_clock::time_point time;


        Timewatch(int numSamples) : rolling(numSamples){
            time = std::chrono::high_resolution_clock::now();
            roll_size = numSamples;
        }

        std::chrono::microseconds Lap()
        {
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - time);
            total -= rolling[current];
            rolling[current] = duration;
            total += duration;
            current = (current + 1) % roll_size;
            time = stop;
            return  total / roll_size;
        }
    };
}

#endif // BOITATAH_UTILS_HPP
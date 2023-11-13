#ifndef BOITATAH_UTILS_HPP
#define BOITATAH_UTILS_HPP
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
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
    
    /// @brief Rolling Averaged timewatch
    using timeduration = std::milli;
    struct Timewatch
    {
        int roll_size;
        int current;
        std::vector<std::chrono::duration<double, timeduration>> rolling;
        std::chrono::duration<double, timeduration> total;
        std::chrono::steady_clock::time_point start;


        Timewatch(int rollSize) : rolling(rollSize){
            for (size_t i = 0; i < rollSize; i++)
            {
                rolling[i] = std::chrono::duration<double, timeduration>(0);
            }
            total = std::chrono::duration<double, timeduration>(0);
            start = std::chrono::steady_clock::now();
            roll_size = rollSize;
        }

        std::chrono::duration<double, timeduration> Lap()
        {
            auto stop = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::duration<double, timeduration>>(stop - start);
            total += duration;
            total -= rolling[current];
            rolling[current] = duration;
            current = (current + 1) % roll_size;
            start = stop;
            return  total / static_cast<double>(roll_size);
        }
    };
}

#endif // BOITATAH_UTILS_HPP
#ifndef BOITATAH_UTILS_HPP
#define BOITATAH_UTILS_HPP
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
// #include <format>
#include <chrono>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>


namespace boitatah::utils
{

    static constexpr glm::mat4x4 getTransformMatrix(
                        glm::vec3               translation,
                        glm::vec3               scale,
                        glm::vec3               rotation){

        auto mat = glm::mat4(1.0f);
        mat = glm::scale(mat, scale);
        mat = glm::translate(mat, translation);
        mat = mat * glm::eulerAngleXYX(rotation.x, rotation.y, rotation.z);
        return mat;


    }

    template<typename T>
    inline static void move_concatenate_vectors(std::vector<T>& dst, std::vector<T>& src){
            dst.insert(dst.end(), 
                        std::make_move_iterator(src.begin()),
                        std::make_move_iterator(src.end()));
    
    }
    template<typename T>
    inline static void concatenate_vectors(std::vector<T>& dst, const std::vector<T>& src){
        dst.insert( dst.end(), src.begin(), src.end());
    }

    

    template <typename T>
    static std::vector<T> flatten(const std::vector<std::vector<T>>& v) {
        std::size_t total_size = 0;
        for (const auto& sub : v)
            total_size += sub.size(); // I wish there was a transform_accumulate
        std::vector<T> result;
        result.reserve(total_size);
        for (const auto& sub : v)
            result.insert(result.end(), sub.begin(), sub.end());
        return result;
    }

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

    template<typename T>
    std::string printList(std::vector<T>){
        return "";
    }

}

#endif // BOITATAH_UTILS_HPP
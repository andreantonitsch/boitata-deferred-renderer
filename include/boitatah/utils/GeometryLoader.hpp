#pragma once
#include "tiny_obj_loader.h"
#include <types/Geometry.hpp>
#include <string>
#include <vector>
namespace boitatah::utils{


    class GeometryLoader{


        struct tiny_data{
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

        };
        
        static tiny_data parseObj(std::string path){


        };

        static GeometryCreateDescription LoadOBJ(std::string file_path){
            
            
            
            
            
            
            GeometryCreateDescription description;

    

            return description;
        };
    };
};
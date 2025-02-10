 glslc ./shaders/$1.frag -o ./shaders/$1_frag.spv
 glslc ./shaders/$1.vert -o ./shaders/$1_vert.spv

 cp ./shaders/$1_vert.spv  ./shaders/$1_frag.spv ../bin/examples/src/

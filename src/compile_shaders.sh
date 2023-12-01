 glslc ./shaders/shader.frag -o ./shaders/frag.spv
 glslc ./shaders/shader.vert -o ./shaders/vert.spv

 cp ./shaders/vert.spv  ./shaders/frag.spv ../bin/examples/src/

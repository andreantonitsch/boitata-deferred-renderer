#pragma once

#include <deque>
#include <functional>

namespace boitatah::utils{

    class FunctionStack{

        std::deque<std::function<void()>> functions;

        void push_function(std::function<void()>&& function){

            functions.push_back(function);
        };
    
        void flush_stack(){
            
            for (auto it = functions.rbegin(); it != functions.rend(); it++){
                (*it)(); //call function
            }

            functions.clear();

        };
    };

}
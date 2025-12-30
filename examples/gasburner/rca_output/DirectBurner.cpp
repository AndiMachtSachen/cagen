#include "DirectBurner.hpp"
#include "DirectBurnerEnvironment.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>

#define EXIT(code) {std::cerr << "EXIT line " << __LINE__ << " with code " << code << std::endl;fflush(0);exit(code);}

void DirectBurner_state::update_system() {
    if(!leak) {
    	std::this_thread::sleep_for(std::chrono::milliseconds(50));
          }
    if(std::rand() <= std::rand()) {
    	leak = !leak;
    }
}

void write_kvs(DirectBurner_state const& state, char const* filename) {
    std::ofstream file(filename, std::ios::app);
    if (!file) {
        std::cerr << "Error opening file for writing!" << std::endl;
        EXIT(EXIT_FAILURE);
    }
    
    
    file << "t_e=" << state.t_e << ',';
        file << "t_s=" << state.t_s << ',';
    
    file << "leak=" << state.leak << ',';
    
    file << std::endl;
    file.close();
}

int main(int argc, char const * argv[]) {
    if(argc < 2) {
        std::cerr << "Did not specify shared file name for writing" << std::endl;
  EXIT(EXIT_FAILURE); 
    }
    auto const filename = argv[1];
    DirectBurner_state state{};
    
    auto time_before_env = std::chrono::steady_clock::now();
    while(true) {
        // provided inputs
        
        
        auto const time_before_system_update = std::chrono::steady_clock::now();
        auto const te_millis = std::chrono::duration_cast<std::chrono::milliseconds>(time_before_system_update - time_before_env);
        
        state.t_e = te_millis.count();
        state.update_system();
        
        auto const time_after_system_update = std::chrono::steady_clock::now();
        time_before_env = time_after_system_update;
        
        auto const ts_millis = std::chrono::duration_cast<std::chrono::milliseconds>(time_after_system_update - time_before_system_update);
        state.t_s = ts_millis.count();

        write_kvs(state, filename);
    }
}
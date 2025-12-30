#include "BurnerController.hpp"
#include "BurnerControllerEnvironment.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>

#define EXIT(code) {std::cerr << "EXIT line " << __LINE__ << " with code " << code << std::endl;fflush(0);exit(code);}

void BurnerController_state::update_system() {
    operate = false;
    if(!leak) {
    	std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 10));
    	operate = true;
          }
    steps += 1;
    if(steps > 5) {
    	leak = !leak;
    	steps = 0;
    	operate = false;
    }
    shutoff = !operate;
}

void write_kvs(BurnerController_state const& state, char const* filename) {
    std::ofstream file(filename, std::ios::app);
    if (!file) {
        std::cerr << "Error opening file for writing!" << std::endl;
        EXIT(EXIT_FAILURE);
    }
    
    
    file << "steps=" << state.steps << ',';
        file << "operate=" << state.operate << ',';
        file << "shutoff=" << state.shutoff << ',';
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
    BurnerController_state state{};
    
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
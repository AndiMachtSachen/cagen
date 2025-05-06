#include "Ecs.hpp"
#include "EcsEnvironment.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>

#define EXIT(code) {std::cerr << "EXIT line " << __LINE__ << " with code " << code << std::endl;fflush(0);exit(code);}

void Ecs_state::update_system() {
    duration = 15;
    tl = 50;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 10));
    
    if(wl == target_lvl) {
    	target_lvl = std::rand() % 10 + 20;
    	if(std::rand() % 2){
    		target_lvl = 2 * tl - target_lvl;
    	}
    }
    if(wl > target_lvl)--wl;
    else ++wl;
    
    operate = (gate_closed != (wl > tl));
    gate_closed = wl > tl;
}

void write_kvs(Ecs_state const& state, char const* filename) {
    std::ofstream file(filename, std::ios::app);
    if (!file) {
        std::cerr << "Error opening file for writing!" << std::endl;
        EXIT(EXIT_FAILURE);
    }
    
    file << "chw=" << state.chw << ',';
    file << "wl=" << state.wl << ',';
        file << "tl=" << state.tl << ',';
        file << "duration=" << state.duration << ',';
        file << "gate_closed=" << state.gate_closed << ',';
        file << "operate=" << state.operate << ',';
        file << "target_lvl=" << state.target_lvl << ',';
        file << "t_e=" << state.t_e << ',';
        file << "t_s=" << state.t_s << ',';
    
    
    
    file << std::endl;
    file.close();
}

int main(int argc, char const * argv[]) {
    if(argc < 2) {
        std::cerr << "Did not specify shared file name for writing" << std::endl;
  EXIT(EXIT_FAILURE); 
    }
    auto const filename = argv[1];
    Ecs_state state{};
    
    auto time_before_env = std::chrono::steady_clock::now();
    while(true) {
        // provided inputs
        state.chw = get_input_chw();
        
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
#include "MinePump.hpp"
#include "MinePumpEnvironment.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>

#define EXIT(code) {std::cerr << "EXIT line " << __LINE__ << " with code " << code << std::endl;fflush(0);exit(code);}

void MinePump_state::update_system() {
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand()%5));
    
    
    d = 50;
    e = 20;
    g = 10;
    kappa = 10;
    w = 10;
    
    if(wl > 0 && ( P || std::rand() % 2)) {
    	wl--;
    }else if(wl < 5 && std::rand() % 2) {
    	wl++;
    }
    HW = wl >= 2;
    DW = wl >= 4;
    DG = (std::rand()%100 == 0);
    
    P = HW && ! DG;
    if(A) {
    	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
    A = DW || DG;
    
}

void write_kvs(MinePump_state const& state, char const* filename) {
    std::ofstream file(filename, std::ios::app);
    if (!file) {
        std::cerr << "Error opening file for writing!" << std::endl;
        EXIT(EXIT_FAILURE);
    }
    
    
    file << "HW=" << state.HW << ',';
        file << "DW=" << state.DW << ',';
        file << "DG=" << state.DG << ',';
        file << "P=" << state.P << ',';
        file << "A=" << state.A << ',';
        file << "d=" << state.d << ',';
        file << "e=" << state.e << ',';
        file << "g=" << state.g << ',';
        file << "kappa=" << state.kappa << ',';
        file << "w=" << state.w << ',';
        file << "wl=" << state.wl << ',';
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
    MinePump_state state{};
    
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
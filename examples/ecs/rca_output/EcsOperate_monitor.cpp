#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <chrono>

#define TRUE true
#define FALSE false

#include "EcsOperate.hpp"

#ifndef MONITOR_RATE
#define MONITOR_RATE 100
#endif
#define EXIT(code) {std::cerr << "EXIT line " << __LINE__ << " with code " << code << std::endl;fflush(0);exit(code);}

std::vector<std::string> split(std::string const& str, char delimiter) {
    std::vector<std::string> words;
    std::string word;
    std::istringstream word_stream(str);
    while (std::getline(word_stream, word, delimiter)) {
        words.push_back(std::move(word));
    }
    return words;
}

std::map<std::string, std::string> read_kvs(char const* filename) {
    static int last_read_pos = 0;

    std::string line;
    while(true) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << std::string(filename) << std::endl;
            EXIT(EXIT_FAILURE);
        }
        
        file.seekg(last_read_pos);
        if(std::getline(file, line)) {
            auto next_read_pos = file.tellg();
            if(next_read_pos != -1) {
                last_read_pos = next_read_pos;
            }
            file.close();
            break;
        }
        #if(MONITOR_RATE)
        std::this_thread::sleep_for(std::chrono::milliseconds(MONITOR_RATE));
        #endif
    }
    std::vector<std::string> assignments = split(line, ',');
    std::map<std::string, std::string> kvs;

    for (const std::string& assignment : assignments) {
        auto kv = split(assignment, '=');
        assert(kv.size() == 2);
        kvs[kv[0]] = kv[1];
    }    
    
    //variableMap
    
            kvs["wl"] = kvs["wl"];
    return kvs;
}
                
int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Did not specify shared file name for reading" << std::endl;
        EXIT(EXIT_FAILURE);
    }
    auto filename = argv[1];
    
    EcsOperateMonitor monitor;
    
    long long iteration = 0;
    while (true) {
        ++iteration;
        auto kvs = read_kvs(filename);
        std::cout << "------------------------------------------------------- ["<<iteration<<"]\n";
        #if(DISPLAY_IOT)
        for (const auto& kv : kvs) {
            std::cout << kv.first << " = " << kv.second << ", ";
        }
        std::cout << std::endl;
        #endif
        
        auto te = std::stoi(kvs["t_e"]);
        auto ts = std::stoi(kvs["t_s"]);
        monitor.advance(te, ts);
        
        //inputs
        
        //outputs
        monitor.wl = std::stoi(kvs["wl"]);
        monitor.tl = std::stoi(kvs["tl"]);
        monitor.gate_closed = std::stoi(kvs["gate_closed"]);
        monitor.duration = std::stoi(kvs["duration"]);
        monitor.operate = std::stoi(kvs["operate"]);
        //internals
        
        
        //process
        monitor.update();
        
        #if(DISPLAY_TRACES)
        std::cout << monitor << '\n' << std::endl;
        #endif
        
        //check exit condition. default configuration uses `STOP_ON_EMPTY' definition
        if(monitor.should_stop()){
            break;
        }
    }
    
}
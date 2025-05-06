#pragma once

#define TRUE true
#define FALSE false
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <deque>
#include <string>
#include <tuple>

enum class ClockId{
    tP,t
};

[[nodiscard]] constexpr int clock_trace_capacity(ClockId clock_id) {
	switch(clock_id){
        //clocks with history
        
		//clocks without history
        default: return 1;
	}
}

#ifndef STOP_ON_EMPTY
#define STOP_ON_EMPTY 1
#endif
#ifndef DISPLAY_TRACES
#define DISPLAY_TRACES 1
#endif
#ifndef DISPLAY_IOT
#define DISPLAY_IOT 1
#endif

#ifdef FUZZY
#include "q_value.hpp"
#else
template<typename T, int... Ids>
using ClockVar = T;

using Q_Value = bool;
#endif

#ifdef RINGBUFFER

#include "ring_buffer.hpp"

template<typename T, std::size_t N>
std::ostream& operator<<(std::ostream& out, ring_buffer<T,N> const& v) {
    out << "[";
    for(auto it = v.begin(); it != v.end(); ++it) {
        if(it != v.begin()) out << ",";
        out << *it;
    }
    return out << "]";
}

template<typename T, int cap>
using TraceT = ring_buffer<T, cap + 1>;
#else
template<typename T, int cap>
using TraceT = std::deque<T>;
#endif

template<int clock_id>
class ClockVal {
    int _e{};
    int _s{};
    public:
    using value_type = ClockVar<int, clock_id>;
    
    ClockVal() noexcept = default;
    ClockVal(int env, int sys) noexcept : _e{env}, _s{sys} {};
    
    [[nodiscard]] value_type env() const { return _e; }
    [[nodiscard]] value_type sys() const { return _s; }
    [[nodiscard]] value_type total() const { return _e + _s; }
    
    void reset() {
        _e = _s = 0;
    }
    void advance(int t_e, int t_s) {
        _e += t_e;
        _s += t_s;
    }
    
    //lexicographical comparison for token deduplication
    [[nodiscard]] bool operator<(ClockVal const& rhs) const {
        return std::tie(_e, _s) < std::tie(rhs._e, rhs._s);
    }
    [[nodiscard]] bool operator==(ClockVal const& rhs) const {
        return std::tie(_e, _s) == std::tie(rhs._e, rhs._s);
    }
};
template<int clock_id>
std::ostream& operator<<(std::ostream& out, ClockVal<clock_id> const& v){
    return out << "(" << v.env() << "," << v.sys() << ")";
}

struct InvalidTimeAccess{};

enum class PumpMode {
    pumpOn, pumpOff
};
std::ostream& operator<<(std::ostream& out, PumpMode v);

using std::map;
using std::vector;
using std::set;
using std::deque; 
using std::string;
struct PumpClockValTrace{
     
        TraceT<ClockVal<(int)ClockId::tP>, clock_trace_capacity(ClockId::tP)> tP_trace;
        TraceT<ClockVal<(int)ClockId::t>, clock_trace_capacity(ClockId::t)> t_trace;
    
    //lexicographical comparison for token deduplication
    [[nodiscard]] bool operator<(PumpClockValTrace const& rhs) const {
        return std::tie(
        tP_trace,t_trace) < std::tie(
        rhs.tP_trace,rhs.t_trace); 
    }
};

template<typename T>
std::ostream& operator<<(std::ostream& out, vector<T> const& v) {
    out << "[";
    for(auto it = v.cbegin(); it != v.cend(); ++it) {
        if(it != v.cbegin()) out << ",";
        out << *it;
    }
    return out << "]";
}
template<typename T>
std::ostream& operator<<(std::ostream& out, deque<T> const& v) {
    out << "[";
    for(auto it = v.cbegin(); it != v.cend(); ++it) {
        if(it != v.cbegin()) out << ",";
        out << *it;
    }
    return out << "]";
}
struct PumpTok {
    PumpMode mode;
    PumpClockValTrace clock_traces;
    #ifdef FUZZY
    Q_Value q_assume;
    Q_Value q_guarantee;
    #endif
    
    //lexicographical comparison for token deduplication
    [[nodiscard]] bool operator<(PumpTok const& rhs) const {
        #ifdef FUZZY
        return std::tie(mode, clock_traces, q_assume, q_guarantee) < std::tie(rhs.mode, rhs.clock_traces, rhs.q_assume, rhs.q_guarantee);
        #else
        return std::tie(mode, clock_traces) < std::tie(rhs.mode, rhs.clock_traces);
        #endif
    }
};

#if(DEDUPLICATE_TOKENS)
using ToksT = std::set<PumpTok>;
#else
using ToksT = std::vector<PumpTok>;
#endif

struct PumpMonitor {
    //inputs
    
    //outputs
    bool HW{};
    bool DW{};
    bool DG{};
    bool P{};
    bool A{};
    int d{};
    //internals
    
    //tokens
    ToksT tokens;
    
    //history
    bool h_P_0{};
    bool h_P_1{};
    
    bool postcondition_accessed_incorrect_time = false;
    bool precondition_accessed_incorrect_time = false;
    
    bool SYSTEM_LOSES = false;
    bool ENVIRONMENT_LOSES = false;

    [[nodiscard]] PumpMonitor() noexcept {
        PumpClockValTrace initial_clock_val;
        
        initial_clock_val.tP_trace = TraceT<ClockVal<(int)ClockId::tP>, clock_trace_capacity(ClockId::tP)>{ClockVal<(int)ClockId::tP>{}};
        initial_clock_val.t_trace = TraceT<ClockVal<(int)ClockId::t>, clock_trace_capacity(ClockId::t)>{ClockVal<(int)ClockId::t>{}};
        tokens = ToksT{PumpTok{PumpMode::pumpOn, initial_clock_val}, PumpTok{PumpMode::pumpOff, initial_clock_val}};
        
    }
    void update();
    void advance(int t_e, int t_s);
    [[nodiscard]] bool should_stop() const;
    friend std::ostream& operator<<(std::ostream& out, PumpMonitor const&);
};

enum class ClockKind { env, sys, total };

template<ClockKind clock_kind, int clock_id>
class ClockHistoryEntry {
    using trace_type = TraceT<ClockVal<clock_id>, clock_trace_capacity((ClockId)clock_id)>; 
	trace_type* trace;
	int depth;
	
	public:
	[[nodiscard]] ClockHistoryEntry(trace_type& trace, int depth) noexcept : 
        trace{&trace}, depth{depth} {}
	
	[[nodiscard]] operator int() const {
        if(depth < trace->size()) {
            auto const& clock = (*trace)[trace->size() - depth - 1];
            switch(clock_kind) {
                case ClockKind::env: return clock.env();
                case ClockKind::sys: return clock.sys();
                case ClockKind::total: return clock.total();
            }
        }else{
            #ifndef ERROR_TRACE_ACCESS
            return 0;
            #else
            throw InvalidTimeAccess{};
            #endif
        }
    }

};
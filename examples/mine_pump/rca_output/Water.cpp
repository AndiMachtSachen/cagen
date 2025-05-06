#include "Water.hpp"

std::ostream& operator<<(std::ostream& out, WaterMode v) {
    switch(v) {
        
        case WaterMode::low:
            return out << "low"; 
        case WaterMode::high:
            return out << "high"; 
        case WaterMode::danger:
            return out << "danger"; 
    }
}

void WaterMonitor::advance(int t_e, int t_s) {
    std::cout << "Advance monitor by t_e = "<<t_e<<", t_s = "<<t_s<<std::endl;
    #if(DEDUPLICATE_TOKENS)
    ToksT next_toks;
    for(auto tok : tokens) {
        
        tok.clock_traces.tW_trace.back().advance(t_e, t_s);
        
        tok.clock_traces.t_trace.back().advance(t_e, t_s);
        
        next_toks.insert(std::move(tok));
    }
    tokens = std::move(next_toks);
    #else
    for(auto& tok : tokens) {
        
        tok.clock_traces.tW_trace.back().advance(t_e, t_s);
        
        tok.clock_traces.t_trace.back().advance(t_e, t_s);
        
    }
    #endif
}

void WaterMonitor::update() {
    postcondition_accessed_incorrect_time = false;
    precondition_accessed_incorrect_time = false;
    
    //update history                                    
    
    
    //update token marking
    ToksT next_tokens;
    bool any_pre = false;
    #if(DEDUPLICATE_TOKENS)
    for(auto tok : tokens) {
    #else
    for(auto& tok : tokens) {
    #endif
        
        auto tW = tok.clock_traces.tW_trace.back().total();
        auto tW_e = tok.clock_traces.tW_trace.back().env();
        auto tW_s = tok.clock_traces.tW_trace.back().sys();
        
        auto t = tok.clock_traces.t_trace.back().total();
        auto t_e = tok.clock_traces.t_trace.back().env();
        auto t_s = tok.clock_traces.t_trace.back().sys();
        
        #ifndef RINGBUFFER
        #ifndef UNBOUNDED_TRACE
        
        if(tok.clock_traces.tW_trace.size() > 1)tok.clock_traces.tW_trace.pop_front();
        if(tok.clock_traces.t_trace.size() > 1)tok.clock_traces.t_trace.pop_front();
        #endif
        
        #else
        //ring_buffer automatically overrides old values
        
        #endif
        switch(tok.mode) {
            case WaterMode::low: {
                try{
                Q_Value pre_cond = (!HW && !DW);
                #ifdef FUZZY
                pre_cond = tok.q_assume && pre_cond;
                #endif
                if(pre_cond) {
                    any_pre = true;
                    try{
                    Q_Value post_cond = true;
                    #ifdef FUZZY
                    post_cond = tok.q_guarantee && post_cond;
                    #endif
                    if(post_cond) {
                        auto new_clock_traces = tok.clock_traces;
                        
                        {
                        auto clockvals = &new_clock_traces.tW_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        {
                        auto clockvals = &new_clock_traces.t_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        
                        new_clock_traces.tW_trace.back().reset();
                        
                        new_clock_traces.t_trace.back().reset();
                        
                        #ifdef FUZZY
                        auto new_tok = WaterTok{WaterMode::low, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = WaterTok{WaterMode::low, std::move(new_clock_traces)};
                        #endif
                        
                        #if(DEDUPLICATE_TOKENS)
                        next_tokens.insert(std::move(new_tok));
                        #else
                        next_tokens.emplace_back(std::move(new_tok));
                        #endif
                    }
                    } catch(InvalidTimeAccess const& time_err) {
                        postcondition_accessed_incorrect_time = true;
                    }
                }
                } catch(InvalidTimeAccess const& time_err) {
                    precondition_accessed_incorrect_time = true;
                }
                try{
                Q_Value pre_cond = (HW && !DW);
                #ifdef FUZZY
                pre_cond = tok.q_assume && pre_cond;
                #endif
                if(pre_cond) {
                    any_pre = true;
                    try{
                    Q_Value post_cond = true;
                    #ifdef FUZZY
                    post_cond = tok.q_guarantee && post_cond;
                    #endif
                    if(post_cond) {
                        auto new_clock_traces = tok.clock_traces;
                        
                        {
                        auto clockvals = &new_clock_traces.tW_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        {
                        auto clockvals = &new_clock_traces.t_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        
                        new_clock_traces.t_trace.back().reset();
                        
                        #ifdef FUZZY
                        auto new_tok = WaterTok{WaterMode::high, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = WaterTok{WaterMode::high, std::move(new_clock_traces)};
                        #endif
                        
                        #if(DEDUPLICATE_TOKENS)
                        next_tokens.insert(std::move(new_tok));
                        #else
                        next_tokens.emplace_back(std::move(new_tok));
                        #endif
                    }
                    } catch(InvalidTimeAccess const& time_err) {
                        postcondition_accessed_incorrect_time = true;
                    }
                }
                } catch(InvalidTimeAccess const& time_err) {
                    precondition_accessed_incorrect_time = true;
                }
                break;
            };
            
            case WaterMode::high: {
                try{
                Q_Value pre_cond = (HW && !DW);
                #ifdef FUZZY
                pre_cond = tok.q_assume && pre_cond;
                #endif
                if(pre_cond) {
                    any_pre = true;
                    try{
                    Q_Value post_cond = true;
                    #ifdef FUZZY
                    post_cond = tok.q_guarantee && post_cond;
                    #endif
                    if(post_cond) {
                        auto new_clock_traces = tok.clock_traces;
                        
                        {
                        auto clockvals = &new_clock_traces.tW_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        {
                        auto clockvals = &new_clock_traces.t_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        
                        new_clock_traces.t_trace.back().reset();
                        
                        #ifdef FUZZY
                        auto new_tok = WaterTok{WaterMode::high, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = WaterTok{WaterMode::high, std::move(new_clock_traces)};
                        #endif
                        
                        #if(DEDUPLICATE_TOKENS)
                        next_tokens.insert(std::move(new_tok));
                        #else
                        next_tokens.emplace_back(std::move(new_tok));
                        #endif
                    }
                    } catch(InvalidTimeAccess const& time_err) {
                        postcondition_accessed_incorrect_time = true;
                    }
                }
                } catch(InvalidTimeAccess const& time_err) {
                    precondition_accessed_incorrect_time = true;
                }
                try{
                Q_Value pre_cond = ((HW && DW) && (tW >= w));
                #ifdef FUZZY
                pre_cond = tok.q_assume && pre_cond;
                #endif
                if(pre_cond) {
                    any_pre = true;
                    try{
                    Q_Value post_cond = true;
                    #ifdef FUZZY
                    post_cond = tok.q_guarantee && post_cond;
                    #endif
                    if(post_cond) {
                        auto new_clock_traces = tok.clock_traces;
                        
                        {
                        auto clockvals = &new_clock_traces.tW_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        {
                        auto clockvals = &new_clock_traces.t_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        
                        new_clock_traces.t_trace.back().reset();
                        
                        #ifdef FUZZY
                        auto new_tok = WaterTok{WaterMode::danger, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = WaterTok{WaterMode::danger, std::move(new_clock_traces)};
                        #endif
                        
                        #if(DEDUPLICATE_TOKENS)
                        next_tokens.insert(std::move(new_tok));
                        #else
                        next_tokens.emplace_back(std::move(new_tok));
                        #endif
                    }
                    } catch(InvalidTimeAccess const& time_err) {
                        postcondition_accessed_incorrect_time = true;
                    }
                }
                } catch(InvalidTimeAccess const& time_err) {
                    precondition_accessed_incorrect_time = true;
                }
                try{
                Q_Value pre_cond = (!HW && !DW);
                #ifdef FUZZY
                pre_cond = tok.q_assume && pre_cond;
                #endif
                if(pre_cond) {
                    any_pre = true;
                    try{
                    Q_Value post_cond = true;
                    #ifdef FUZZY
                    post_cond = tok.q_guarantee && post_cond;
                    #endif
                    if(post_cond) {
                        auto new_clock_traces = tok.clock_traces;
                        
                        {
                        auto clockvals = &new_clock_traces.tW_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        {
                        auto clockvals = &new_clock_traces.t_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        
                        new_clock_traces.tW_trace.back().reset();
                        
                        new_clock_traces.t_trace.back().reset();
                        
                        #ifdef FUZZY
                        auto new_tok = WaterTok{WaterMode::low, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = WaterTok{WaterMode::low, std::move(new_clock_traces)};
                        #endif
                        
                        #if(DEDUPLICATE_TOKENS)
                        next_tokens.insert(std::move(new_tok));
                        #else
                        next_tokens.emplace_back(std::move(new_tok));
                        #endif
                    }
                    } catch(InvalidTimeAccess const& time_err) {
                        postcondition_accessed_incorrect_time = true;
                    }
                }
                } catch(InvalidTimeAccess const& time_err) {
                    precondition_accessed_incorrect_time = true;
                }
                break;
            };
            
            case WaterMode::danger: {
                try{
                Q_Value pre_cond = (HW && DW);
                #ifdef FUZZY
                pre_cond = tok.q_assume && pre_cond;
                #endif
                if(pre_cond) {
                    any_pre = true;
                    try{
                    Q_Value post_cond = true;
                    #ifdef FUZZY
                    post_cond = tok.q_guarantee && post_cond;
                    #endif
                    if(post_cond) {
                        auto new_clock_traces = tok.clock_traces;
                        
                        {
                        auto clockvals = &new_clock_traces.tW_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        {
                        auto clockvals = &new_clock_traces.t_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        
                        new_clock_traces.t_trace.back().reset();
                        
                        #ifdef FUZZY
                        auto new_tok = WaterTok{WaterMode::danger, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = WaterTok{WaterMode::danger, std::move(new_clock_traces)};
                        #endif
                        
                        #if(DEDUPLICATE_TOKENS)
                        next_tokens.insert(std::move(new_tok));
                        #else
                        next_tokens.emplace_back(std::move(new_tok));
                        #endif
                    }
                    } catch(InvalidTimeAccess const& time_err) {
                        postcondition_accessed_incorrect_time = true;
                    }
                }
                } catch(InvalidTimeAccess const& time_err) {
                    precondition_accessed_incorrect_time = true;
                }
                try{
                Q_Value pre_cond = (HW && !DW);
                #ifdef FUZZY
                pre_cond = tok.q_assume && pre_cond;
                #endif
                if(pre_cond) {
                    any_pre = true;
                    try{
                    Q_Value post_cond = true;
                    #ifdef FUZZY
                    post_cond = tok.q_guarantee && post_cond;
                    #endif
                    if(post_cond) {
                        auto new_clock_traces = tok.clock_traces;
                        
                        {
                        auto clockvals = &new_clock_traces.tW_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        {
                        auto clockvals = &new_clock_traces.t_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        
                        new_clock_traces.t_trace.back().reset();
                        
                        #ifdef FUZZY
                        auto new_tok = WaterTok{WaterMode::high, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = WaterTok{WaterMode::high, std::move(new_clock_traces)};
                        #endif
                        
                        #if(DEDUPLICATE_TOKENS)
                        next_tokens.insert(std::move(new_tok));
                        #else
                        next_tokens.emplace_back(std::move(new_tok));
                        #endif
                    }
                    } catch(InvalidTimeAccess const& time_err) {
                        postcondition_accessed_incorrect_time = true;
                    }
                }
                } catch(InvalidTimeAccess const& time_err) {
                    precondition_accessed_incorrect_time = true;
                }
                break;
            };
            
        }
    
    }
    tokens = std::move(next_tokens);
    
    //check termination condition if not already terminated 
    if(!ENVIRONMENT_LOSES && !SYSTEM_LOSES) {
        if(!any_pre) {
            ENVIRONMENT_LOSES = true;
        }else if(tokens.empty()) {
            SYSTEM_LOSES = true;
        }
    }
}

std::ostream& operator<<(std::ostream& out, WaterMonitor const& monitor) {
    //inputs
    out << "WaterMonitor\n";
    
    
    //outputs
    out << " HW = " << monitor.HW << ',';
    out << " DW = " << monitor.DW << ',';
    out << " DG = " << monitor.DG << ',';
    out << " P = " << monitor.P << ',';
    out << " A = " << monitor.A << ',';
    out << " w = " << monitor.w << ',';
    out << '\n';
    //internals
    
    
    //tokens
    for(auto const& tok : monitor.tokens) {
        #ifdef FUZZY
        out << "      " << tok.mode << "    ("<<tok.q_assume<<","<<tok.q_guarantee<<")\n";
        #else
        out << "      " << tok.mode << "\n";
        #endif
        
        
        out << "        tW\n           " << tok.clock_traces.tW_trace << "\n"; 
        
        out << "        t\n           " << tok.clock_traces.t_trace << "\n"; 
        
    }
    if(monitor.precondition_accessed_incorrect_time)out << "         (precondition accessed incorrect clock history)\n";
    if(monitor.postcondition_accessed_incorrect_time)out << "         (postcondition accessed incorrect clock history)\n";
    if(monitor.SYSTEM_LOSES)out << "         (SYSTEM LOSES)\n";
    if(monitor.ENVIRONMENT_LOSES)out << "         (ENVIRONMENT LOSES)\n";
    return out;
}

bool WaterMonitor::should_stop() const {
    #if(STOP_ON_EMPTY)
    if(tokens.empty()){
        return true;
    }
    #endif
    return false;
}

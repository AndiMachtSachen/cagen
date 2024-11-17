#include "NonDetEcs.hpp"

std::ostream& operator<<(std::ostream& out, NonDetEcsMode v) {
    switch(v) {
        
        case NonDetEcsMode::open:
            return out << "open"; 
        case NonDetEcsMode::Closing:
            return out << "Closing"; 
        case NonDetEcsMode::closed:
            return out << "closed"; 
        case NonDetEcsMode::Opening:
            return out << "Opening"; 
    }
}

void NonDetEcsMonitor::advance(int t_e, int t_s) {
    std::cout << "Advance monitor by t_e = "<<t_e<<", t_s = "<<t_s<<std::endl;
    #if(DEDUPLICATE_TOKENS)
    ToksT next_toks;
    for(auto tok : tokens) {
        
        tok.clock_traces.timer_trace.back().advance(t_e, t_s);
        
        tok.clock_traces.t_trace.back().advance(t_e, t_s);
        
        next_toks.insert(std::move(tok));
    }
    tokens = std::move(next_toks);
    #else
    for(auto& tok : tokens) {
        
        tok.clock_traces.timer_trace.back().advance(t_e, t_s);
        
        tok.clock_traces.t_trace.back().advance(t_e, t_s);
        
    }
    #endif
}

void NonDetEcsMonitor::update() {
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
        
        auto timer = tok.clock_traces.timer_trace.back().total();
        auto timer_e = tok.clock_traces.timer_trace.back().env();
        auto timer_s = tok.clock_traces.timer_trace.back().sys();
        
        auto t = tok.clock_traces.t_trace.back().total();
        auto t_e = tok.clock_traces.t_trace.back().env();
        auto t_s = tok.clock_traces.t_trace.back().sys();
        
        #ifndef RINGBUFFER
        #ifndef UNBOUNDED_TRACE
        
        if(tok.clock_traces.timer_trace.size() > 1)tok.clock_traces.timer_trace.pop_front();
        if(tok.clock_traces.t_trace.size() > 1)tok.clock_traces.t_trace.pop_front();
        #endif
        
        #else
        //ring_buffer automatically overrides old values
        
        #endif
        switch(tok.mode) {
            case NonDetEcsMode::open: {
                try{
                Q_Value pre_cond = (wl <= tl);
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
                        auto clockvals = &new_clock_traces.timer_trace;
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
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::open, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::open, std::move(new_clock_traces)};
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
                Q_Value pre_cond = (wl >= tl);
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
                        auto clockvals = &new_clock_traces.timer_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        {
                        auto clockvals = &new_clock_traces.t_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        
                        new_clock_traces.timer_trace.back().reset();
                        
                        new_clock_traces.t_trace.back().reset();
                        
                        #ifdef FUZZY
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::Closing, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::Closing, std::move(new_clock_traces)};
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
            
            case NonDetEcsMode::Closing: {
                try{
                Q_Value pre_cond = !gate_closed;
                #ifdef FUZZY
                pre_cond = tok.q_assume && pre_cond;
                #endif
                if(pre_cond) {
                    any_pre = true;
                    try{
                    Q_Value post_cond = (timer < duration);
                    #ifdef FUZZY
                    post_cond = tok.q_guarantee && post_cond;
                    #endif
                    if(post_cond) {
                        auto new_clock_traces = tok.clock_traces;
                        
                        {
                        auto clockvals = &new_clock_traces.timer_trace;
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
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::Closing, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::Closing, std::move(new_clock_traces)};
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
                Q_Value pre_cond = gate_closed;
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
                        auto clockvals = &new_clock_traces.timer_trace;
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
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::closed, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::closed, std::move(new_clock_traces)};
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
            
            case NonDetEcsMode::closed: {
                try{
                Q_Value pre_cond = (wl >= tl);
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
                        auto clockvals = &new_clock_traces.timer_trace;
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
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::closed, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::closed, std::move(new_clock_traces)};
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
                Q_Value pre_cond = (wl <= tl);
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
                        auto clockvals = &new_clock_traces.timer_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        {
                        auto clockvals = &new_clock_traces.t_trace;
                        auto next_clock = clockvals->back();
                        clockvals->push_back(std::move(next_clock));
                        }
                        
                        
                        new_clock_traces.timer_trace.back().reset();
                        
                        new_clock_traces.t_trace.back().reset();
                        
                        #ifdef FUZZY
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::Opening, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::Opening, std::move(new_clock_traces)};
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
            
            case NonDetEcsMode::Opening: {
                try{
                Q_Value pre_cond = gate_closed;
                #ifdef FUZZY
                pre_cond = tok.q_assume && pre_cond;
                #endif
                if(pre_cond) {
                    any_pre = true;
                    try{
                    Q_Value post_cond = (timer < duration);
                    #ifdef FUZZY
                    post_cond = tok.q_guarantee && post_cond;
                    #endif
                    if(post_cond) {
                        auto new_clock_traces = tok.clock_traces;
                        
                        {
                        auto clockvals = &new_clock_traces.timer_trace;
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
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::Opening, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::Opening, std::move(new_clock_traces)};
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
                Q_Value pre_cond = !gate_closed;
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
                        auto clockvals = &new_clock_traces.timer_trace;
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
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::open, std::move(new_clock_traces), pre_cond, post_cond};
                        #else
                        auto new_tok = NonDetEcsTok{NonDetEcsMode::open, std::move(new_clock_traces)};
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

std::ostream& operator<<(std::ostream& out, NonDetEcsMonitor const& monitor) {
    //inputs
    out << "NonDetEcsMonitor\n";
    
    
    //outputs
    out << " wl = " << monitor.wl << ',';
    out << " tl = " << monitor.tl << ',';
    out << " gate_closed = " << monitor.gate_closed << ',';
    out << " duration = " << monitor.duration << ',';
    out << '\n';
    //internals
    
    
    //tokens
    for(auto const& tok : monitor.tokens) {
        #ifdef FUZZY
        out << "      " << tok.mode << "    ("<<tok.q_assume<<","<<tok.q_guarantee<<")\n";
        #else
        out << "      " << tok.mode << "\n";
        #endif
        
        
        out << "        timer\n           " << tok.clock_traces.timer_trace << "\n"; 
        
        out << "        t\n           " << tok.clock_traces.t_trace << "\n"; 
        
    }
    if(monitor.precondition_accessed_incorrect_time)out << "         (precondition accessed incorrect clock history)\n";
    if(monitor.postcondition_accessed_incorrect_time)out << "         (postcondition accessed incorrect clock history)\n";
    if(monitor.SYSTEM_LOSES)out << "         (SYSTEM LOSES)\n";
    if(monitor.ENVIRONMENT_LOSES)out << "         (ENVIRONMENT LOSES)\n";
    return out;
}

bool NonDetEcsMonitor::should_stop() const {
    #if(STOP_ON_EMPTY)
    if(tokens.empty()){
        return true;
    }
    #endif
    return false;
}

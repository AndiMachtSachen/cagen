package cagen.code

import cagen.*
import cagen.code.CCodeUtils.applySubst
import cagen.code.CCodeUtilsSimplified.toC
import cagen.code.CCodeUtilsSimplified.toCExpr
import java.nio.file.Path
import kotlin.io.path.createFile
import kotlin.io.path.div
import kotlin.io.path.exists
import kotlin.io.path.writeText

object CppGen {
    const val headerExtension = ".hpp"
    const val sourceExtension = ".cpp"

    private fun writeCode(folder: Path, name: String, extension : String, code: String) {
        val filename = folder / (name + extension)
        println("Write code of $name to $filename")
        if(!filename.exists()){
            folder.toFile().mkdir()
            filename.createFile()
        }
        filename.writeText(code)
    }


    fun writeRuntimeMonitor(contract: UseContract, folder: Path) {
        writeMonitorHeader(contract.contract, folder)
        writeFuzzyHeader(folder)
        writeFuzzyDefaultImpl(folder)
        writeRingBufferImpl(folder)
        writeMonitorTu(contract.contract, folder)
        writeMainTu(contract.contract, contract.variableMap, folder)
    }

    fun writeMonitorHeader(contract: Contract, folder: Path) {
        val signature = contract.signature
        val name = contract.name
        val monitorName = getMonitorName(name)
        val modeName = getModeName(name)
        val tokName = getTokenName(name)
        val clockTraceName = getClockValuationTraceName(name)

        val code = """
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
                ${contract.signature.clocks
                .filter { !it.name.isSuffixedClock() }
                .joinToString(",") {it.name}}
            };
            
            [[nodiscard]] constexpr int clock_trace_capacity(ClockId clock_id) {
            	switch(clock_id){
                    //clocks with history
                    ${contract.history.filter {
                        !it.first.isSuffixedClock() && contract.signature.clocks.any { v -> v.name == it.first }
                    }.joinToString("") { (name, depth) -> """
                    case ClockId::${name}: return 1 + $depth; """
                    }}
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
            #include "q_value$headerExtension"
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
            
            enum class $modeName {
                ${contract.states.joinToString(", ")}
            };
            std::ostream& operator<<(std::ostream& out, $modeName v);
            
            using std::map;
            using std::vector;
            using std::set;
            using std::deque; 
            using std::string;
            struct $clockTraceName{
                 ${contract.signature.clocks
                .filter { !it.name.isSuffixedClock() }
                .joinToString("") {"""
                    TraceT<ClockVal<(int)ClockId::${it.name}>, clock_trace_capacity(ClockId::${it.name})> ${it.name}_trace;"""}
                }
                
                //lexicographical comparison for token deduplication
                [[nodiscard]] bool operator<($clockTraceName const& rhs) const {
                    return std::tie(
                    ${contract.signature.clocks
                    .filter { !it.name.isSuffixedClock() }
                    .joinToString(",") {"${it.name}_trace"}
                    }) < std::tie(
                    ${contract.signature.clocks
                    .filter { !it.name.isSuffixedClock() }
                    .joinToString(",") {"rhs.${it.name}_trace"}
                    }); 
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
            struct $tokName {
                $modeName mode;
                $clockTraceName clock_traces;
                #ifdef FUZZY
                Q_Value q_assume;
                Q_Value q_guarantee;
                #endif
                
                //lexicographical comparison for token deduplication
                [[nodiscard]] bool operator<($tokName const& rhs) const {
                    #ifdef FUZZY
                    return std::tie(mode, clock_traces, q_assume, q_guarantee) < std::tie(rhs.mode, rhs.clock_traces, rhs.q_assume, rhs.q_guarantee);
                    #else
                    return std::tie(mode, clock_traces) < std::tie(rhs.mode, rhs.clock_traces);
                    #endif
                }
            };
            
            #if(DEDUPLICATE_TOKENS)
            using ToksT = std::set<$tokName>;
            #else
            using ToksT = std::vector<$tokName>;
            #endif
            
            struct $monitorName {
                //inputs
                ${signature.inputs.declareMembers()}
                //outputs
                ${signature.outputs.declareMembers()}
                //internals
                ${signature.internals.declareMembers()}
                //tokens
                ToksT tokens;
                
                //history${
                    contract.history.filter { contract.signature.clocks.none { v -> v.name == it.first } }.joinToString("") { (name, depth) ->
                        val type = contract.signature.get(name)?.type?.toC()
                        (0..depth).joinToString("") {"""
                $type h_${name}_${it}{};"""
                        }
                    }
                }
                
                bool postcondition_accessed_incorrect_time = false;
                bool precondition_accessed_incorrect_time = false;
                
                bool SYSTEM_LOSES = false;
                bool ENVIRONMENT_LOSES = false;
            
                [[nodiscard]] $monitorName() noexcept {
                    $clockTraceName initial_clock_val;
                    ${contract.signature.clocks
                    .filter { !it.name.isSuffixedClock() }
                    .toList().joinToString("") { """
                    initial_clock_val.${it.name}_trace = TraceT<ClockVal<(int)ClockId::${it.name}>, clock_trace_capacity(ClockId::${it.name})>{ClockVal<(int)ClockId::${it.name}>{}};"""}
                    }
                    tokens = ToksT{${contract.states.filter { it[0].isLowerCase() }.joinToString(", ") { 
                        "$tokName{$modeName::$it, initial_clock_val}"
                    }}};
                    
                }
                void update();
                void advance(int t_e, int t_s);
                [[nodiscard]] bool should_stop() const;
                friend std::ostream& operator<<(std::ostream& out, $monitorName const&);
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
        """.trimIndent()
        writeCode(folder, contract.name, headerExtension, code)
    }

    fun writeMonitorTu(contract: Contract, folder: Path) {
        val name = contract.name
        val monitorName = getMonitorName(name)
        val tokName = getTokenName(name)
        val modeName = getModeName(name)

        val code = """
            #include "$name$headerExtension"
            
            std::ostream& operator<<(std::ostream& out, $modeName v) {
                switch(v) {
                    ${contract.states.joinToString("") { """
                    case $modeName::$it:
                        return out << "$it"; """ }}
                }
            }
            
            void $monitorName::advance(int t_e, int t_s) {
                std::cout << "Advance monitor by t_e = "<<t_e<<", t_s = "<<t_s<<std::endl;
                #if(DEDUPLICATE_TOKENS)
                ToksT next_toks;
                for(auto tok : tokens) {
                    ${contract.signature.clocks
                    .filter { !it.name.isSuffixedClock() }
                    .joinToString("") {"""
                    tok.clock_traces.${it.name}_trace.back().advance(t_e, t_s);
                    """}}
                    next_toks.insert(std::move(tok));
                }
                tokens = std::move(next_toks);
                #else
                for(auto& tok : tokens) {
                    ${contract.signature.clocks
                    .filter { !it.name.isSuffixedClock() }
                    .joinToString("") {"""
                    tok.clock_traces.${it.name}_trace.back().advance(t_e, t_s);
                    """}}
                }
                #endif
            }
            
            void $monitorName::update() {
                postcondition_accessed_incorrect_time = false;
                precondition_accessed_incorrect_time = false;
                
                //update history                                    
                ${contract.history.filter { contract.signature.clocks.none { v -> v.name == it.first } }.joinToString("\n") { (n, d) ->
                val t = contract.signature.get(n)?.type?.toC()
                (d downTo 1).joinToString("") { """
                h_${n}_$it = h_${n}_${it - 1};""" } +"""
                h_${n}_0 = $n;
                """
                }}
                
                //update token marking
                ToksT next_tokens;
                bool any_pre = false;
                #if(DEDUPLICATE_TOKENS)
                for(auto tok : tokens) {
                #else
                for(auto& tok : tokens) {
                #endif
                    ${contract.signature.clocks
                    .filter { !it.name.isSuffixedClock() }
                    .joinToString("") {"""
                    auto ${it.name} = tok.clock_traces.${it.name}_trace.back().total();
                    auto ${it.name}_e = tok.clock_traces.${it.name}_trace.back().env();
                    auto ${it.name}_s = tok.clock_traces.${it.name}_trace.back().sys();
                    """}}
                    #ifndef RINGBUFFER
                    #ifndef UNBOUNDED_TRACE
                    ${contract.signature.clocks.filter {
                        x -> !x.name.isSuffixedClock() && contract.history.none { it.first == x.name } 
                    }.joinToString("") { """
                    if(tok.clock_traces.${it.name}_trace.size() > 1)tok.clock_traces.${it.name}_trace.pop_front();""" }}
                    #endif
                    ${contract.history.filter {
                        !it.first.isSuffixedClock() && contract.signature.clocks.any { v -> v.name == it.first } 
                    }.joinToString("") { (name, depth) -> """
                    #ifndef UNBOUNDED_TRACE
                    if(tok.clock_traces.${name}_trace.size() > $depth + 1)tok.clock_traces.${name}_trace.pop_front();
                    #endif""" +
                    (1..depth).joinToString("") {"""
                    auto h_${name}_${it} = ClockHistoryEntry<ClockKind::total,(int)ClockId::$name>(tok.clock_traces.${name}_trace, $it);
                    auto h_${envClockName(name)}_${it} = ClockHistoryEntry<ClockKind::env,(int)ClockId::$name>(tok.clock_traces.${name}_trace, $it);
                    auto h_${sysClockName(name)}_${it} = ClockHistoryEntry<ClockKind::sys,(int)ClockId::$name>(tok.clock_traces.${name}_trace, $it);"""
                    }}}
                    #else
                    //ring_buffer automatically overrides old values
                    ${contract.history.filter {
                        !it.first.isSuffixedClock() && contract.signature.clocks.any { v -> v.name == it.first }
                    }.joinToString("") { (name, depth) -> 
                    (1..depth).joinToString("") {"""
                    auto h_${name}_${it} = ClockHistoryEntry<ClockKind::total,(int)ClockId::$name>(tok.clock_traces.${name}_trace, $it);
                    auto h_${envClockName(name)}_${it} = ClockHistoryEntry<ClockKind::env,(int)ClockId::$name>(tok.clock_traces.${name}_trace, $it);
                    auto h_${sysClockName(name)}_${it} = ClockHistoryEntry<ClockKind::sys,(int)ClockId::$name>(tok.clock_traces.${name}_trace, $it);"""
                    }}}
                    #endif
                    switch(tok.mode) {
                        ${contract.transitions.groupBy { it.from }.toList().joinToString("""
                        """) { "case $modeName::${it.first}: {" +
                        it.second.joinToString("") {
                            """
                            try{
                            Q_Value pre_cond = ${it.contract.pre.toCExpr()};
                            #ifdef FUZZY
                            pre_cond = q_combine(tok.q_assume, pre_cond);
                            #endif
                            if(pre_cond) {
                                any_pre = true;
                                try{
                                Q_Value post_cond = ${it.contract.post.toCExpr()};
                                #ifdef FUZZY
                                post_cond = q_combine(tok.q_guarantee, post_cond);
                                #endif
                                if(post_cond) {
                                    auto new_clock_traces = tok.clock_traces;
                                    ${contract.signature.clocks
                                    .filter { !it.name.isSuffixedClock() }
                                    .joinToString("") {"""
                                    {
                                    auto clockvals = &new_clock_traces.${it.name}_trace;
                                    auto next_clock = clockvals->back();
                                    clockvals->push_back(std::move(next_clock));
                                    }
                                    """}}
                                    ${it.clocks.joinToString(""){"""
                                    new_clock_traces.${it}_trace.back().reset();
                                    """        
                                    }}
                                    #ifdef FUZZY
                                    auto new_tok = $tokName{$modeName::${it.to}, std::move(new_clock_traces), pre_cond, post_cond};
                                    #else
                                    auto new_tok = $tokName{$modeName::${it.to}, std::move(new_clock_traces)};
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
                            }"""
                        } + """
                            break;
                        };
                        """ }}
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
            
            std::ostream& operator<<(std::ostream& out, $monitorName const& monitor) {
                //inputs
                out << ${"\"$monitorName\\n\""};
                ${contract.signature.inputs.printVars()}
                ${if(contract.signature.inputs.isNotEmpty()){"""out << '\n';"""}else{""}}
                //outputs
                ${contract.signature.outputs.printVars()}
                ${if(contract.signature.outputs.isNotEmpty()){"""out << '\n';"""}else{""}}
                //internals
                ${contract.signature.internals.printVars()}
                ${if(contract.signature.internals.isNotEmpty()){"""out << '\n';"""}else{""}}
                //tokens
                for(auto const& tok : monitor.tokens) {
                    #ifdef FUZZY
                    out << "      " << tok.mode << "    ("<<tok.q_assume<<","<<tok.q_guarantee<<")\n";
                    #else
                    out << "      " << tok.mode << "\n";
                    #endif
                    
                    ${contract.signature.clocks
                    .filter { !it.name.isSuffixedClock() }
                    .joinToString("") {"""
                    out << "        ${it.name}\n           " << tok.clock_traces.${it.name}_trace << "\n"; 
                    """
                    }}
                }
                if(monitor.precondition_accessed_incorrect_time)out << "         (precondition accessed incorrect clock history)\n";
                if(monitor.postcondition_accessed_incorrect_time)out << "         (postcondition accessed incorrect clock history)\n";
                if(monitor.SYSTEM_LOSES)out << "         (SYSTEM LOSES)\n";
                if(monitor.ENVIRONMENT_LOSES)out << "         (ENVIRONMENT LOSES)\n";
                return out;
            }
            
            bool $monitorName::should_stop() const {
                #if(STOP_ON_EMPTY)
                if(tokens.empty()){
                    return true;
                }
                #endif
                return false;
            }
            
    """.trimIndent()
        writeCode(folder, contract.name, sourceExtension, code)
    }

    fun writeMainTu(contract: Contract, variableMap: MutableList<Pair<String, IOPort>>, folder: Path) {

        val name = contract.name
        val monitorName = getMonitorName(name)
        val tokName = getTokenName(name)
        val modeName = getModeName(name)
        val code = """ 
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
            
                #include "${contract.name}$headerExtension"
                
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
                    ${
                        variableMap.joinToString("") { (dest, src) -> """
                            kvs["$dest"] = kvs["${applySubst(src)}"];"""
                        }
                    }
                    return kvs;
                }
                                
                int main(int argc, char *argv[]) {
                    if (argc < 2) {
                        std::cerr << "Did not specify shared file name for reading" << std::endl;
                        EXIT(EXIT_FAILURE);
                    }
                    auto filename = argv[1];
                    
                    $monitorName monitor;
                    
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
                        
                        auto te = std::stoi(kvs["${envClockName(tClockName)}"]);
                        auto ts = std::stoi(kvs["${sysClockName(tClockName)}"]);
                        monitor.advance(te, ts);
                        
                        //inputs
                        ${contract.signature.inputs.readVars()}
                        //outputs
                        ${contract.signature.outputs.readVars()}
                        //internals
                        ${contract.signature.internals.readVars()}
                        
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
                """.trimIndent()
        writeCode(folder, contract.name+"_monitor", sourceExtension, code)
    }

    fun writeFuzzyHeader(folder: Path) {
        writeCode(folder, "q_value", headerExtension, qValueCode)
    }

    fun writeFuzzyDefaultImpl(folder: Path) {
        writeCode(folder, "fuzzy_impl", headerExtension, fuzzyImplCode)
    }

    fun writeRingBufferImpl(folder: Path) {
        writeCode(folder, "ring_buffer", headerExtension, ringBufferCode)
    }

    fun writeSystemTu(system: System, folder: Path) {
        val signature = system.signature
        val name = system.name

        val code = """
            #include "$name$headerExtension"
            #include "${name}Environment$headerExtension"
            #include <iostream>
            #include <fstream>
            #include <string>
            #include <thread>
            #include <chrono>

            #define EXIT(code) {std::cerr << "EXIT line " << __LINE__ << " with code " << code << std::endl;fflush(0);exit(code);}

            void ${name}_state::update_system() {${ 
                ("\n" + (system.code ?: system.toporder)).trimIndent().replace("\n", "\n                ") 
            }
            }

            void write_kvs(${name}_state const& state, char const* filename) {
                std::ofstream file(filename, std::ios::app);
                if (!file) {
                    std::cerr << "Error opening file for writing!" << std::endl;
                    EXIT(EXIT_FAILURE);
                }
                
                ${signature.inputs.writeVars()}
                ${signature.outputs.writeVars()}
                ${signature.clocks.writeVars()}
                ${signature.plainInternals.writeVars()}
                ${signature.instances.writeVars()}
                file << std::endl;
                file.close();
            }
            
            int main(int argc, char const * argv[]) {
                if(argc < 2) {
                    std::cerr << "Did not specify shared file name for writing" << std::endl;
		            EXIT(EXIT_FAILURE); 
                }
                auto const filename = argv[1];
                ${name}_state state{};
                
                auto time_before_env = std::chrono::steady_clock::now();
                while(true) {
                    // provided inputs
                    ${signature.inputs.joinToString("\n                    ") { "state.${it.name} = get_input_${it.name}();" }}
                    
                    auto const time_before_system_update = std::chrono::steady_clock::now();
                    auto const te_millis = std::chrono::duration_cast<std::chrono::milliseconds>(time_before_system_update - time_before_env);
                    
                    state.${envClockName(tClockName)} = te_millis.count();
                    state.update_system();
                    
                    auto const time_after_system_update = std::chrono::steady_clock::now();
                    time_before_env = time_after_system_update;
                    
                    auto const ts_millis = std::chrono::duration_cast<std::chrono::milliseconds>(time_after_system_update - time_before_system_update);
                    state.${sysClockName(tClockName)} = ts_millis.count();

                    write_kvs(state, filename);
                }
            }
        """.trimIndent()
        writeCode(folder, system.name, sourceExtension, code)
    }

    fun writeSystemHeader(system: System, folder: Path) {
        val code = """
            #pragma once
            #define TRUE true
            #define FALSE false
            
            ${
            system.signature.instances.map { it.type.name }.toSet()
                .joinToString("\n") { "#include \"$it.h\"" }
        }

            class ${system.name}_state {
                public:
              ${system.signature.clocks.declareMembers()}
              // Inputs
              ${system.signature.inputs.declareMembers()}
              // Outputs
              ${system.signature.outputs.declareMembers()}
              // Internals
              ${system.signature.internals.declareMembers()}
              
              ${system.name}_state() noexcept = default;
              void update_system();
            };
     
        """.trimIndent()
        writeCode(folder, system.name, headerExtension, code)
    }

    fun writeEnvironmentHeader(system: System, folder: Path) {
        val signature = system.signature
        val name = system.name

        val code = """
            #include <cstdlib>
            class Random {
                public:
            	operator int() const {
            		return std::rand();
            	}
            	operator bool() const {
            		return std::rand() % 2;
            	}
            };
            
            // input functions
            ${signature.inputs.joinToString("") { """
            auto get_input_${it.name}(){
                return Random{};
            }""" }}
        """.trimIndent()
        writeCode(folder, system.name+"Environment", headerExtension, code)
    }

    private fun Iterable<Variable>.declareMembers(nameSuffix : String = "") = joinToString("\n                ") { "${it.type.name} ${it.name+nameSuffix}{};" }

    private fun Iterable<Variable>.readVars(monitorName : String = "monitor", nameSuffix : String = "") = joinToString("\n                        ") { "$monitorName.${it.name+nameSuffix} = std::stoi(kvs[\"${it.name+nameSuffix}\"]);" }

    private fun Iterable<Variable>.printVars(monitorName : String = "monitor", nameSuffix : String = "") = joinToString("\n                ") { "out << \" ${it.name+nameSuffix} = \" << $monitorName.${it.name+nameSuffix} << ',';" }

    private fun Iterable<Variable>.writeVars(nameSuffix : String = "") = joinToString("\n                    ") { "file << \"${it.name+nameSuffix}=\" << state.${it.name+nameSuffix} << ',';" }

}
private const val ringBufferCode = """
#include <utility>
#include <cstddef>
#include <new>
#include <initializer_list>
#include <algorithm>

//STL compatible container
//only supports the member functions I need.
template <typename T, std::size_t N>
class ring_buffer {
public:

    class iterator {
    public:

        iterator(ring_buffer* buffer, size_t index, size_t count)
            : buffer_(buffer), index_(index), remaining_(count) {}

        T& operator*() { return buffer_->data()[index_]; }
        T* operator->() { return &buffer_->data()[index_]; }

        iterator& operator++() {
            index_ = (index_ + 1) % N;
            --remaining_;
            return *this;
        }

        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const iterator& other) const {
            return buffer_ == other.buffer_ && remaining_ == other.remaining_;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        ring_buffer* buffer_;
        size_t index_;
        size_t remaining_;
    };
    class const_iterator {
    public:
        
        const_iterator(ring_buffer const* buffer, size_t index, size_t count)
            : buffer_(buffer), index_(index), remaining_(count) {}

        T const& operator*() { return buffer_->data()[index_]; }
        T const* operator->() { return &buffer_->data()[index_]; }

        const_iterator& operator++() {
            index_ = (index_ + 1) % N;
            --remaining_;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const const_iterator& other) const {
            return buffer_ == other.buffer_ && remaining_ == other.remaining_;
        }
        
        bool operator<(const const_iterator& other) const {
            return remaining_ > other.remaining_;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }
        
        int operator-(const_iterator const& other) const {
            return -( remaining_ - other.remaining_);
        }

    private:
        ring_buffer const* buffer_;
        size_t index_;
        size_t remaining_;
    };

    ring_buffer() : start_(0), end_(0), count_(0) {}
    ring_buffer(std::initializer_list<T> values) : ring_buffer() {
        for(auto v : values) {
            push_back(std::move(v));
        }
    }
    ~ring_buffer() { clear(); }

    bool empty() const { return count_ == 0; }
    size_t size() const { return count_; }
    size_t count() const { return count_; }
    size_t capacity() const { return N; }

    void clear() {
        for (size_t i = 0; i < count_; ++i) {
            data()[(start_ + i) % N].~T();
        }
        count_ = 0;
        start_ = 0;
        end_ = 0;
    }

    void push_back(const T& value) {
        if (count_ == N) {
            data()[start_].~T();
            start_ = (start_ + 1) % N;
        } else {
            ++count_;
        }
        new (&data()[end_]) T(value);
        end_ = (end_ + 1) % N;
    }

    ring_buffer& operator=(const ring_buffer& other) {
        if (this != &other) {
            clear();
            for (const auto& item : other) {
                push_back(item);
            }
        }
        return *this;
    }

    const_iterator begin() const { return const_iterator(this, start_, count_); }
    const_iterator end() const { return const_iterator(this, end_, 0); }
    iterator begin() { return iterator(this, start_, count_); }
    iterator end() { return iterator(this, end_, 0); }
	T& operator[](size_t idx) {
		return data()[(start_ + idx) % N];
	}
	T const& operator[](size_t idx) const {
		return data()[(start_ + idx) % N];
	}
	T const& back() const {
		return (*this)[count_-1];
	}
	T& back() {
		return (*this)[count_-1];
	}
    bool operator<(ring_buffer const& rhs) const {
        if(size() != rhs.size()){
            return size() < rhs.size();
        }
        for(int i = 0; i < count_; ++i) {
            if((*this)[i] < rhs[i]) return true;
            if(rhs[i] < (*this)[i]) return false;
        }
        return false;
    }
private:
    alignas(T) char buffer_[N * sizeof(T)];
    size_t start_;
    size_t end_;
    size_t count_;

    T* data() { return reinterpret_cast<T*>(buffer_); }
    const T* data() const { return reinterpret_cast<const T*>(buffer_); }
};

"""
private const val fuzzyImplCode = """
#include <algorithm>
inline Q_Value negate(Q_Value const& q){
    //default to standard negation
    return Q_Value(1 - q.v);
}
inline Q_Value t_norm(Q_Value const& q1, Q_Value const& q2){
    //default to min norm
    return Q_Value(std::min(q1.v, q2.v));
}
inline Q_Value s_norm(Q_Value const& q1, Q_Value const& q2){
    //default to max norm
    return Q_Value(std::max(q1.v, q2.v));
}

template<int id, typename T1, typename T2>
auto eq_substitution(T1 v1, T2 v2) {
    //default to non-fuzzy behaviour
	return Q_Value(v1 == v2);
}
template<int id, typename T1, typename T2>
auto lt_substitution(T1 v1, T2 v2) {
	//default to non-fuzzy behaviour
	return Q_Value(v1 < v2);
}
constexpr Q_Value q_combine(Q_Value const& lhs, Q_Value const& rhs) {
    //default to t-norm
    return t_norm(lhs, rhs);
}
"""
private const val qValueCode = """
#include <iostream>
#include <type_traits>

class Q_Value {
	public:
    double v;
	
	constexpr Q_Value() noexcept : v{1} {}
	constexpr Q_Value(double v) noexcept : v(v) {}
	
	explicit operator bool() const {
		return v > 0;
	}

    friend std::ostream& operator<<(std::ostream& os, Q_Value const& q) {
        return os << "Q("<<q.v<<")";
    }
    
    bool operator<(Q_Value const& rhs) const {
        return v < rhs.v;
    }
};
#include "fuzzy_impl${CppGen.headerExtension}"

inline Q_Value operator&&(bool lhs, Q_Value const& rhs) {
	return t_norm(lhs, rhs);
}
inline Q_Value operator||(bool lhs, Q_Value const& rhs) {
	return s_norm(lhs, rhs);
}
inline Q_Value operator&(bool lhs, Q_Value const& rhs) {
	return t_norm(lhs, rhs);
}
inline Q_Value operator|(bool lhs, Q_Value const& rhs) {
	return s_norm(lhs, rhs);
}
inline Q_Value operator&&(Q_Value const& lhs, Q_Value const& rhs) {
	return t_norm(lhs, rhs);
}
inline Q_Value operator||(Q_Value const& lhs, Q_Value const& rhs) {
	return s_norm(lhs, rhs);
}
inline Q_Value operator&(Q_Value const& lhs, Q_Value const& rhs) {
	return t_norm(lhs, rhs);
}
inline Q_Value operator|(Q_Value const& lhs, Q_Value const& rhs) {
	return s_norm(lhs, rhs);
}
inline Q_Value operator&&(Q_Value const& lhs, bool rhs) {
	return t_norm(lhs, rhs);
}
inline Q_Value operator||(Q_Value const& lhs, bool rhs) {
	return s_norm(lhs, rhs);
}
inline Q_Value operator&(Q_Value const& lhs, bool rhs) {
	return t_norm(lhs, rhs);
}
inline Q_Value operator|(Q_Value const& lhs, bool rhs) {
	return s_norm(lhs, rhs);
}

template<int Id, int... Ids>
struct Contains;

template<int Id>
struct Contains<Id> : std::false_type {};

template<int Id, int First, int... Rest>
struct Contains<Id, First, Rest...> : Contains<Id, Rest...> {};

template<int Id, int... Rest>
struct Contains<Id, Id, Rest...> : std::true_type {};

template<typename Seq, int Id>
struct AddIfNotPresent;

template<int Id, int... Ids>
struct AddIfNotPresent<std::integer_sequence<int, Ids...>, Id> {
    using type = std::conditional_t<Contains<Id, Ids...>::value,
                                    std::integer_sequence<int, Ids...>,
                                    std::integer_sequence<int, Ids..., Id>>;
};

template<typename Seq1, typename Seq2>
struct CombineUnique;

template<int FirstId2, int... Ids1, int... Ids2>
struct CombineUnique<std::integer_sequence<int, Ids1...>, std::integer_sequence<int, FirstId2, Ids2...>> {
    using type = typename CombineUnique<
		typename AddIfNotPresent<
			std::integer_sequence<int, Ids1...>,
			FirstId2
		>::type,
		std::integer_sequence<int, Ids2...>
	>::type;
};

template<int... Ids>
struct CombineUnique<std::integer_sequence<int, Ids...>, std::integer_sequence<int>> {
    using type = std::integer_sequence<int, Ids...>;
};

template<typename T, int... Ids>
struct ClockVar;

template<typename T, typename Seq>
struct ClockVarT;

template<typename T, int... Ids>
struct ClockVarT<T, std::integer_sequence<int, Ids...>> {
    using type = ClockVar<T, Ids...>;
};

template<typename T, typename Seq>
using ClockVar_t = typename ClockVarT<T, Seq>::type;

template<typename T, int... Ids>
struct ClockVar {
    T value;

    ClockVar() : value{} {}
    ClockVar(T val) : value(val) {}

	/// arithmetic: ClockVar @ ClockVar

    template<typename OtherT, int... OtherIds>
    auto operator+(ClockVar<OtherT, OtherIds...> const& other) const {
        return ClockVar_t<decltype(value + other.value),typename CombineUnique<std::integer_sequence<int, Ids...>, std::integer_sequence<int, OtherIds...>>::type>(value + other.value);
    }

    template<typename OtherT, int... OtherIds>
    auto operator-(ClockVar<OtherT, OtherIds...> const& other) const {
        return ClockVar_t<decltype(value - other.value),typename CombineUnique<std::integer_sequence<int, Ids...>, std::integer_sequence<int, OtherIds...>>::type>(value - other.value);
    }

    template<typename OtherT, int... OtherIds>
    auto operator*(ClockVar<OtherT, OtherIds...> const& other) const {
        return ClockVar_t<decltype(value * other.value),typename CombineUnique<std::integer_sequence<int, Ids...>, std::integer_sequence<int, OtherIds...>>::type>(value * other.value);
    }

    template<typename OtherT, int... OtherIds>
    auto operator/(ClockVar<OtherT, OtherIds...> const& other) const {
        return ClockVar_t<decltype(value / other.value),typename CombineUnique<std::integer_sequence<int, Ids...>, std::integer_sequence<int, OtherIds...>>::type>(value / other.value);
    }

    template<typename OtherT, int... OtherIds>
    auto operator%(ClockVar<OtherT, OtherIds...> const& other) const {
        return ClockVar_t<decltype(value % other.value),typename CombineUnique<std::integer_sequence<int, Ids...>, std::integer_sequence<int, OtherIds...>>::type>(value % other.value);
    }

	/// arithmetic: ClockVar @ builtin
	
    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    auto operator+(U other) const {
        using ResultType = decltype(value + other);
        return ClockVar<ResultType, Ids...>(value + other);
    }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    auto operator-(U other) const {
        using ResultType = decltype(value - other);
        return ClockVar<ResultType, Ids...>(value - other);
    }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    auto operator*(U other) const {
        using ResultType = decltype(value * other);
        return ClockVar<ResultType, Ids...>(value * other);
    }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    auto operator/(U other) const {
        using ResultType = decltype(value / other);
        return ClockVar<ResultType, Ids...>(value / other);
    }

    template<typename U, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
    auto operator%(U other) const {
        using ResultType = decltype(value % other);
        return ClockVar<ResultType, Ids...>(value % other);
    }
	
    auto operator-() const {
        return ClockVar<T, Ids...>(-value);
    }

    friend std::ostream& operator<<(std::ostream& os, const ClockVar& w) {
		#ifdef DBG_INFO
        os << "Value: " << w.value << ", Type: "<<typeid(T).name()<<", IDs: { ";
        ((os << Ids << " "), ...);
        os << "}";
        return os;
		#else
		return os << w.value;
		#endif
    }
	
	/// predicates: ClockVar @ ClockVar
	
    template<typename OtherT, int... OtherIds>
    auto operator==(ClockVar<OtherT, OtherIds...> const& other) const {
        return (eq_substitution<Ids>(value, other.value) || ...) || (eq_substitution<OtherIds>(value, other.value) || ...); 
    }
    template<typename OtherT, int... OtherIds>
    auto operator<(ClockVar<OtherT, OtherIds...> const& other) const {
        return (lt_substitution<Ids>(value, other.value) || ...) || (lt_substitution<OtherIds>(value, other.value) || ...); 
    }
    template<typename OtherT, int... OtherIds>
    auto operator<=(ClockVar<OtherT, OtherIds...> const& other) const {
        return *this < other || *this == other; 
    }
    template<typename OtherT, int... OtherIds>
    auto operator>(ClockVar<OtherT, OtherIds...> const& other) const {
        return other < *this;
    }
    template<typename OtherT, int... OtherIds>
    auto operator>=(ClockVar<OtherT, OtherIds...> const& other) const {
        return other <= *this;
    }
    template<typename OtherT, int... OtherIds>
    auto operator!=(ClockVar<OtherT, OtherIds...> const& other) const {
        return !(*this == other);
    }
	
	/// predicates: ClockVar @ builtin
	
    template<typename U>
    auto operator==(U other) const {
        return *this == ClockVar<U>(other);
    }
    template<typename U>
    auto operator<(U other) const {
        return *this < ClockVar<U>(other);
    }
    template<typename U>
    auto operator<=(U other) const {
        return *this <= ClockVar<U>(other);
    }
    template<typename U>
    auto operator>(U other) const {
        return *this > ClockVar<U>(other);
    }
    template<typename U>
    auto operator>=(U other) const {
        return *this >= ClockVar<U>(other);
    }
    template<typename U>
    auto operator!=(U other) const {
        return *this != ClockVar<U>(other);
    }
};
/// arithmetic: builtin @ ClockVar

template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator+(U lhs, ClockVar<T, Ids...> const& rhs) {
    using ResultType = decltype(lhs + rhs.value);
    return ClockVar<ResultType, Ids...>(lhs + rhs.value);
}

template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator-(U lhs, ClockVar<T, Ids...> const& rhs) {
    using ResultType = decltype(lhs - rhs.value);
    return ClockVar<ResultType, Ids...>(lhs - rhs.value);
}

template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator*(U lhs, ClockVar<T, Ids...> const& rhs) {
    using ResultType = decltype(lhs * rhs.value);
    return ClockVar<ResultType, Ids...>(lhs * rhs.value);
}

template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator/(U lhs, ClockVar<T, Ids...> const& rhs) {
    using ResultType = decltype(lhs / rhs.value);
    return ClockVar<ResultType, Ids...>(lhs / rhs.value);
}

template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator%(U lhs, ClockVar<T, Ids...> const& rhs) {
    using ResultType = decltype(lhs % rhs.value);
    return ClockVar<ResultType, Ids...>(lhs % rhs.value);
}

/// predicates: builtin @ ClockVar

template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator==(U lhs, ClockVar<T, Ids...> const& rhs) {
	return rhs == lhs;
}
template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator<=(U lhs, ClockVar<T, Ids...> const& rhs) {
	return rhs >= lhs;
}
template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator>=(U lhs, ClockVar<T, Ids...> const& rhs) {
	return rhs <= lhs;
}
template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator<(U lhs, ClockVar<T, Ids...> const& rhs) {
	return rhs > lhs;
}
template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator>(U lhs, ClockVar<T, Ids...> const& rhs) {
	return rhs < lhs;
}
template<typename U, typename T, int... Ids, typename = std::enable_if_t<std::is_arithmetic_v<U>>>
auto operator!=(U lhs, ClockVar<T, Ids...> const& rhs) {
	return rhs != lhs;
}
"""

private fun List<CATransition>.incomingList(): List<Pair<String, List<CATransition>>> = this.groupBy { it.to }.toList()

const val tClockName = "t"
const val envClockNameSuffix = "_e"
const val sysClockNameSuffix = "_s"
fun envClockName(clockName : String) = clockName + envClockNameSuffix
fun sysClockName(clockName : String) = clockName + sysClockNameSuffix

fun String.isSuffixedClock():Boolean = this.endsWith(envClockNameSuffix) || this.endsWith(sysClockNameSuffix)

fun getMonitorName(rcaName : String) = rcaName + "Monitor"
fun getModeName(rcaName : String) = rcaName + "Mode"
fun getTokenName(rcaName : String) = rcaName + "Tok"
fun getClockValuationTraceName(rcaName : String) = rcaName + "ClockValTrace"
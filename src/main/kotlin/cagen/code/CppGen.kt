package cagen.code

import cagen.*
import cagen.code.CCodeUtils.applySubst
import cagen.code.CCodeUtilsSimplified.toC
import java.nio.file.Path
import kotlin.io.path.createFile
import kotlin.io.path.div
import kotlin.io.path.exists
import kotlin.io.path.writeText

object CppGen {
    val headerExtension = ".hpp"
    val sourceExtension = ".cpp"

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
        writeMonitorTu(contract.contract, folder)
        writeMainTu(contract.contract, contract.variableMap, folder)
    }

    fun writeMonitorHeader(contract: Contract, folder: Path) {
        val signature = contract.signature
        val name = contract.name
        val structName = name+"_state"
        val code = """
            #pragma once

            #define TRUE true
            #define FALSE false
            #include <iostream>
            
            struct $structName {
                //inputs
                ${signature.inputs.declareMembers()}
                //outputs
                ${signature.outputs.declareMembers()}
                //internals
                ${signature.internals.declareMembers()}
                //states
                bool ${contract.states.joinToString(", ") {"$it = true"}};
                //history${
            contract.history.joinToString("") { (n, d) ->
                val type = contract.signature.get(n)?.type?.toC()
                (0..d).joinToString("") {
                    """
                $type h_${n}_${it}{};
                """
                }
            }
        }
                
                bool SYSTEM_LOSES = false;
                bool ENVIRONMENT_LOSES = false;
            
                [[nodiscard]] constexpr $structName() noexcept {
                    ${contract.states.joinToString("") { """
                    $it = ${!it.startsWith("init")};"""}}
                }
                void update();
                friend std::ostream& operator<<(std::ostream& out, $structName const&);
            };
        """.trimIndent()
        writeCode(folder, contract.name, headerExtension, code)
    }

    fun writeMonitorTu(contract: Contract, folder: Path) {
        val name = contract.name
        val structName = name+"_state"
        val stateVars = contract.signature.inputs + contract.signature.outputs

        val code = """
            #include "$name$headerExtension"
            
            void $structName::update() {
                bool any_pre = false;
            
                //update history                                    
                ${contract.history.joinToString("\n") { (n, d) ->
            val t = contract.signature.get(n)?.type?.toC()
            (d downTo 1).joinToString("") { """
                h_${n}_$it = h_${n}_${it - 1};""" } +"""
                h_${n}_0 = $n;
                """
        }}
                
                //eval pre and post conditions
                ${contract.transitions.joinToString("\n") {"""
                bool const pre_${it.name} = ${it.contract.pre.inState(stateVars, "")};
                bool const post_${it.name} = ${it.contract.post.inState(stateVars, "")};
                """
        }
        }
                //update states
                ${contract.transitions.incomingList().joinToString("") { 
                    val (nextState, incomingTransitions) = it;
                    """
                    bool const next_$nextState = """ + incomingTransitions.joinToString(" | ") {
                        "${it.from} & pre_${it.name} & post_${it.name}"
                    } + ";"
                }}
                
                ${contract.transitions.incomingList().joinToString("") {
                    val (nextState, incomingTransitions) = it;
                    """
                        any_pre |= """ + incomingTransitions.joinToString(" | ") {
                        "${it.from} & pre_${it.name}"
                    } + ";"
                }}
                
                    ${
            contract.states.joinToString("") {"""
                    $it = next_$it;"""
            }
        }
                bool const any_contracts = ${contract.states.joinToString(" | ") {it}};
                
                if(!ENVIRONMENT_LOSES && !SYSTEM_LOSES){
                    if(!any_pre){
                        ENVIRONMENT_LOSES = true;
                    }else if(!any_contracts){
                        SYSTEM_LOSES = true;
                    }
                }
            }
            
            std::ostream& operator<<(std::ostream& out, $structName const& state) {
                //inputs
                out << ${"\"$structName\\n\""};
                ${contract.signature.inputs.printVars()}
                ${if(contract.signature.inputs.isNotEmpty()){"""out << '\n';"""}else{""}}
                //outputs
                ${contract.signature.outputs.printVars()}
                ${if(contract.signature.outputs.isNotEmpty()){"""out << '\n';"""}else{""}}
                //internals
                ${contract.signature.internals.printVars()}
                ${if(contract.signature.internals.isNotEmpty()){"""out << '\n';"""}else{""}}
                //states
                ${contract.states.joinToString("\n                ") {"if(state.$it)out << \"         State: $it\\n\";"}}
                if(state.SYSTEM_LOSES)out << "         (SYSTEM LOSES)\n";
                if(state.ENVIRONMENT_LOSES)out << "         (ENVIRONMENT LOSES)\n";
                return out;
            }
    """.trimIndent()
        writeCode(folder, contract.name, sourceExtension, code)
    }

    fun writeMainTu(contract: Contract, variableMap: MutableList<Pair<String, IOPort>>, folder: Path) {
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
                
                #define EXIT(code) {std::cerr << "EXIT line " << __LINE__ << " with code " << code << std::endl;fflush(0);exit(code);}
                
                std::vector<std::string> split(const std::string& str, char delimiter) {
                    std::vector<std::string> tokens;
                    std::string token;
                    std::istringstream tokenStream(str);
                    while (std::getline(tokenStream, token, delimiter)) {
                        tokens.push_back(token);
                    }
                    return tokens;
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
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
                    
                    ${contract.name}_state state;
                    
                    int iteration = 0;
                    while (true) {
                        auto kvs = read_kvs(filename);
                        std::cout << "-------------------------------------------------------\n";
                        for (const auto& kv : kvs) {
                            std::cout << kv.first << " = " << kv.second << ", ";
                        }
                        std::cout << std::endl;
                        
                        auto te = std::stoi(kvs["te"]);
                        auto ts = std::stoi(kvs["ts"]);
                        
                        //inputs
                        ${contract.signature.inputs.readVars()}
                        //outputs
                        ${contract.signature.outputs.readVars()}
                        //internals
                        ${contract.signature.internals.readVars()}
                        
                        //process
                        state.update();
                        std::cout << state << '\n' << std::endl;
                    }
                    
                }
                """.trimIndent()
        writeCode(folder, contract.name+"_monitor", sourceExtension, code)
    }

    fun writeSystemTu(system: System, folder: Path) {
        val signature = system.signature
        val name = system.name

        val code = """
            #include "$name$headerExtension"
            #include <iostream>
            #include <fstream>
            #include <string>
            #include <thread>
            #include <chrono>

            #define EXIT(code) {std::cerr << "EXIT line " << __LINE__ << " with code " << code << std::endl;fflush(0);exit(code);}

            class Random {
                public:
            	operator int() const {
            		return std::rand();
            	}
            	operator bool() const {
            		return std::rand() % 2;
            	}
            };
            
            void ${name}_state::update_system() {
                ${ system.code ?: system.toporder }
            }

            void write_kvs(${name}_state const& state, char const* filename) {
                std::ofstream file(filename, std::ios::app);
                if (!file) {
                    std::cerr << "Error opening file for writing!" << std::endl;
                    EXIT(EXIT_FAILURE);
                }
                
                ${signature.inputs.writeVars()}
                ${signature.outputs.writeVars()}
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
                    // provide random inputs
                    ${signature.inputs.joinToString("\n                    ") { "state.${it.name} = Random{};" }}
                    
                    auto const time_before_system_update = std::chrono::steady_clock::now();
                    auto const te_millis = std::chrono::duration_cast<std::chrono::milliseconds>(time_before_system_update - time_before_env);
                    
                    state.te = te_millis.count();
                    state.update_system();
                    
                    auto const time_after_system_update = std::chrono::steady_clock::now();
                    time_before_env = time_after_system_update;
                    
                    auto const ts_millis = std::chrono::duration_cast<std::chrono::milliseconds>(time_after_system_update - time_before_system_update);
                    state.ts = ts_millis.count();

                    std::cout << "t=" << state.tick << ", v=" << state.val << ", d=" << state.down << std::endl;
                    write_kvs(state, filename);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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
              // Inputs
              ${system.signature.inputs.declareMembers()}
              // Outputs
              ${system.signature.outputs.declareMembers()}
              // Internals
              ${system.signature.internals.declareMembers()}
              
              constexpr ${system.name}_state() noexcept = default;
              void update_system();
            };
     
        """.trimIndent()
        writeCode(folder, system.name, headerExtension, code)
    }

    private fun Iterable<Variable>.declareMembers() = joinToString("\n                ") { "${it.type.name} ${it.name}{};" }

    private fun Iterable<Variable>.readVars() = joinToString("\n                        ") { "state.${it.name} = std::stoi(kvs[\"${it.name}\"]);" }

    private fun Iterable<Variable>.printVars() = joinToString("\n                ") { "out << \" ${it.name} = \" << state.${it.name} << ',';" }

    private fun Iterable<Variable>.writeVars() = joinToString("\n                    ") { "file << \"${it.name}=\" << state.${it.name} << ',';" }

}

private fun List<CATransition>.incomingList(): List<Pair<String, List<CATransition>>> = this.groupBy { it.to }.toList()

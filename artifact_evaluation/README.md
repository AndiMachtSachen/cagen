# cagen RV TCA: Artifact Evaluation FASE 26

## Getting Started

This artifact contains the implementation code for the prototype tool mentioned in the paper `Timed Contract Automata`.
The kotlin code can be compiled with the shipped gradle, and the generated C++ code requires a C++17 compliant compiler (see Step-by-Step).
Each .sys file contains the definition of the respective TCA (`contract`) and a possible system implementation (`reactor`) to generate IO-compatible TIO-traces.

## Step-by-step

claim: The tool is able to check system conformance for a given TIO-trace
How to reproduce:
Compile the tool with Gradle by running
```sh
$ ./gradlew shadowJar
```
in the project directory.
To generate the program code run 
```sh
$ java -jar ../../build/libs/cagen-all.jar rca Ecs.sys
```
in the `examples/ecs` directory.
This generates the c++ source and header files for the monitor in the `rca_output` subdirectory.
It also generates a trace-compatible system implementation from the provided c++ implementation snippet in the .sys file's `reactor` to generate possible TIO-traces.
Compile the generated C++ code by running
```sh
cd rca_output/
clang++ SafeEcs.cpp SafeEcs_monitor.cpp -std=c++17 -o safe_ecs
```
to create the monitor executable (tested using clang 20.1.8).
When running the monitor, provide a filepath, e.g.,
```sh
./safe_ecs trace_ok.txt
```
which contains the TIO-trace of the play to check.
The monitor should display the read trace values, the clock values for each iteration, and the outcome of the play (for the game between environment and system).
We provide pre-generated traces in
    - trace_delay.txt (The gate operation is slowed down -- the environment loses)
    - trace_fluc.txt (Water level does not conform to assummptions (fluctuates around threshold, does not allow correct operation) -- the environment loses)
    - trace_ok.txt (every iteration's values conform to assumptions and guarantees -- the play does not terminate)
    - trace_operate.txt (system sends an incorrect operate signal -- the system loses)

The TCAs shown in the paper are:
    - event based gas burner automaton GB_1 (Fig. 1): examples/gasburner/Burner.sys (`contract BurnerControllerEvent`)
    - sampling gas burner automaton GB_2 (Fig. 3): examples/gasburner/Burner.sys (`contract BurnerControllerSample`)
    - floodgate TCA (Fig. 4): examples/ecs/Ecs.sys (`contract EcsOperate`)


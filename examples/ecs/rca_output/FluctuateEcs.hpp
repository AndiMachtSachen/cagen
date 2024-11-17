#pragma once
#define TRUE true
#define FALSE false



class FluctuateEcs_state {
    public:
  
  // Inputs
  int chw{};
  // Outputs
  int wl{};
    int tl{};
    int duration{};
    bool gate_closed{};
    bool operate{};
    int t_e{};
    int t_s{};
  // Internals
  
  
  FluctuateEcs_state() noexcept = default;
  void update_system();
};

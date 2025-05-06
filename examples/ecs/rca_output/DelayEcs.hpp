#pragma once
#define TRUE true
#define FALSE false



class DelayEcs_state {
    public:
  
  // Inputs
  int chw{};
  // Outputs
  int wl{};
    int tl{};
    int duration{};
    bool gate_closed{};
    bool gate_closed_next_1{};
    bool gate_closed_next_2{};
    int target_lvl{};
    bool operate{};
    int t_e{};
    int t_s{};
  // Internals
  
  
  DelayEcs_state() noexcept = default;
  void update_system();
};

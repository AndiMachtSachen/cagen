#pragma once
#define TRUE true
#define FALSE false



class Ecs_state {
    public:
  
  // Inputs
  int chw{};
  // Outputs
  int wl{};
    int tl{};
    int duration{};
    bool gate_closed{};
    bool operate{};
    int target_lvl{};
    int t_e{};
    int t_s{};
  // Internals
  
  
  Ecs_state() noexcept = default;
  void update_system();
};

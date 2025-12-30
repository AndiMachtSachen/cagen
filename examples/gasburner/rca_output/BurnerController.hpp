#pragma once
#define TRUE true
#define FALSE false



class BurnerController_state {
    public:
  
  // Inputs
  
  // Outputs
  int steps{};
    bool operate{};
    bool shutoff{};
    int t_e{};
    int t_s{};
  // Internals
  bool leak{};
  
  BurnerController_state() noexcept = default;
  void update_system();
};

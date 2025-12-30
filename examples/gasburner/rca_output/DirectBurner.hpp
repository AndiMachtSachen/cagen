#pragma once
#define TRUE true
#define FALSE false



class DirectBurner_state {
    public:
  
  // Inputs
  
  // Outputs
  int t_e{};
    int t_s{};
  // Internals
  bool leak{};
  
  DirectBurner_state() noexcept = default;
  void update_system();
};

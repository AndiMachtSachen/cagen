#pragma once
#define TRUE true
#define FALSE false



class Burner_state {
    public:
  
  // Inputs
  
  // Outputs
  int steps{};
    int t_e{};
    int t_s{};
  // Internals
  bool leak{};
  
  Burner_state() noexcept = default;
  void update_system();
};

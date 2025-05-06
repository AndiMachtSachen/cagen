#pragma once
#define TRUE true
#define FALSE false



class MinePump_state {
    public:
  
  // Inputs
  
  // Outputs
  bool HW{};
    bool DW{};
    bool DG{};
    bool P{};
    bool A{};
    int d{};
    int e{};
    int g{};
    int kappa{};
    int w{};
    int wl{};
    int t_e{};
    int t_s{};
  // Internals
  
  
  MinePump_state() noexcept = default;
  void update_system();
};

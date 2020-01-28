@0xf0a0f58dc1d1e0c0;

struct Control {
    linearSpeed @0 :Float64;
    angularSpeed @1 :Float64;
}

struct Dynamics {
    linearSpeed @0 :Float64;
    angularSpeed @1 :Float64;
    linearAcceleration @2 :Float64;
    angularAcceleration @3 :Float64;
}

interface Ev3Control {
  command @0 (cmd :Control);
  state @1 () -> (state :Dynamics);
}
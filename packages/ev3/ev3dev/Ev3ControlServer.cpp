#include "packages/ev3/ev3dev/ev3control.capnp.h"
#include <capnp/ez-rpc.h>
#include <capnp/message.h>
#include <iostream>
#include "ev3dev.h"
#include <unistd.h>

const uint MAX_SPEED = 1000;
ev3dev::medium_motor motor(ev3dev::OUTPUT_A);

class Ev3ControlServer final : public Ev3Control::Server
{
public:    
    
    ::kj::Promise<void> command(CommandContext context) override
    {
        auto cmd = context.getParams().getCmd();
        // std::cout << "command " << cmd.getLinearSpeed() << " " << cmd.getAngularSpeed() << std::endl;

        motor.set_speed_sp(MAX_SPEED * cmd.getLinearSpeed()).run_forever();
        
        return kj::READY_NOW;
    }

    ::kj::Promise<void> state(StateContext context) override {
        ::capnp::MallocMessageBuilder message;

        Dynamics::Builder state = message.initRoot<Dynamics>();
        state.setLinearSpeed(0.0);
        state.setAngularSpeed(0.0);
        state.setLinearAcceleration(0.0);
        state.setAngularAcceleration(0.0);
        context.getResults().setState(state);
        // std::cout << "state " << state.getLinearSpeed() << " " << state.getAngularSpeed() << std::endl;
        return kj::READY_NOW;
    }
};

//---------------------------------------------------------------------------
void precondition(bool cond, const std::string &msg) {
    if (!cond) throw std::runtime_error(msg);
}

int main(int argc, const char *argv[])
{
    // We expect one argument specifying the address to which
    // to bind and accept connections.
    if (argc != 2)
    {
        std::cerr << "usage: "
                  << "ev3_control_server"
                  << " ADDRESS[:PORT]"
                  << std::endl;
        return 1;
    }

    precondition(motor.connected(), "Motor not connected");

    // Set up the EzRpcServer, binding to port 5923 unless a
    // different port was specified by the user.  Note that the
    // first parameter here can be any "Client" object or anything
    // that can implicitly cast to a "Client" object.  You can even
    // re-export a capability imported from another server.
    capnp::EzRpcServer server(kj::heap<Ev3ControlServer>(), argv[1], 5923);

    auto &waitScope = server.getWaitScope();
    std::cout << "running on "
                  << argv[1]
                  << std::endl;
    //   // Run forever, accepting connections and handling requests.
    kj::NEVER_DONE.wait(waitScope);
}
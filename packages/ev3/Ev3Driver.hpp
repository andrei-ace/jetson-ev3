#pragma once

#include "engine/alice/alice_codelet.hpp"
#include "messages/messages.hpp"
#include <capnp/ez-rpc.h>
#include <capnp/message.h>

#include "packages/ev3/ev3dev/ev3control.capnp.h"

namespace isaac
{
namespace drivers
{
class Ev3;
}
}; // namespace isaac

namespace isaac
{
class Ev3Driver : public isaac::alice::Codelet
{
public:
    void start() override;
    void tick() override;
    void stop() override;

    ISAAC_PROTO_RX(StateProto, ev3_cmd);

    ISAAC_PROTO_TX(StateProto, ev3_state);

    ISAAC_PARAM(std::string, address, "localhost");
    ISAAC_PARAM(int, port, 9000);

private:
    alice::Failsafe* failsafe_;
    
    ::kj::Promise<void> sendCommand(double linearSpeed, double angularSpeed, Ev3Control::Client*); 
};
} // namespace isaac

ISAAC_ALICE_REGISTER_CODELET(isaac::Ev3Driver);
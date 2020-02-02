#include "Ev3Driver.hpp"
#include "packages/ev3/ev3dev/ev3control.capnp.h"
#include "engine/alice/components/Failsafe.hpp"
#include "engine/gems/state/io.hpp"
#include "messages/state/differential_base.hpp"

namespace isaac
{

::kj::Promise<void> sendCommand(double linearSpeed, double angularSpeed, Ev3Control::Client *ev3Control);

void Ev3Driver::start()
{
    failsafe_ = node()->getComponent<alice::Failsafe>();
    tickBlocking();
}

void Ev3Driver::tick()
{
    capnp::EzRpcClient client(get_address(), get_port());
    Ev3Control::Client ev3Control = client.getMain<Ev3Control>();
    auto &waitScope = client.getWaitScope();
    if (rx_ev3_cmd().available())
    {
        messages::DifferentialBaseControl command;
        ASSERT(FromProto(rx_ev3_cmd().getProto(), rx_ev3_cmd().buffers(), command),
               "Failed to parse rx_ev3_cmd");
        if (command.linear_speed() || command.angular_speed())
        {
            LOG_INFO("CMD available ls=%F as=%F", command.linear_speed(), command.angular_speed());
        }

        auto safeCmdPromise = sendCommand(command.linear_speed(), command.angular_speed(), &ev3Control);

        safeCmdPromise.wait(waitScope);
    }

    {
        auto request = ev3Control.stateRequest();
        auto statePromise = request.send();

        auto safeStatePromise = statePromise.then([this](capnp::Response<Ev3Control::StateResults> response) { 
                messages::DifferentialBaseDynamics ev3_state;
                ev3_state.linear_speed() = response.getState().getLinearSpeed();
                ev3_state.angular_speed() = response.getState().getAngularSpeed();
                ev3_state.linear_acceleration() = response.getState().getLinearAcceleration();
                ev3_state.angular_acceleration() = response.getState().getAngularAcceleration();

                ToProto(ev3_state, tx_ev3_state().initProto(), tx_ev3_state().buffers());
                tx_ev3_state().publish();
                return; },[](kj::Exception &&exception) {
                                                      LOG_ERROR("state %s", exception.getDescription());
                                                      return;
                                                  });

        safeStatePromise.wait(waitScope);
    }

    // stop robot if the failsafe is triggered
    if (!failsafe_->isAlive())
    {
        auto safeCmdPromise = sendCommand(0, 0, &ev3Control);
        safeCmdPromise.wait(waitScope);
    }
}

::kj::Promise<void> sendCommand(double linearSpeed, double angularSpeed, Ev3Control::Client *ev3Control)
{
    ::capnp::MallocMessageBuilder message;

    auto request = ev3Control->commandRequest();
    Control::Builder cmd = message.initRoot<Control>();
    cmd.setLinearSpeed(linearSpeed);
    cmd.setAngularSpeed(angularSpeed);
    request.setCmd(cmd);

    auto cmdPromise = request.send().ignoreResult();

    auto safeCmdPromise = cmdPromise.catch_([](kj::Exception &&exception) {
        LOG_ERROR("command %s", exception.getDescription());
        return;
    });

    return safeCmdPromise;
}

void Ev3Driver::stop()
{
    capnp::EzRpcClient client(get_address(), get_port());
    Ev3Control::Client ev3Control = client.getMain<Ev3Control>();
    auto &waitScope = client.getWaitScope();
    auto safeCmdPromise = sendCommand(0, 0, &ev3Control);
    safeCmdPromise.wait(waitScope);
}
} // namespace isaac
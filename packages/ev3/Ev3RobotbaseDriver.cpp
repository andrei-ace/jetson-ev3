#include "Ev3RobotbaseDriver.hpp"

#include "engine/gems/state/io.hpp"
#include "messages/state/differential_base.hpp"

namespace isaac
{
void Ev3RobotbaseDriver::start()
{
    tickBlocking();
}

void Ev3RobotbaseDriver::tick()
{
    if (rx_ev3_cmd().available())
    {
        messages::DifferentialBaseControl command;
        ASSERT(FromProto(rx_ev3_cmd().getProto(), rx_ev3_cmd().buffers(), command),
               "Failed to parse rx_ev3_cmd");
        LOG_INFO("CMD available");
    }

    messages::DifferentialBaseDynamics ev3_state;
    ev3_state.linear_speed() = 0;
    ev3_state.angular_speed() = 0;
    ev3_state.linear_acceleration() = 0;
    ev3_state.angular_speed() = 0;

    ToProto(ev3_state, tx_ev3_state().initProto(), tx_ev3_state().buffers());
    tx_ev3_state().publish();
}

void Ev3RobotbaseDriver::stop()
{
}
} // namespace isaac
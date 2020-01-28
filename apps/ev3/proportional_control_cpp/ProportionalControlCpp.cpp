/*
Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#include "ProportionalControlCpp.hpp"

#include "engine/gems/state/io.hpp"
#include "messages/math.hpp"
#include "messages/state/differential_base.hpp"

namespace isaac {

void ProportionalControlCpp::start() {
  // This part will be run once in the beginning of the program

  // Print some information
  LOG_INFO("Please head to the Sight website at <IP>:<PORT> to see how I am doing.");
  LOG_INFO("<IP> is the Internet Protocol address where the app is running,");
  LOG_INFO("and <PORT> is set in the config file, typically to '3000'.");
  LOG_INFO("By default, local link is 'localhost:3000'.");

  // We can tick periodically, on every message, or blocking. See documentation for details.
  tickPeriodically();
}

void ProportionalControlCpp::tick() {
  // This part will be run at every tick. We are ticking periodically in this example.

  // Nothing to do if we haven't received odometry data yet
  if (!rx_odometry().available()) {
    return;
  }

  // Read parameters that can be set through Sight webpage
  const double reference = get_desired_position_meters();
  const double gain = get_gain();

  // Read odometry message received
  const auto& odom_reader = rx_odometry().getProto();
  const Pose2d odometry_T_robot = FromProto(odom_reader.getOdomTRobot());
  const double position = odometry_T_robot.translation.x();

  // Compute the control action
  const double control = gain * (reference - position);

  // Show some data in Sight
  show("reference (m)", reference);
  show("position (m)", position);
  show("control", control);
  show("gain", gain);

  // Publish control command
  messages::DifferentialBaseControl command;
  command.linear_speed() = control;
  command.angular_speed() = 0.0;  // This simple example sets zero angular speed
  ToProto(command, tx_cmd().initProto(), tx_cmd().buffers());
  tx_cmd().publish();
}

}  // namespace isaac

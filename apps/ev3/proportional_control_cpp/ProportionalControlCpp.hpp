/*
Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#pragma once

#include "engine/alice/alice.hpp"
#include "messages/messages.hpp"

namespace isaac {

// A C++ codelet for proportional control
//
// We receive odometry information, from which we extract the x position. Then, using refence and
// gain parameters that are provided by the user, we compute and publish a linear speed command
// using `control = gain * (reference - position)`
class ProportionalControlCpp : public alice::Codelet {
 public:
  // Has whatever needs to be run in the beginning of the program
  void start() override;
  // Has whatever needs to be run repeatedly
  void tick() override;

  // List of messages this codelet recevies
  ISAAC_PROTO_RX(Odometry2Proto, odometry);
  // List of messages this codelet transmits
  ISAAC_PROTO_TX(StateProto, cmd);

  // Gain for the proportional controller
  ISAAC_PARAM(double, gain, 1.0);
  // Reference for the controller
  ISAAC_PARAM(double, desired_position_meters, 1.0);
};

}  // namespace isaac

ISAAC_ALICE_REGISTER_CODELET(isaac::ProportionalControlCpp);

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

// A simple C++ codelet which receives a ping and reacts to it
class Pong : public alice::Codelet {
 public:
  void start() override;
  void tick() override;

  // An incoming message channel on which we receive pings.
  ISAAC_PROTO_RX(PingProto, trigger);

  ISAAC_PARAM(std::string, address, "localhost");
  ISAAC_PARAM(int, port, 9000);
};

}  // namespace isaac

ISAAC_ALICE_REGISTER_CODELET(isaac::Pong);

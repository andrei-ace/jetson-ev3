/*
Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#include "Ping.hpp"

#include "engine/core/logger.hpp"

namespace isaac {

void Ping::start() {
  tickPeriodically();
}

void Ping::tick() {
  // Create and publish a ping message
  auto proto = tx_ping().initProto();
  // We send a different ping each time we tick. `getTickCount()` will give us the number of times
  // this codelet ticked.
  proto.setMessage("ping_" + std::to_string(getTickCount()));
  tx_ping().publish();

  LOG_INFO("Sent a ping");
}

}  // namespace isaac

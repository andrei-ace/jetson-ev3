/*
Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "Pong.hpp"

#include <string>

#include "ping.capnp.h"
#include <capnp/ez-rpc.h>
#include <capnp/message.h>
#include <iostream>

namespace isaac
{

void Pong::start()
{
  // By using tickOnMessage instead of tickPeriodically we instruct the codelet to only tick when
  // a new message is received on the incoming data channel `trigger`.
  tickOnMessage(rx_trigger());
}

void Pong::tick()
{
  // This function will now only be executed whenever we receive a new message. This is guaranteed
  // by the Isaac Robot Engine.

  // Parse the message we received
  auto proto = rx_trigger().getProto();
  const std::string message = proto.getMessage();

  capnp::EzRpcClient client(get_address()+":"+std::to_string(get_port()));
  Ev3Ping::Client ping = client.getMain<Ev3Ping>();
  auto &waitScope = client.getWaitScope();
  {
    std::cout << "Sending ping... ";
    std::cout.flush();

    auto request = ping.pingRequest();
    request.setPing("Ping");

    // Send it, which returns a promise for the result (without blocking).
    auto pingPromise = request.send();

    auto response = pingPromise.wait(waitScope);
    std::cout << response.getValue().cStr() << std::endl;
  }

}

} // namespace isaac

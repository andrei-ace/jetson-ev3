#pragma once

#include "LidarAngleChanger.hpp"

#include "engine/alice/alice.hpp"
#include "messages/messages.hpp"

namespace isaac {
namespace ev3{

class LidarAngleChanger : public isaac::alice::Codelet {
 public:

  void start() override;
  void stop() override;
  void tick() override;

  ISAAC_PROTO_RX(FlatscanProto, scan);
  ISAAC_PROTO_TX(FlatscanProto, flatscan);

};

}  // namespace ev3
}  // namespace benchbot

ISAAC_ALICE_REGISTER_CODELET(isaac::ev3::LidarAngleChanger);
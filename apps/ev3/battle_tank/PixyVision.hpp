#pragma once

#include <string>
#include <signal.h>

#include "libpixyusb2.h"

#include "engine/alice/alice.hpp"
#include "messages/messages.hpp"

namespace isaac {
namespace ev3 {

// A codelet that uses a Pixy2 Camera for detecting objects
class PixyVision : public alice::Codelet {
 public:
  void start() override;
  void tick() override;
  void stop() override;

 private:
  Pixy2 pixy;
  uint32_t index_frame = 0;
};
} // namespace ev3
} // namespace isaac

ISAAC_ALICE_REGISTER_CODELET(isaac::ev3::PixyVision);

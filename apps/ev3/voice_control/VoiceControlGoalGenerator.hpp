#pragma once

#include <string>

#include "engine/alice/alice.hpp"
#include "messages/messages.hpp"

namespace isaac
{

class VoiceControlGoalGenerator : public alice::Codelet
{
public:
  void start() override;
  void tick() override;

  ISAAC_PROTO_RX(VoiceCommandDetectionProto, detected_command);
  
  ISAAC_PROTO_TX(Goal2Proto, goal);
  ISAAC_PROTO_RX(Goal2FeedbackProto, feedback);

private:
  void publish_goal(Pose2d);
};

} // namespace isaac

ISAAC_ALICE_REGISTER_CODELET(isaac::VoiceControlGoalGenerator);

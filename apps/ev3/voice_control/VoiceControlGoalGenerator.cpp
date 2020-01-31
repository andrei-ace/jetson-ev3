#include "VoiceControlGoalGenerator.hpp"

#include "messages/math.hpp"


namespace isaac {

void VoiceControlGoalGenerator::start() {
  tickOnMessage(rx_voice_command_id());
}

void VoiceControlGoalGenerator::tick() {
  LOG_INFO("Message");
  auto proto = rx_voice_command_id().getProto();
  int id = proto.getCommandId();
  auto goal_proto = tx_goal().initProto();
  switch (id)
  {
  case 1:
    goal_proto.setStopRobot(true);
    goal_proto.setTolerance(0.1);
    goal_proto.setGoalFrame("robot");
    ToProto(Pose2d::Rotation(90), goal_proto.initGoal());
    LOG_INFO("Go Left");
    tx_goal().publish();
    break;
  case 2:
    goal_proto.setStopRobot(true);
    goal_proto.setTolerance(0.1);
    goal_proto.setGoalFrame("robot");
    ToProto(Pose2d::Rotation(-90), goal_proto.initGoal());
    LOG_INFO("Go Right");
    tx_goal().publish();
    break;
  
  default:
    break;
  }

}

}  // namespace isaac

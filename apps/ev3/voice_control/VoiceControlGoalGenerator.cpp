#include "VoiceControlGoalGenerator.hpp"

#include "messages/math.hpp"

namespace isaac
{

void VoiceControlGoalGenerator::start()
{
  tickOnMessage(rx_detected_command());
}

void VoiceControlGoalGenerator::publish_goal(Pose2d pose)
{
  auto goal_proto = tx_goal().initProto();
  goal_proto.setStopRobot(false);
  goal_proto.setTolerance(0.1);
  goal_proto.setGoalFrame("robot");
  ToProto(pose, goal_proto.initGoal());
  tx_goal().publish();
}

void VoiceControlGoalGenerator::tick()
{

  if (rx_detected_command().available())
  {
    auto proto = rx_detected_command().getProto();
    int id = proto.getCommandId();
    switch (id)
    {
    case 1:
      LOG_INFO("Go Left");
      publish_goal(Pose2d::Rotation(M_PI_2));
      break;
    case 2:
      LOG_INFO("Go Right");
      publish_goal(Pose2d::Rotation(-M_PI_2));
      break;
    default:
      break;
    }
  }

  // Process feedback
  rx_feedback().processLatestNewMessage(
      [this](auto feedback_proto, int64_t pubtime, int64_t acqtime) {
        const bool arrived = feedback_proto.getHasArrived();
        LOG_INFO("Arrived!");
        // Show arrival information on WebSight
        show("arrived", arrived ? 1.0 : 0.0);
      });
}

} // namespace isaac

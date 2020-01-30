/*
Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#include "VoiceControlGoalGenerator.hpp"

#include "messages/math.hpp"


namespace isaac {

void VoiceControlGoalGenerator::start() {
  tickOnMessage(rx_voice_command_id());
}

void VoiceControlGoalGenerator::tick() {
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
    tx_goal().publish();
    break;
  case 2:
    goal_proto.setStopRobot(true);
    goal_proto.setTolerance(0.1);
    goal_proto.setGoalFrame("robot");
    ToProto(Pose2d::Rotation(-90), goal_proto.initGoal());
    tx_goal().publish();
    break;
  
  default:
    break;
  }

}

}  // namespace isaac

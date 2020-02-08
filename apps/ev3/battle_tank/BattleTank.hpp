#pragma once

#include <string>
#include <utility>
#include <vector>

#include "libpixyusb2.h"

#include "engine/alice/alice.hpp"
#include "messages/messages.hpp"
#include "engine/alice/components/deprecated/GroupSelectorBehavior.hpp"
#include "engine/alice/components/deprecated/SelectorBehavior.hpp"
#include "engine/gems/state_machine/state_machine.hpp"
#include "messages/math.hpp"

namespace isaac
{

namespace navigation {
class GroupSelectorBehavior;
}  // navigation


namespace ev3
{

// A codelet that uses a Pixy2 Camera for detecting objects
class BattleTank : public alice::Codelet
{
public:
    void start() override;
    void tick() override;
    void stop() override;

    // The original goal
    ISAAC_PROTO_RX(Goal2Proto, original_goal);    
    // Feedback about our progress towards the goal
    ISAAC_PROTO_RX(Goal2FeedbackProto, feedback);

    // The desired goal for the tank
    ISAAC_PROTO_TX(Goal2Proto, goal);

    // Parameter to get navigation mode behavior
    ISAAC_PARAM(std::string, navigation_mode,
                "navigation_mode/isaac.navigation.GroupSelectorBehavior");

    ISAAC_PARAM(double, center_threshold, 0.1);
    ISAAC_PARAM(double, area_threshold, 0.055);

    ISAAC_POSE2(world,robot);

private:
    // pixy2 interface
    Pixy2 pixy;
    uint32_t index_frame = 0;    

    navigation::GroupSelectorBehavior* navigation_mode_;
    bool target_centered = false;
    bool shoot_target = false;
    bool success = false;
    bool translation = false;
    bool rotation = false;
    bool prev_translation = false;
    bool prev_rotation = false;

    using State = std::string;
    // Creates the state machine
    void createStateMachine();
    state_machine::StateMachine<State> machine_;
    
};
} // namespace ev3
} // namespace isaac

ISAAC_ALICE_REGISTER_CODELET(isaac::ev3::BattleTank);

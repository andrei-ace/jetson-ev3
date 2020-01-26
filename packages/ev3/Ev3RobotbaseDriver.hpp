#pragma once

#include "engine/alice/alice_codelet.hpp"
#include "messages/messages.hpp"

namespace isaac { namespace drivers { class Ev3; }};

namespace isaac {
    class Ev3RobotbaseDriver : public isaac::alice::Codelet {
        public:
        void start() override;
        void tick() override;
        void stop() override;

        ISAAC_PROTO_RX(StateProto, ev3_cmd);

        ISAAC_PROTO_TX(StateProto, ev3_state);
    };
}

ISAAC_ALICE_REGISTER_CODELET(isaac::Ev3RobotbaseDriver);
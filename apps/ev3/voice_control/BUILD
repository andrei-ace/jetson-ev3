load("@com_nvidia_isaac//engine/build:isaac.bzl", "isaac_app", "isaac_cc_module","isaac_subgraph")

isaac_subgraph(
    name = "voice_commad_detection",
    data = [
    ],
    subgraph = "voice_command_detection.subgraph.json",
    visibility = ["//visibility:public"],
)

isaac_cc_module(
    name = "voice_control_goal_generator",
    srcs = [
        "VoiceControlGoalGenerator.cpp",
    ],
    hdrs = [
        "VoiceControlGoalGenerator.hpp"
    ],
    visibility = ["//visibility:public"]
)

isaac_app(
    name = "voice_control",
    data = [
        "maps",
        "voice_command_detection.subgraph.json",
        "model/isaac_vcd_model.pb",
        "model/isaac_vcd_model.metadata.json",
        "model/isaac_vcd_config.pb",
        "//apps/ev3:2d_ev3_subgraph",
        "@com_nvidia_isaac//packages/navigation/apps:differential_base_commander_subgraph",
        "@com_nvidia_isaac//packages/navigation/apps:differential_base_navigation_subgraph",
    ],
    modules = [
        "//apps/ev3/voice_control:voice_control_goal_generator",
        "@com_nvidia_isaac//packages/audio",
        "@com_nvidia_isaac//packages/ml:tensorflow",
        "@com_nvidia_isaac//packages/navigation",
        "@com_nvidia_isaac//packages/planner"
    ]
)

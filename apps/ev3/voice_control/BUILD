load("@com_nvidia_isaac//engine/build:isaac.bzl", "isaac_app", "isaac_cc_module","isaac_subgraph")

isaac_subgraph(
    name = "voice_commad_detection",
    modules = [
        "@com_nvidia_isaac//packages/audio",
        "@com_nvidia_isaac//packages/ml:tensorflow",
    ],
    data = [
        ":model/isaac_vcd_model.pb",
        ":model/isaac_vcd_config.pb",
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
    visibility = ["//visibility:public"],
    deps = ["@com_nvidia_isaac//engine/core/image",],
)

isaac_app(
    name = "voice_control",
    data = [
        "//apps/ev3:2d_ev3_subgraph",
        ":voice_command_detection.subgraph.json",
        "@com_nvidia_isaac//packages/navigation/apps:differential_base_commander_subgraph",
        "@com_nvidia_isaac//packages/navigation/apps:differential_base_imu_odometry_subgraph",
        "@com_nvidia_isaac//packages/navigation/apps:local_map_subgraph",
        "@com_nvidia_isaac//packages/navigation/apps:go_to_subgraph",        
        "@com_nvidia_isaac//packages_jetpack43/audio:libaudio_module.so_release",
    ],
    modules = [
        "//apps/ev3/voice_control:voice_control_goal_generator",
        "@com_nvidia_isaac//packages/audio",
        "@com_nvidia_isaac//packages/ml:tensorflow",
        "@com_nvidia_isaac//packages/navigation",
        "@com_nvidia_isaac//packages/perception",
        "@com_nvidia_isaac//packages/planner",
    ]
)
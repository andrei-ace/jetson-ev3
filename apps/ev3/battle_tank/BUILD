load("@com_nvidia_isaac//engine/build:isaac.bzl", "isaac_app", "isaac_cc_module")
isaac_app(
    name = "battle_tank",
    data = [
        "//apps/assets/maps",
        "//apps/ev3:2d_ev3_subgraph",
        "@com_nvidia_isaac//packages/navigation/apps:differential_base_commander_subgraph",
        "@com_nvidia_isaac//packages/navigation/apps:differential_base_navigation_subgraph",
    ],
    modules = [
        "//apps/ev3/battle_tank:battle_tank_components",
        "@com_nvidia_isaac//packages/planner",
        "@com_nvidia_isaac//packages/navigation"
    ]
)


isaac_app(
    name = "pixy_vision",
    data = [
    ],
    modules = [
        "//apps/ev3/battle_tank:pixy_vision_components"     
    ]
)


isaac_cc_module(
    name = "pixy_vision_components",
    srcs = [
        "PixyVision.cpp",
    ],
    hdrs = [
        "PixyVision.hpp",
    ],
    includes = [
        ".",
        "src/host/libpixyusb2/include",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@libpixyusb2_git//:libpixyusb2"
    ]
)


isaac_cc_module(
    name = "battle_tank_components",
    srcs = [
        "BattleTank.cpp",
    ],
    hdrs = [
        "BattleTank.hpp",
    ],
    includes = [
        ".",
        "src/host/libpixyusb2/include",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@com_nvidia_isaac//engine/gems/state_machine",
        "@libpixyusb2_git//:libpixyusb2"
    ]
)

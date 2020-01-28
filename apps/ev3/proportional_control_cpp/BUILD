"""
Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
"""

load("//engine/build:isaac.bzl", "isaac_app", "isaac_cc_module")

isaac_cc_module(
    name = "proportional_control_cpp_codelet",
    srcs = ["ProportionalControlCpp.cpp"],
    hdrs = ["ProportionalControlCpp.hpp"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_nvidia_isaac//engine/gems/state:io",
        "@com_nvidia_isaac//messages/state:differential_base",
    ],
)

isaac_app(
    name = "proportional_control_cpp",
    data = [
        "@com_nvidia_isaac//packages/navigation/apps:differential_base_commander_subgraph",
    ],
    modules = [
        "//apps/ev3/proportional_control_cpp:proportional_control_cpp_codelet",
        "@com_nvidia_isaac//packages/navigation",
        "ev3",
        "@com_nvidia_isaac//packages/sensors:joystick",
    ],
)

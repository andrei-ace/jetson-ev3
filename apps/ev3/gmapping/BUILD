"""
Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
"""

load("@com_nvidia_isaac//engine/build:isaac.bzl", "isaac_app")

isaac_app(
    name = "gmapping",
    data = [
        "//apps/ev3:2d_ev3_subgraph",
        "@com_nvidia_isaac//packages/navigation/apps:differential_base_commander_subgraph",
        "@com_nvidia_isaac//packages/navigation/apps:differential_base_imu_odometry_subgraph",
        "@com_nvidia_isaac//packages/navigation/apps:local_map_subgraph",
        "@com_nvidia_isaac//packages/record_replay/apps:record_subgraph",
    ],
    modules = [
        "@com_nvidia_isaac//packages/lidar_slam:g_mapping",
        "@com_nvidia_isaac//packages/navigation",
    ],
)

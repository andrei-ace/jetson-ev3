"""
Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
"""

load("@com_nvidia_isaac//engine/build:isaac.bzl", "isaac_app", "isaac_cc_module")
load("@com_nvidia_isaac//engine/build:cc_capnp_library.bzl", "cc_capnp_library")

isaac_cc_module(
    name = "ping_components",
    srcs = [
        "Ping.cpp",
        "Pong.cpp",
        "PongClient.cpp",
        "ping.capnp.c++",
    ],
    hdrs = [
        "Ping.hpp",
        "Pong.hpp",
        "ping.capnp.h"
    ],
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = [
        "@capnproto_git//:capnproto_rpc"
    ]
)

isaac_app(
    name = "ping_pong",
    modules = [
        "//apps/ev3/ping_pong:ping_components",
    ],
)

cc_capnp_library(
    name = "ping_messages",
    protos = ["ping.capnp"],
)

cc_binary(
    name = "ev3_pong",
    srcs = [
        "PongEv3Server.cpp",
        "ping.capnp.c++"
    ],
    includes = ["."],
    deps = [
        ":ping_messages",
        "@ev3dev_lang_cpp_git//:ev3dev_lang_cpp", 
        "@capnproto_git//:capnproto_cpp"
        ]
)

cc_binary(
    name = "test_ping",
    srcs = [
        "PongClient.cpp",
        "ping.capnp.c++",
    ],
    includes = ["."],
    deps = [
        ":ping_messages",
        "@capnproto_git//:capnproto_cpp"
        ]
)
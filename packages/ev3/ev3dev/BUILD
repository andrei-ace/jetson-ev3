load("@com_nvidia_isaac//engine/build:cc_capnp_library.bzl", "cc_capnp_library")

cc_capnp_library(
    name = "ev3control_messages",
    protos = ["ev3control.capnp"],
)

cc_library(
    name = "ev3control_messages_generated",
    visibility = ["//visibility:public"],
    deps = [
        ":ev3control_messages"
    ]
)


cc_binary(
    name = "ev3_control_server",
    srcs = [
        "Ev3ControlServer.cpp",
    ],
    deps = [
        ":ev3control_messages_generated",
        "@ev3dev_lang_cpp_git//:ev3dev_lang_cpp", 
        "@capnproto_git//:capnproto_cpp",
        "@com_nvidia_isaac//messages/state:differential_base",
        ],
)

cc_binary(
    name = "ev3_mock_server",
    srcs = [
        "Ev3MockServer.cpp",
    ],
    deps = [
        ":ev3control_messages_generated",
        "@ev3dev_lang_cpp_git//:ev3dev_lang_cpp", 
        "@capnproto_git//:capnproto_cpp",
        "@com_nvidia_isaac//messages/state:differential_base",
        ],
)
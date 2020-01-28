
load("//engine/build:isaac.bzl", "isaac_cc_module", "isaac_component")

isaac_cc_module(
    name = "ev3",
    deps = [
        ":ev3_driver"
    ]
)

isaac_component(
    name = "ev3_driver",
    deps = [
        "//packages/ev3/ev3dev:ev3control_messages",
        "@com_nvidia_isaac//engine/gems/state:io",
        "@com_nvidia_isaac//messages/state:differential_base",
        "@capnproto_git//:capnproto_rpc",
    ],
)

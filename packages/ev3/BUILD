
load("@com_nvidia_isaac//engine/build:isaac.bzl", "isaac_cc_module", "isaac_component")

isaac_cc_module(
    name = "ev3",
    deps = [
        ":ev3_robotbase_driver",
    ],
)

isaac_component(
    name = "ev3_robotbase_driver",
    deps = [
        "@com_nvidia_isaac//engine/gems/state:io",
        "@com_nvidia_isaac//messages/state:differential_base"
    ],
)

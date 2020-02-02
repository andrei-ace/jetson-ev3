load("@com_nvidia_isaac//engine/build:isaac.bzl", "isaac_app")

isaac_app(
    name = "voice_test",
    data = [
        "model/isaac_vcd_model.metadata.json",
        "model/isaac_vcd_model.pb",
        "model/isaac_vcd_config.pb",
        ],
    modules = [
        "@com_nvidia_isaac//packages/audio",
        "@com_nvidia_isaac//packages/ml:tensorflow",
    ],
)

py_binary(
    name="training_denoise",
    srcs=[
        "training_denoise.py",
        "utils.py"
    ],
)

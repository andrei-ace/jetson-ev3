
package(default_visibility = ['//visibility:public'])

cc_toolchain_suite(
    name = "armv5tejl_toolchain_suite",
    toolchains = {
        "armv5tejl": ":armv5tejl_toolchain",
        "k8|compiler": ":cc-compiler-k8",
    },
)

filegroup(name = "empty")

cc_toolchain(
    name = "armv5tejl_toolchain",
    toolchain_identifier = "armv5tejl_toolchain",
    cpu='armv7l',
    all_files = ":empty",
    compiler_files = ":empty",
    dwp_files = ":empty",
    linker_files = ":empty",
    objcopy_files = ":empty",
    strip_files = ":empty",
    static_runtime_libs = [":empty"],
    dynamic_runtime_libs = [":empty"],
    supports_param_files = 0,
)

cc_toolchain(
    name = "cc-compiler-k8",
    all_files = ":empty",
    compiler_files = ":empty",
    cpu = "k8",
    dwp_files = ":empty",
    dynamic_runtime_libs = [":empty"],
    linker_files = ":empty",
    objcopy_files = ":empty",
    static_runtime_libs = [":empty"],
    strip_files = ":empty",
    supports_param_files = 1,
)
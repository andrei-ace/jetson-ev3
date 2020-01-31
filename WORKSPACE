workspace(name = "jetson_ev3")

local_repository(
    name = "com_nvidia_isaac",
    path = "/home/andrei/ml/isaac-sdk-20191213-65ec14db"
)

load("@com_nvidia_isaac//engine/build:isaac.bzl", "isaac_git_repository", "isaac_new_http_archive")
load("@com_nvidia_isaac//third_party:engine.bzl", "isaac_engine_workspace")
load("@com_nvidia_isaac//third_party:packages.bzl", "isaac_packages_workspace")
load("@com_nvidia_isaac//third_party:ros.bzl", "isaac_ros_workspace")
load("@com_nvidia_isaac//third_party:zed.bzl", "isaac_zed_workspace")

isaac_engine_workspace()

isaac_packages_workspace()

isaac_ros_workspace()

isaac_zed_workspace()


load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")
new_git_repository(
    name = "ev3dev_lang_cpp_git",
    remote = "https://github.com/ddemidov/ev3dev-lang-cpp.git",
    commit = "6e2ca7e68113f538119a953f6b3bd3f2c9e6e572",
    build_file_content = """
cc_library(
    name = "ev3dev_lang_cpp",
    srcs = ["ev3dev.cpp"],
    hdrs = ["ev3dev.h"],
    visibility = ["//visibility:public"],
    copts = [
        "-Wno-unused-result",
    ],
)""",
)

new_git_repository(
    name = "capnproto_git",
    tag = "v0.7.0",
    remote = "https://github.com/capnproto/capnproto.git",
    build_file_content = """
cc_library(
    name = "capnproto_cpp",
    visibility = ["//visibility:public"],
    srcs = 
        glob([
        "c++/src/capnp/rpc.c++",
        "c++/src/capnp/rpc.capnp.c++",
        "c++/src/capnp/ez-rpc.c++",
        "c++/src/capnp/rpc-twoparty.c++",
        "c++/src/capnp/stringify.c++",
        "c++/src/capnp/capability.c++",
        "c++/src/capnp/serialize-async.c++",
        "c++/src/capnp/any.c++",
        "c++/src/capnp/arena.c++",
        "c++/src/capnp/blob.c++",
        "c++/src/capnp/c++.capnp.c++",
        "c++/src/capnp/layout.c++",
        "c++/src/capnp/list.c++",
        "c++/src/capnp/message.c++",
        "c++/src/capnp/schema.capnp.c++",
        "c++/src/capnp/serialize.c++",
        "c++/src/capnp/serialize-packed.c++",
        "c++/src/capnp/schema.c++",
        "c++/src/capnp/compat/json.c++",
        "c++/src/capnp/compat/json.capnp.c++",
        "c++/src/capnp/dynamic.c++",
        "c++/src/capnp/compiler/md5.c++",
        "c++/src/capnp/compiler/error-reporter.c++",
        "c++/src/capnp/compiler/lexer.capnp.c++",
        "c++/src/capnp/compiler/lexer.c++",
        "c++/src/capnp/compiler/grammar.capnp.c++",
        "c++/src/capnp/compiler/parser.c++",
        "c++/src/capnp/compiler/node-translator.c++",
        "c++/src/capnp/compiler/compiler.c++",
        "c++/src/capnp/compiler/type-id.c++",
        "c++/src/capnp/schema-parser.c++",
        "c++/src/capnp/serialize-text.c++",
        "capnproto/c++/src/capnp/schema.capnp.c++",
        "c++/src/kj/async.c++",
        "c++/src/kj/async-unix.c++",
        "c++/src/kj/async-io.c++",
        "c++/src/kj/async-io-unix.c++",
        "c++/src/kj/timer.c++",
        "c++/src/kj/array.c++",
        "c++/src/kj/common.c++",
        "c++/src/kj/debug.c++",
        "c++/src/kj/encoding.c++",
        "c++/src/kj/exception.c++",
        "c++/src/kj/hash.c++",
        "c++/src/kj/io.c++",
        "c++/src/kj/main.c++",
        "c++/src/kj/memory.c++",
        "c++/src/kj/mutex.c++",
        "c++/src/kj/string.c++",
        "c++/src/kj/test-helpers.c++",
        "c++/src/kj/thread.c++",
        "c++/src/kj/time.c++",
        "c++/src/kj/units.c++",
        "c++/src/kj/table.c++",
        "c++/src/kj/string-tree.c++",    
        "c++/src/kj/filesystem.c++",
        "c++/src/kj/filesystem-disk-unix.c++",
        "c++/src/kj/parse/char.c++",
        "c++/src/kj/refcount.c++",
        "c++/src/kj/string-tree.c++",        
        ]),
    hdrs = 
        glob([
            "c++/src/capnp/*.h",
            "c++/src/capnp/compiler/*.h",
            "c++/src/kj/*.h",
            "c++/src/kj/parse/*.h",
            "c++/src/capnp/compat/json.capnp.h",
            "c++/src/capnp/compat/json.h",
        ]),
    includes = [
        ".",
        "c++/src",
    ],
    copts = [
        "-Wno-maybe-uninitialized",
        "-Wno-unused-result",
        "-Wno-sign-compare",
        "-Wno-strict-aliasing",  
    ],
    linkopts=[
        "-lpthread",
    ],
)

cc_library(
    name = "capnproto_rpc",
    visibility = ["//visibility:public"],
    srcs = [
        "c++/src/capnp/rpc.c++",
        "c++/src/capnp/rpc.capnp.c++",
        "c++/src/capnp/ez-rpc.c++",
        "c++/src/capnp/rpc-twoparty.c++",
        "c++/src/capnp/stringify.c++",
        "c++/src/capnp/capability.c++",
        "c++/src/capnp/serialize-async.c++",
        "c++/src/kj/async.c++",
        "c++/src/kj/async-unix.c++",
        "c++/src/kj/async-io.c++",
        "c++/src/kj/async-io-unix.c++",
        "c++/src/kj/timer.c++",
        "c++/src/kj/refcount.c++",
    ],
    hdrs = 
        glob([
            "c++/src/capnp/*.h",
            "c++/src/capnp/compiler/*.h",
            "c++/src/kj/*.h",
            "c++/src/kj/parse/*.h",
            "c++/src/capnp/compat/json.capnp.h",
            "c++/src/capnp/compat/json.h",
        ]),
    includes = [
        ".",
        "c++/src",
    ],
    copts = [
        "-Wno-maybe-uninitialized",
        "-Wno-unused-result",
        "-Wno-sign-compare",
        "-Wno-strict-aliasing",  
    ],
)
""",
)

####################################################################################################
# Load cartographer

isaac_git_repository(
    name = "com_github_googlecartographer_cartographer",
    commit = "b6b41e9b173ea2e49e606f1e0d54d6d57ed421e3",
    licenses = ["@com_github_googlecartographer_cartographer//:LICENSE"],
    remote = "https://github.com/googlecartographer/cartographer.git",
)

isaac_git_repository(
    name = "com_google_protobuf",
    commit = "48cb18e5c419ddd23d9badcfe4e9df7bde1979b2",
    licenses = ["@com_google_protobuf//:LICENSE"],
    remote = "https://github.com/protocolbuffers/protobuf.git",
)

isaac_new_http_archive(
    name = "org_lzma_lzma",
    build_file = "@com_nvidia_isaac//third_party:lzma.BUILD",
    licenses = ["@org_lzma_lzma//:COPYING"],
    sha256 = "9717ae363760dedf573dad241420c5fea86256b65bc21d2cf71b2b12f0544f4b",
    strip_prefix = "xz-5.2.4",
    type = "tar.xz",
    url = "https://developer.nvidia.com/isaac/download/third_party/xz-5-2-4-tar-xz",
)

load("@com_github_googlecartographer_cartographer//:bazel/repositories.bzl", "cartographer_repositories")

cartographer_repositories()

# Loads boost c++ library (https://www.boost.org/) and
# custom bazel build support (https://github.com/nelhage/rules_boost/)
# explicitly for cartographer
# due to bazel bug: https://github.com/bazelbuild/bazel/issues/1550
isaac_git_repository(
    name = "com_github_nelhage_rules_boost",
    commit = "82ae1790cef07f3fd618592ad227fe2d66fe0b31",
    licenses = ["@com_github_nelhage_rules_boost//:LICENSE"],
    remote = "https://github.com/nelhage/rules_boost.git",
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")

boost_deps()

# Loads Google grpc C++ library (https://grpc.io/) explicitly for cartographer
load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

# Loads Prometheus Data Model Client Library (https://github.com/jupp0r/prometheus-cpp/) and
# CivetWeb (https://github.com/civetweb/civetweb/) explicitly for cartographer
load("@com_github_jupp0r_prometheus_cpp//:repositories.bzl", "load_civetweb", "load_prometheus_client_model")

load_civetweb()

load_prometheus_client_model()

####################################################################################################

# Configures toolchain
load("@com_nvidia_isaac//engine/build/toolchain:toolchain.bzl", "toolchain_configure")

toolchain_configure(name = "toolchain")

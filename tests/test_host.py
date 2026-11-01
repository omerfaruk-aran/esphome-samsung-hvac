"""Compile and run the real transport core with simulated UART IO."""

import os
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[1]


@pytest.mark.parametrize(
    ("program", "sources", "fixtures", "success"),
    [
        (
            "test_transport.cpp",
            [
                "frame_assembler.cpp",
                "nasa_framer.cpp",
                "non_nasa_framer.cpp",
                "bus_transport.cpp",
            ],
            "framing",
            "All transport assertions passed",
        ),
        (
            "test_nasa.cpp",
            [
                "frame_assembler.cpp",
                "nasa_framer.cpp",
                "nasa_packet.cpp",
                "nasa_read_queue.cpp",
                "nasa_engine.cpp",
            ],
            "nasa",
            "All NASA assertions passed",
        ),
        (
            "test_non_nasa.cpp",
            [
                "non_nasa_packet.cpp",
                "non_nasa_registration.cpp",
                "non_nasa_control_queue.cpp",
                "non_nasa_engine.cpp",
            ],
            "non_nasa",
            "All Non-NASA assertions passed",
        ),
        (
            "test_ac_profiles.cpp",
            [
                "ac_profile.cpp",
                "nasa_packet.cpp",
                "nasa_framer.cpp",
                "frame_assembler.cpp",
                "non_nasa_packet.cpp",
            ],
            "ac_profiles",
            "All AC profile assertions passed",
        ),
        (
            "test_ehs_profile.cpp",
            [
                "ehs_profile.cpp",
                "bus_transport.cpp",
                "nasa_engine.cpp",
                "nasa_read_queue.cpp",
                "nasa_packet.cpp",
                "nasa_framer.cpp",
                "frame_assembler.cpp",
            ],
            "ehs",
            "All EHS profile assertions passed",
        ),
        (
            "test_messages.cpp",
            [
                "typed_message.cpp",
                "write_policy.cpp",
                "nasa_packet.cpp",
                "nasa_framer.cpp",
                "frame_assembler.cpp",
            ],
            "advanced",
            "All typed message and policy assertions passed",
        ),
    ],
)
def test_host_with_sanitizers(tmp_path, program, sources, fixtures, success):
    compiler = os.environ.get("CXX") or shutil.which("clang++") or shutil.which("g++")
    assert compiler, "A C++17 compiler is required for transport tests"
    component = ROOT / "components/samsung_hvac"
    binary = tmp_path / "transport-tests"
    platform_defines = ["-DLOW=0", "-DHIGH=1"] if program == "test_non_nasa.cpp" else []
    if program == "test_ac_profiles.cpp":
        platform_defines = ["-DDISABLED=0", "-Dbit(b)=(1UL << (b))"]
    command = [
        compiler,
        "-std=c++17",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-pedantic",
        "-fsanitize=address,undefined",
        "-fno-omit-frame-pointer",
        "-g",
        *platform_defines,
        "-I",
        str(component),
        str(ROOT / "tests/cpp" / program),
        *[str(component / source) for source in sources],
        "-o",
        str(binary),
    ]
    compiled = subprocess.run(command, capture_output=True, text=True, timeout=120)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run(
        [str(binary), str(ROOT / "tests/fixtures" / fixtures)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    assert success in result.stdout

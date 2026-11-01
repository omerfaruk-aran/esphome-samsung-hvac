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
    ],
)
def test_host_with_sanitizers(tmp_path, program, sources, fixtures, success):
    compiler = os.environ.get("CXX") or shutil.which("clang++") or shutil.which("g++")
    assert compiler, "A C++17 compiler is required for transport tests"
    component = ROOT / "components/samsung_hvac"
    binary = tmp_path / "transport-tests"
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

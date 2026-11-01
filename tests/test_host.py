"""Compile and run the real transport core with simulated UART IO."""

import os
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def test_transport_with_sanitizers(tmp_path):
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
        str(ROOT / "tests/cpp/test_transport.cpp"),
        *[
            str(component / source)
            for source in (
                "frame_assembler.cpp",
                "nasa_framer.cpp",
                "non_nasa_framer.cpp",
                "bus_transport.cpp",
            )
        ],
        "-o",
        str(binary),
    ]
    compiled = subprocess.run(command, capture_output=True, text=True, timeout=120)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run(
        [str(binary), str(ROOT / "tests/fixtures/framing")],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    assert "All transport assertions passed" in result.stdout

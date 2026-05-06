#!/usr/bin/env python3
"""
Generate stub WAV files for JUCE AudioPluginHost BinaryData build.

This script creates minimal silent WAV files to satisfy the AudioPluginHost
CMakeLists.txt BinaryData requirements. These stub files enable building
AudioPluginHost from the vendored JUCE source without requiring copyrighted
JUCE example assets.

Context:
- AudioPluginHost includes demo plugins that reference example WAV files
- These files are missing from the vendored JUCE 8.0.10 source tree
- The build fails during BinaryData generation if they are absent

Limitations:
- Stub files are silent/minimal - demo plugins will not produce useful audio
- This is acceptable for Task 2.7 validation (ACE-Step VST3 load/pass-through)
- For full AudioPluginHost demo plugin validation, restore real JUCE assets

Usage:
    python ACE-Step-Plugin\\scripts\\generate-stub-juce-assets.py

Output:
    ACE-Step-Plugin\\External\\JUCE\\examples\\Assets\\cassette_recorder.wav
    ACE-Step-Plugin\\External\\JUCE\\examples\\Assets\\cello.wav
    ACE-Step-Plugin\\External\\JUCE\\examples\\Assets\\guitar_amp.wav
    ACE-Step-Plugin\\External\\JUCE\\examples\\Assets\\reverb_ir.wav
"""

import wave
import struct
from pathlib import Path

# Asset definitions
ASSETS = [
    {
        "filename": "cassette_recorder.wav",
        "duration_sec": 1.0,
        "description": "Stub audio sample (replaces JUCE example)",
    },
    {
        "filename": "cello.wav",
        "duration_sec": 1.0,
        "description": "Stub sampler sample (replaces JUCE example)",
    },
    {
        "filename": "guitar_amp.wav",
        "duration_sec": 0.5,
        "description": "Stub convolution IR (replaces JUCE example)",
    },
    {
        "filename": "reverb_ir.wav",
        "duration_sec": 0.5,
        "description": "Stub convolution IR (replaces JUCE example)",
    },
]

SAMPLE_RATE = 48000
CHANNELS = 2  # stereo
SAMPLE_WIDTH = 2  # 16-bit


def create_stub_wav(output_path: Path, duration_sec: float):
    """Create a silent stereo WAV file."""
    num_frames = int(duration_sec * SAMPLE_RATE)

    with wave.open(str(output_path), "w") as wav:
        wav.setnchannels(CHANNELS)
        wav.setsampwidth(SAMPLE_WIDTH)
        wav.setframerate(SAMPLE_RATE)
        # Write silent frames (all zeros)
        wav.writeframes(b"\x00\x00" * num_frames * CHANNELS)


def main():
    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent.parent
    assets_dir = repo_root / "ACE-Step-Plugin" / "External" / "JUCE" / "examples" / "Assets"

    if not assets_dir.exists():
        print(f"Error: Assets directory not found: {assets_dir}")
        return 1

    print(f"Generating stub JUCE assets in: {assets_dir}")
    print()

    for asset in ASSETS:
        output_path = assets_dir / asset["filename"]

        if output_path.exists():
            print(f"⚠️  SKIP: {asset['filename']} (already exists)")
            continue

        create_stub_wav(output_path, asset["duration_sec"])
        file_size_kb = output_path.stat().st_size / 1024
        print(
            f"✅ CREATED: {asset['filename']} "
            f"({asset['duration_sec']}s, {file_size_kb:.1f} KB) - {asset['description']}"
        )

    print()
    print("✅ Stub asset generation complete.")
    print()
    print("Note: These are minimal silent WAV files for build-only.")
    print("AudioPluginHost demo plugins will not produce useful audio output.")
    print("For full demo plugin validation, restore real JUCE 8.0.10 assets.")
    print()
    print("Next steps:")
    print("1. cd ACE-Step-Plugin\\External\\JUCE")
    print("2. cmake -B build-audio-plugin-host -G \"Visual Studio 17 2022\" -A x64 ^")
    print("       -DJUCE_BUILD_EXTRAS=ON -DJUCE_BUILD_EXAMPLES=OFF")
    print("3. cmake --build build-audio-plugin-host --config RelWithDebInfo ^")
    print("       --target AudioPluginHost")

    return 0


if __name__ == "__main__":
    exit(main())

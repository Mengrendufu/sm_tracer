#!/usr/bin/env python3
import sys
import os
import shutil
import subprocess
import argparse
from pathlib import Path

# ANSI color codes
COLOR_RESET = '\033[0m'
COLOR_GREEN = '\033[32m'
COLOR_CYAN = '\033[36m'
COLOR_RED = '\033[31m'

def color(text, color_code):
    if sys.stdout.isatty():
        return f"{color_code}{text}{COLOR_RESET}"
    return text

def header(msg):
    print(color(f">>> {msg}", COLOR_CYAN))

def success(msg):
    print(color(f">>> {msg}", COLOR_GREEN))

def error(msg):
    print(color(f">>> {msg}", COLOR_RED))

def get_script_dir():
    return Path(__file__).parent.resolve()

def get_build_dir():
    return get_script_dir() / "build"

def get_binary_path():
    bin_dir = get_build_dir() / "bin"
    if sys.platform.startswith("win"):
        return bin_dir / "sm_tracer.exe"
    else:
        return bin_dir / "sm_tracer"

def configure():
    header("Configuring project (CMake Ninja)...")
    build_dir = get_build_dir()
    try:
        subprocess.run([
            "cmake", "-S", str(get_script_dir()), "-B", str(build_dir), "-G", "Ninja"
        ], check=True)
        ccjson = build_dir / "compile_commands.json"
        success(f"CMake configure complete. Compile commands: {ccjson}")
    except subprocess.CalledProcessError as e:
        error("CMake configure failed.")
        sys.exit(1)

def build():
    build_dir = get_build_dir()
    ninja_file = build_dir / "build.ninja"
    if not ninja_file.exists():
        configure()
    header("Building project...")
    try:
        subprocess.run([
            "cmake", "--build", str(build_dir), "--parallel"
        ], check=True)
        bin_path = get_binary_path()
        success(f"Build complete. Binary: {bin_path}")
    except subprocess.CalledProcessError as e:
        error("Build failed.")
        sys.exit(1)

def clean():
    build_dir = get_build_dir()
    header("Cleaning build directory...")
    if build_dir.exists():
        try:
            shutil.rmtree(build_dir)
            success("Build directory removed.")
        except Exception as e:
            error(f"Failed to remove build directory: {e}")
            sys.exit(1)
    else:
        header("No build directory to clean.")

def run():
    build()
    bin_path = get_binary_path()
    header(f"Running binary: {bin_path}")
    if not bin_path.exists():
        error(f"Binary not found: {bin_path}")
        sys.exit(1)
    try:
        subprocess.run([str(bin_path)], check=True)
    except subprocess.CalledProcessError as e:
        error("Binary execution failed.")
        sys.exit(1)

def setup():
    header("Initializing git submodules...")
    try:
        subprocess.run([
            "git", "submodule", "update", "--init", "--recursive"
        ], check=True, cwd=str(get_script_dir()))
        success("Submodules initialized.")
    except subprocess.CalledProcessError as e:
        error("git submodule update failed.")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="sm_tracer build tool")
    subparsers = parser.add_subparsers(dest="command", help="Subcommands")

    parser_configure = subparsers.add_parser("configure", help="Configure project with CMake Ninja")
    parser_build = subparsers.add_parser("build", help="Build project (default)")
    parser_clean = subparsers.add_parser("clean", help="Remove build directory")
    parser_run = subparsers.add_parser("run", help="Build and run binary")
    parser_setup = subparsers.add_parser("setup", help="Initialize git submodules (run once after clone)")

    args = parser.parse_args()
    cmd = args.command or "build"

    if cmd == "configure":
        configure()
    elif cmd == "build":
        build()
    elif cmd == "clean":
        clean()
    elif cmd == "run":
        run()
    elif cmd == "setup":
        setup()
    else:
        parser.print_help()
        sys.exit(1)

if __name__ == "__main__":
    main()

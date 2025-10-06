#!/usr/bin/env python3
"""
Build script for llama-app-generator

Copyright 2025 Pooria Yousefi
Licensed under the Apache License, Version 2.0
"""

import os
import sys
import subprocess
import platform
from pathlib import Path


def main():
    """Build the llama-app-generator project"""
    
    # Project paths
    project_root = Path(__file__).parent.resolve()
    src_dir = project_root / "src"
    include_dir = project_root / "include"
    bin_dir = project_root / "bin"
    
    # Create bin directory if it doesn't exist
    bin_dir.mkdir(exist_ok=True)
    
    # Source files
    generator_cpp = src_dir / "generator.cpp"
    
    # Output executable
    if platform.system() == "Windows":
        output_exe = bin_dir / "llama-app-generator.exe"
    else:
        output_exe = bin_dir / "llama-app-generator"
    
    # Compiler selection
    if platform.system() == "Windows":
        compiler = os.environ.get("CXX", "g++")
    else:
        compiler = os.environ.get("CXX", "g++")
    
    # Compile command
    compile_cmd = [
        compiler,
        "-std=c++17",
        "-Wall",
        "-Wextra",
        "-O2",
        f"-I{include_dir}",
        str(generator_cpp),
        "-o", str(output_exe)
    ]
    
    # Add threading support on Unix-like systems
    if platform.system() != "Windows":
        compile_cmd.append("-pthread")
    
    print(f"Building llama-app-generator...")
    print(f"Compiler: {compiler}")
    print(f"Compiling... (fast build, no large headers)")
    print(f"Progress: ", end="", flush=True)
    
    try:
        result = subprocess.run(compile_cmd, check=True, capture_output=True, text=True)
        
        print(f"\r✓ Compilation complete!{' ' * 30}")
        print()
        
        if result.stdout:
            print(result.stdout)
        
        print(f"✓ Build successful!")
        print(f"✓ Executable: {output_exe}")
        print()
        print(f"Run with: {output_exe}")
        
        return 0
        
    except subprocess.CalledProcessError as e:
        print(f"✗ Build failed!")
        print(f"Error code: {e.returncode}")
        
        if e.stdout:
            print("\nStandard output:")
            print(e.stdout)
        
        if e.stderr:
            print("\nError output:")
            print(e.stderr)
        
        return 1
    
    except FileNotFoundError:
        print(f"✗ Compiler '{compiler}' not found!")
        print(f"Please install a C++ compiler or set the CXX environment variable.")
        return 1


if __name__ == "__main__":
    sys.exit(main())

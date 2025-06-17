# CFETCH - Configurable System Information Tool

A fast and customizable system information display tool written in C, inspired by neofetch but with more flexibility and better performance.

## Features

🎨 **Customizable ASCII Art** - Automatic distro detection or custom logos  
⚙️ **Configurable Display** - Show/hide any information field  
🖥️ **GPU Detection** - Advanced NVIDIA/AMD/Intel GPU support with driver info  
🌈 **Color Schemes** - Multiple built-in color themes  
💻 **Wayland & X11** - Full support for both display servers  
📦 **Package Managers** - Supports dpkg, pacman, rpm  
🚀 **Lightweight** - Fast startup, minimal dependencies  

## Quick Start

```bash
# Make installation script executable
chmod +x install.sh

# Install CFETCH
./install.sh

# Run it
cfetch

~/.config/cfetch/config > config file

How 2 edit? : # System Information
show_os=1          # Operating system
show_kernel=1      # Kernel version  
show_uptime=1      # System uptime
show_packages=1    # Package count

# User Environment
show_shell=1       # Current shell
show_terminal=1    # Terminal emulator
show_wm=1          # Window manager/DE

# Hardware
show_cpu=1         # CPU information
show_gpu=1         # GPU information
show_memory=1      # RAM usage
show_disk=1        # Disk usage
show_resolution=1  # Screen resolution

# Visual
show_colors=1      # Color palette


Themes

color_scheme=blue     # Default
color_scheme=red      # Red theme
color_scheme=green    # Green theme  
color_scheme=yellow   # Yellow theme
color_scheme=purple   # Purple theme
color_scheme=cyan     # Cyan theme
color_scheme=white    # White theme


ascii_file=auto


create your ascii file
~/.config/cfetch/ascii/mylogo

update config : ascii_file=mylogo

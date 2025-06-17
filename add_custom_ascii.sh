#!/bin/bash

echo "🎨 Custom ASCII Art Manager for CFETCH"
echo "======================================"

if [ $# -eq 0 ]; then
    echo "Usage: $0 <distro_name>"
    echo "Example: $0 mydistro"
    echo ""
    echo "This will create ~/.config/cfetch/ascii/mydistro"
    echo "Edit that file to add your custom ASCII art!"
    exit 1
fi

DISTRO_NAME="$1"
ASCII_DIR="$HOME/.config/cfetch/ascii"
ASCII_FILE="$ASCII_DIR/$DISTRO_NAME"

mkdir -p "$ASCII_DIR"

if [ -f "$ASCII_FILE" ]; then
    echo "⚠️  ASCII file already exists: $ASCII_FILE"
    echo "🔧 Opening in editor..."
else
    echo "📝 Creating new ASCII file: $ASCII_FILE"
    cat > "$ASCII_FILE" << 'EOF'
                 ###
               #######
             ###########
            #############
           ###############
          #################
         ###################
        #####################
        #####################
        #####################
         ###################
          #################
           ###############
            #############
             ###########
               #######
                 ###
EOF
    echo "✅ Created template ASCII art!"
    echo "🔧 Opening in editor to customize..."
fi

if command -v nano &> /dev/null; then
    nano "$ASCII_FILE"
elif command -v vim &> /dev/null; then
    vim "$ASCII_FILE"
elif command -v vi &> /dev/null; then
    vi "$ASCII_FILE"
else
    echo "📁 Edit this file manually: $ASCII_FILE"
fi

echo ""
echo "✨ To use your custom ASCII:"
echo "   1. Edit ~/.config/
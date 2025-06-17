#!/bin/bash

echo "🚀 Installing CFETCH - Fixed Version"
echo "===================================="

make clean
make

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    make install
    
    if [ $? -eq 0 ]; then
        echo "✅ Installation complete!"
        echo ""
        echo "🎉 CFETCH is ready!"
        echo "📁 Config: ~/.config/cfetch/config"
        echo "🎨 ASCII: ~/.config/cfetch/ascii/"
        echo ""
        echo "🚀 Run: cfetch"
    else
        echo "⚠️  Install failed, run: ./cfetch"
    fi
else
    echo "❌ Build failed!"
    exit 1
fi
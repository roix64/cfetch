#!/bin/bash

echo "🧪 Testing CFETCH..."
echo "==================="

echo "📋 System compatibility check..."

echo -n "🔍 lspci... "
if command -v lspci &> /dev/null; then
    echo "✅"
else
    echo "  Missing (GPU detection limited)"
fi

echo -n "🔍 xrandr... "
if command -v xrandr &> /dev/null; then
    echo "✅"
else
    echo "  Missing (No resolution detection)"
fi

echo -n "🔍 ip command... "
if command -v ip &> /dev/null; then
    echo "✅"
else
    echo "  Missing (No IP detection)"
fi

echo ""
echo "🏗️  Building cfetch..."
make clean && make

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    echo " Running cfetch..."
    echo "===================="
    ./cfetch
    echo ""
    echo " Test completed successfully!"
else
    echo "❌ Build failed!"
    exit 1
fi
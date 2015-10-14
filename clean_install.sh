#!/bin/bash
echo "Ehita ja paigalda!"
rm -rf build/
xcodebuild -project esteid-plugin.xcodeproj -target esteidfirefoxplugin
xcodebuild -project esteid-plugin.xcodeproj -target esteidfirefoxplugin install DSTROOT=/

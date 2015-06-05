# Browser token signing

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

## Building
[![Build Status](https://travis-ci.org/open-eid/browser-token-signing.svg?branch=master)](https://travis-ci.org/open-eid/browser-token-signing)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/3355/badge.svg)](https://scan.coverity.com/projects/3355)

### Ubuntu

1. Install dependencies

        sudo apt-get install libgtk2.0-dev libssl-dev

2. Fetch the source

        git clone --recursive https://github.com/open-eid/browser-token-signing
        cd browser-token-signing

3. Build

        make plugin

4. Install

        make install

5. Execute

        firefox
        
### OSX

1. Fetch the source

        git clone --recursive https://github.com/open-eid/browser-token-signing
        cd browser-token-signing

2. Build

        xcodebuild -project esteid-plugin.xcodeproj -target esteidfirefoxplugin

3. Install

        xcodebuild -project esteid-plugin.xcodeproj -target esteidfirefoxplugin install DSTROOT=/

4. Execute

        open /Application/Safari.app

## Support
Official builds are provided through official distribution point [installer.id.ee](https://installer.id.ee). If you want support, you need to be using official builds. Contact for assistance by email abi@id.ee or http://www.id.ee.

Source code is provided on "as is" terms with no warranty (see license for more information). Do not file Github issues with generic support requests.

# Browser token signing

 * License: LGPL 2.1
 * &copy; Estonian Information System Authority

Warning: Firefox 52 removed NPAPI support and added Chrome Extensions support since version 50 which is implemented in [chrome-token-signing](/open-eid/chrome-token-signing)

## Documentation

 - https://github.com/open-eid/browser-token-signing/wiki

## Building
[![Build Status](https://travis-ci.org/open-eid/browser-token-signing.svg?branch=master)](https://travis-ci.org/open-eid/browser-token-signing)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/3355/badge.svg)](https://scan.coverity.com/projects/3355)

### Ubuntu

1. Install dependencies

        sudo apt-get install libgtk2.0-dev libssl-dev libpcsclite-dev

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

- Fetch the source

        git clone --recursive https://github.com/open-eid/browser-token-signing
        cd browser-token-signing

- Build a installable package firefox-token-signing.pkg

        make

- Execute

        open /Application/Safari.app

### Windows

1. Fetch the source

        git clone --recursive https://github.com/open-eid/browser-token-signing
        cd browser-token-signing

2. Build

        msbuild /t:ie-token-signing:Rebuild;ie-token-signing-wix:Rebuild /p:Configuration=Release;Platform=Win32 browser-token-signing.sln
        msbuild /t:firefox-token-signing:Rebuild;firefox-token-signing-wix:Rebuild /p:Configuration=Release;Platform=Win32 browser-token-signing.sln

3. Install

        ie/Release/ie-token-signing_3.12.0.0_x86.msi
        ie/Release/firefox_token_signing_3.12.0.0_x86.msi

4. Execute

        Open Internet Explorer or Firefox

## Support
Official builds are provided through official distribution point [installer.id.ee](https://installer.id.ee). If you want support, you need to be using official builds. Contact for assistance by email abi@id.ee or http://www.id.ee.

Source code is provided on "as is" terms with no warranty (see license for more information). Do not file Github issues with generic support requests.

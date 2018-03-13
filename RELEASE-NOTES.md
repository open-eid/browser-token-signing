browser-token-signing version [3.13.1](https://github.com/open-eid/browser-token-signing/releases/tag/v3.13.1) release notes
--------------------------------------------
- Use OpenSC driver with unkown ATR on OSX/Linux (#45)
- Add new Lithuanian driver paths (#48)
- Use SCardGetStatusChange to get ATR-s and avoid to connecting with card (#51)
- Filter attributes with token filter (#53)
- Code and build improvements

[Full Changelog](https://github.com/open-eid/browser-token-signing/compare/v3.13.0...v3.13.1)


browser-token-signing version [3.13.0](https://github.com/open-eid/browser-token-signing/releases/tag/v3.13.0) release notes
--------------------------------------------
- Rewrite NPAPI plugin for Safari MacOS
- Rewrite BHO plugin
- Drop Windows/Linux NPAPI support
- Add ECDSA token support

[Full Changelog](https://github.com/open-eid/browser-token-signing/compare/v3.12.2...v3.13.0)


browser-token-signing version [3.12.2](https://github.com/open-eid/browser-token-signing/releases/tag/v3.12.2) release notes
--------------------------------------------
- Lithuania token iprovements
- Code and build improvements

[Full Changelog](https://github.com/open-eid/browser-token-signing/compare/v3.12.1...v3.12.2)


Esteid-plugin version 3.12.1 release notes
--------------------------------------
Changes compared to ver 3.11.0

- Improved Esteid-plugin for Internet Explorer
	- More generic pkcs11 card manager for IE, moved certificate match check from signer to card manager and support having multiple cards connected when signing with pkcs11.
	- Fixed transparent PIN dialog issue
	- Fixed wrong private key handle used for signing with LT card
	- Added Custom Dialog and using PKCS11 for LT cards to bypass LT middleware PIN cache issues. It uses the selected certificate to determine if it's a LT card
	- Use WIX visual studio integration and cleanup

- Improved Esteid-plugin for Mozilla Firefox
	- Fixed random Firefox crash in OSX
	- Fixed PIN attempts remaining counter not decreasing with Lithuanian and Finnish tokens
	- Added Token Support for Latvian and Finnish eID on OSX and Token Support for Lithuanian, Latvian and Finnish eID on Linux. Note: you need to have middleware for the eID that you are using installed (Finland, Estonia, Latvia or Lithuania). No other configuration should be necessary.

List on known issues: https://github.com/open-eid/browser-token-signing/wiki/Known-issues
List of supported tokens: hhttps://github.com/open-eid/browser-token-signing/wiki/Token-Support


Esteid-plugin version 3.11.0 release notes
--------------------------------------
Changes compared to ver 3.9.1

- Improved Esteid-plugin for Internet Explorer
	- Added support for international tokens
	- Removed openssl dependency
	- Added license and icon to the installer
	- Added CAPI support beyond SHA-1
	- Created installers for x86 and x64 platforms
	- Fixed CAPI signing error check
	- Fixed certificate selection cancelling
	- Fixed signing cancelling
	- Fixed access denied error that could occur on some versions of Windows after certificate selection
	- Added platform (32bit or 64bit) to IE plugin version number
	- Refactoring. Cleaned up a lot of code, deleted unused code

List on known issues: https://github.com/open-eid/browser-token-signing/wiki/Known-issues
List of supported tokens: hhttps://github.com/open-eid/browser-token-signing/wiki/Token-Support


Esteid-plugin version 3.9.1 release notes
--------------------------------------
Changes compared to ver 3.8

- Added 64-bit version of browser signing plug-in for 64-bit Chrome browser in Windows environment.



Esteid-plugin version 3.8 release notes
--------------------------------------

- Started using esteid-pkcs11 and esteid-tokend drivers in OSX (instead of opensc).

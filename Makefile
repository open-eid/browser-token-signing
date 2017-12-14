BUILD_NUMBER ?= 0
include VERSION.mk

SIGNER ?= Riigi Infosüsteemi Amet

PROJ = xcodebuild -project browser-token-signing.xcodeproj VERSION=$(VERSION) BUILD_NUMBER=$(BUILD_NUMBER)

TMPROOT = $(PWD)/tmp
TARGET = $(TMPROOT)/Library/Internet\ Plug-Ins/esteidfirefoxplugin.bundle
PKG = firefox-token-signing.pkg

build: default

default: pkg

$(TARGET):
	$(PROJ) DSTROOT=$(TMPROOT) install

clean:
	rm -rf $(TMPROOT)
	git clean -dfx

codesign: $(TARGET)
	codesign -f -s "Developer ID Application: $(SIGNER)" --entitlements=browser-token-signing.entitlements $(TARGET)

$(PKG): $(TARGET)
	pkgbuild --version $(VERSION) \
		--identifier ee.ria.firefox-token-signing \
		--root $(TMPROOT) \
		--install-location / \
		$(PKG)

pkg: $(PKG)

signed: codesign
	pkgbuild --version $(VERSION) \
		--identifier ee.ria.firefox-token-signing \
		--root $(TMPROOT) \
		--install-location / \
		--sign "Developer ID Installer: $(SIGNER)" \
		$(PKG)

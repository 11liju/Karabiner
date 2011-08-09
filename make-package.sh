#!/bin/sh

version=$(cat version)

packagemaker=/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker
pkgName="KeyRemap4MacBook.pkg"
archiveName="KeyRemap4MacBook-${version}.pkg.zip"

make clean build || exit $?

# --------------------------------------------------
# http://developer.apple.com/documentation/Darwin/Conceptual/KEXTConcept/KEXTConceptPackaging/packaging_kext.html
echo "Copy Files"

rm -rf pkgroot
mkdir -p pkgroot

basedir="/Library/org.pqrs/KeyRemap4MacBook"
mkdir -p "pkgroot/$basedir"
cp -R src/core/kext/build/Release/KeyRemap4MacBook.kext "pkgroot/$basedir"
cp -R files/scripts "pkgroot/$basedir"

mkdir -p "pkgroot/$basedir/prefpane"
cp files/prefpane/number.xml files/prefpane/output/checkbox.xml "pkgroot/$basedir/prefpane"

mkdir -p "pkgroot/$basedir/extra"
cp -R pkginfo/Resources/preflight "pkgroot/$basedir/extra/uninstall_core.sh"
cp -R files/extra/launchUninstaller.sh "pkgroot/$basedir/extra/"
cp -R files/extra/uninstall.sh "pkgroot/$basedir/extra/"

mkdir -p "pkgroot/Library"
cp -R files/LaunchDaemons pkgroot/Library
cp -R files/LaunchAgents pkgroot/Library

mkdir -p "pkgroot/$basedir/app"
cp -R "src/core/server/build/Release/KeyRemap4MacBook.app" "pkgroot/$basedir/app"
cp -R "src/util/KeyDump/build/Release/KeyDump.app" "pkgroot/$basedir/app"
cp -R "src/util/multitouchextension/build/Release/KeyRemap4MacBook_multitouchextension.app" "pkgroot/$basedir/app"
cp -R "src/util/cli/build/Release/KeyRemap4MacBook_cli.app" "pkgroot/$basedir/app"
cp -R "src/util/uninstaller/build/Release/uninstaller.app" "pkgroot/$basedir/app"

mkdir -p "pkgroot/Library/PreferencePanes"
cp -R "src/util/prefpane/build/Release/KeyRemap4MacBook.prefPane" "pkgroot/Library/PreferencePanes"

find pkgroot -type d -print0 | xargs -0 chmod 755
find pkgroot -type f -print0 | xargs -0 chmod 644
find pkgroot -name '*.sh' -print0 | xargs -0 chmod 755
for file in `find pkgroot -type f`; do
    if ./pkginfo/is-mach-o.sh "$file"; then
        chmod 755 "$file"
    fi
done

# --------------------------------------------------
echo "Exec PackageMaker"

rm -rf $pkgName

# Note: Don't add --no-recommend option.
# It breaks /Library permission.
# (It overwrites /Library permission with pkgroot/Library permission.)
# - Mac OS X 10.5: /Library is 1775
# - Mac OS X 10.6: /Library is 0755
$packagemaker \
    --root pkgroot \
    --info pkginfo/Info.plist \
    --resources pkginfo/Resources \
    --title "KeyRemap4MacBook $version" \
    --no-relocate \
    --out $pkgName

# --------------------------------------------------
echo "Make Archive"

zip -X -r $archiveName $pkgName
rm -rf $pkgName
chmod 644 $archiveName
unzip $archiveName

# --------------------------------------------------
echo "Cleanup"
rm -rf pkgroot
make -C src clean

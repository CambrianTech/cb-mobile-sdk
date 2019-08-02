

if [ -z "$1" ]; then
	CBROOT=${CB}
	BUILD_PATH="${CBROOT}/unity/UnityAsFramework/unity-ios-project/Build/Products/Release-iphoneos"
else
	CBROOT=$1
	BUILD_PATH=$2
fi

SRC_PROJECT_ROOT="${CBROOT}/unity/Build/ios"

INSTALL_DIR="${CBROOT}/prebuilts/unity/ios"
FRAMEWORK_PATH="${BUILD_PATH}/UnityAsFramework.framework"

mkdir -p "${INSTALL_DIR}"

rm -Rf "${INSTALL_DIR}/UnityAsFramework.framework"
mv "${FRAMEWORK_PATH}" "${INSTALL_DIR}"


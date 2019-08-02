

if [ -z "$1" ]; then
	CBROOT=${CB}
else
	CBROOT=$1
fi

SRC_PROJECT_ROOT="${CBROOT}/cb-unity/src/cb-unity/Build/ios"
INSTALL_DIR="${CBROOT}/prebuilts/unity"

#transfer Data directory
DATA_SRC="${SRC_PROJECT_ROOT}/Data"
DATA_DEST="${INSTALL_DIR}/Data"

rm -Rf "${DATA_DEST}"
cp -R "${DATA_SRC}" "${DATA_DEST}"


if [ -z "$1" ]; then
	CBROOT=${CB}
else
	CBROOT=$1
fi

SRC_PROJECT_ROOT="${CBROOT}/unity/Build/ios"
INSTALL_DIR="${CBROOT}/prebuilts/unity"

mkdir -p "${INSTALL_DIR}"

#transfer Data directory
DATA_SRC="${SRC_PROJECT_ROOT}/Data"
DATA_DEST="${INSTALL_DIR}/Data"

rm -Rf "${DATA_DEST}"
cp -R "${DATA_SRC}" "${DATA_DEST}"
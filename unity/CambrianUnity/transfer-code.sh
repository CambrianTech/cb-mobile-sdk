set -e 
set -o pipefail

if [ -z "$1" ]; then
	CBROOT=${CB}
else
	CBROOT=$1
fi

SRC_PROJECT_ROOT="${CBROOT}/unity/Build/ios"
DEST_PROJECT_ROOT="${CBROOT}/unity/CambrianUnity/Unity"

#TRANSFER NATIVES
NATIVE_SRC="${SRC_PROJECT_ROOT}/Classes"
NATIVE_DEST="${DEST_PROJECT_ROOT}/Classes"


rm -Rf ${NATIVE_DEST}
cp -R ${NATIVE_SRC} "${DEST_PROJECT_ROOT}"

touch ${NATIVE_DEST}/*.*
touch ${NATIVE_DEST}/**/*.*

#rm ${NATIVE_DEST}/Generated*


#TRANSFER libcpp
LIBS_SRC="${SRC_PROJECT_ROOT}/Libraries"
LIBS_DEST="${DEST_PROJECT_ROOT}/Libraries"


rm -Rf ${LIBS_DEST}/libil2cpp

cp -R ${LIBS_SRC}/libil2cpp ${LIBS_DEST}

#cp -Rf ${LIBS_SRC}/*.* ${LIBS_DEST}

rsync -av --progress ${LIBS_SRC} ${DEST_PROJECT_ROOT} --exclude CambrianARPlugin
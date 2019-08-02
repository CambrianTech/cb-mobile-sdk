

if [ -z "$1" ]; then
	CBROOT=${CB}
else
	CBROOT=$1
fi

SRC_PROJECT_ROOT="${CBROOT}/cb-unity/src/cb-unity/Build/ios"
DEST_PROJECT_ROOT="${CBROOT}/cb-unity/src/cb-unity/CambrianUnity/Unity"

#TRANSFER NATIVES
NATIVE_SRC="${SRC_PROJECT_ROOT}/Classes/Native"
NATIVE_DEST="${DEST_PROJECT_ROOT}/Classes/Native"

rm -Rf ${NATIVE_DEST}
cp -R ${NATIVE_SRC} "${DEST_PROJECT_ROOT}/Classes"
touch ${NATIVE_DEST}/*.cpp
#rm ${NATIVE_DEST}/Generated*

#TRANSFER libcpp
LIBS_SRC="${SRC_PROJECT_ROOT}/Libraries"
LIBS_DEST="${DEST_PROJECT_ROOT}/Libraries"

rm -Rf ${LIBS_DEST}/libil2cpp

cp -R ${LIBS_SRC}/libil2cpp ${LIBS_DEST}

cp -f ${LIBS_SRC}/*.* ${LIBS_DEST}/

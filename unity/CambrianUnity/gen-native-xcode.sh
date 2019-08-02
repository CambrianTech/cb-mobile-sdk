

if [ -z "$1" ]; then
	CBROOT=${CB}
else
	CBROOT=$1
fi

DEST_PROJECT_ROOT="${CBROOT}/unity/UnityAsFramework/unity-ios-project"

CLASSES_DEST="${DEST_PROJECT_ROOT}/Classes"

cd ${CLASSES_DEST}

#gen Native project
/usr/local/bin/cmake -G Xcode ./ -DCMAKE_INSTALL_PREFIX=native -DBUILD_SHARED_LIBS=OFF

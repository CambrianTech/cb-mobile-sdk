#!/bin/sh -e


echo "Target architectures: $VALID_ARCHS"
find "$APP_PATH" -name '*.framework' -type d | while read -r FRAMEWORK
do
	
	FILENAME=$(basename "$FRAMEWORK" .framework)
	FRAMEWORK_EXECUTABLE_NAME=$FILENAME
    FRAMEWORK_EXECUTABLE_PATH="$FRAMEWORK/$FRAMEWORK_EXECUTABLE_NAME"

    if ! [[ "$(file "$FRAMEWORK_EXECUTABLE_PATH")" == *"dynamically linked shared library"* ]]; then
    	continue
  	fi

  	echo "Stripping executable $FRAMEWORK_EXECUTABLE_PATH"

  	archs="$(lipo -info "${FRAMEWORK_EXECUTABLE_PATH}" | rev | cut -d ':' -f1 | rev)"
  	stripped=""
  	for arch in $archs; do
    if ! [[ "${VALID_ARCHS}" == *"$arch"* ]]; then
      # Strip non-valid architectures in-place
      lipo -remove "$arch" -output "$FRAMEWORK_EXECUTABLE_PATH" "$FRAMEWORK_EXECUTABLE_PATH" || exit 1
      stripped="$stripped $arch"
    fi
  	done
  	if [[ "$stripped" != "" ]]; then
    	echo "Stripped $FRAMEWORK_EXECUTABLE_PATH of architectures:$stripped"
    	if [ "${CODE_SIGNING_REQUIRED}" == "YES" ]; then
      		code_sign "${FRAMEWORK_EXECUTABLE_PATH}"
    	fi
  	fi

done
#!/bin/sh -e

# for more info, go here:
#https://github.com/Carthage/Carthage/blob/master/Documentation/StaticFrameworks.md

function carthage-sync() {

  	echo "Build options -clean (whatever version beta or normal last selected) or -beta to select beta symlink path"

	if [ $# -eq 1 ] && [ "$1" == "clean" ]; then
		echo "Cleaning Carthage/Build, Carthage/Checkouts"
        rm -Rf Carthage
		rm -Rf Carthage/Build/*
		rm -Rf Carthage/Checkouts
		exit
	fi

    mkdir -p Carthage

    pushd Carthage

	rm -f Build

	if [ $# -eq 1 ] && [ "$1" == "beta" ]; then
		echo "Building in beta mode (Xcode-beta.app)"
		sudo xcode-select -s /Applications/Xcode-beta.app/
		mkdir -p Build-beta
		ln -s Build-beta Build
	else 
		echo "Building in normal mode (Xcode.app)"
		mkdir -p Build-normal
		ln -s Build-normal Build
	fi

	popd

	carthage update --platform iOS 

	#static frameworks go here:
	echo "Building static frameworks"

	bash $cb_home/ios/carthage-build-static.sh

	sudo xcode-select -s /Applications/Xcode.app/
}



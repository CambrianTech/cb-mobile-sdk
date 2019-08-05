#!/usr/bin/env bash

unamestr=`uname`
if [[ "$unamestr" == 'Linux' ]]; then
    PLATFORM='linux-x86_64'
    PYTHON_EXE=python
    PIP_EXE=pip
elif [[ "$unamestr" == 'Darwin' ]]; then
    PLATFORM='darwin-x86_64'
    PYTHON_EXE=python3
    PIP_EXE=pip3
    #export PYTHONPATH=`brew --prefix`/lib/python3.6/site-packages:$PYTHONPATH
fi


#format commands e.g. echo -e "this is some ${sb}bold${eb} text."
sb="\033[1m"
eb="\033[0m"

Black="0;30"     
DarkGray="1;30"
Red="0;31"    
LightRed="1;31"
Green="0;32"   
LightGreen="1;32"
BrownOrange="0;33"   
Yellow="1;33"
Blue="0;34"  
LightBlue="1;34"
Purple="0;35"   
LightPurple="1;35"
Cyan="0;36"   
LightCyan="1;36"
LightGray="0;37"  
White="1;37"
Clear="0"

#red="\033[0;31m"
#nc='\033[0m' # No Color

function echo_color() {
    COLOR=$1
    TEXT=$2
    echo -e "\033[${COLOR}m${TEXT}\033[0m"
}

function error_message() {
    echo -e "${red}Error: $1${nc}"
}

function install_tool() {
    TOOL="$1"

    # apt get etc
    case "$OSTYPE" in
      linux*)   sudo apt-get install "${TOOL}" ;;
      darwin*)  
            if brew list ${TOOL} &>/dev/null; then
                if brew upgrade "${TOOL}" &>/dev/null; then 
                    >&2 echo "Upgraded ${TOOL}"
                else 
                    echo "Tool ${TOOL} is up to date"
                fi
            else
                if brew install ${TOOL}; then 
                    echo "Installed ${TOOL}"
                fi
            fi
            ;;
            
      *)        echo "error" ;;
    esac
}

function download_dependency() {

    install_tool mercurial
    #Usage download_dependency "name", where name matches one defined inside downloader/CMakeLists.txt

    if ping -c 1 8.8.8.8 >> /dev/null 2>&1; then
        cd $CB/scripts/cb/downloader
        rm -Rf temp
        mkdir temp
        cd temp
        cmake -Ddownload="$1" ../
        make
        cd ..
        
        rm -Rf temp
    else
        echo "Offline, ignoring dependency $1"
    fi
}



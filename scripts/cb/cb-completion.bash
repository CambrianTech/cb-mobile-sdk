#!/bin/bash

# Completions for ev shell function
function _cb()
{
    case "${COMP_WORDS[1]}" in
        build)
            _cb_build_completions
            return 0
            ;;
        download)
            _cb_download_completions
            return 0
            ;;
        carthage-sync)
            COMPREPLY=( $( compgen -W "beta clean" -- "${COMP_WORDS[COMP_CWORD]}" ) )
            return 0
            ;;
        install)
            _cb_install_completions
            return 0
            ;;
    esac

    if [ "${COMP_CWORD}" -eq "1" ]; then
        COMPREPLY=( $( compgen -W "build download install carthage-sync" -- "${COMP_WORDS[COMP_CWORD]}" ) )
    fi
}
complete -F _cb cb

function _cb_build_completions() {
if [ "${COMP_CWORD}" -eq "2" ]; then
    COMPREPLY=( $( compgen -W "core sdk cbclient opencv bgfx keras tensorflow assimp libyuv liblzma libjpeg bzip2 jsoncpp pcl tests" -- "${COMP_WORDS[COMP_CWORD]}" ) )
fi

if [ "${COMP_CWORD}" -eq "3" ]; then
    case "${COMP_WORDS[2]}" in
        sdk)
            COMPREPLY=( $( compgen -W "android ios" -- "${COMP_WORDS[COMP_CWORD]}" ) )
            return 0
            ;;
        cbclient)
            COMPREPLY=( $( compgen -W "osx linux" -- "${COMP_WORDS[COMP_CWORD]}" ) )
            return 0
            ;;
        keras)
            COMPREPLY=( $( compgen -W "linux osx" -- "${COMP_WORDS[COMP_CWORD]}" ) )
            return 0
            ;;
        assimp)
            COMPREPLY=( $( compgen -W "android ios" -- "${COMP_WORDS[COMP_CWORD]}" ) )
            return 0
            ;;
        *)
            COMPREPLY=( $( compgen -W "android ios linux osx" -- "${COMP_WORDS[COMP_CWORD]}" ) )
            return 0
            ;;
    esac
fi

case "${COMP_WORDS[2]}" in
    core)
        case "${COMP_WORDS[3]}" in
            android*)
                _cb_core_android_completions
                return 0
                ;;
        esac
        ;;
    sdk)
        case "${COMP_WORDS[3]}" in
            android|ios)
                if [ "${COMP_CWORD}" -eq "4" ]; then    
                    COMPREPLY=( $( compgen -W "minimal" -- "${COMP_WORDS[COMP_CWORD]}" ) )
                fi
                return 0
                ;;
            android-remote|ios-remote)
                if [ "${COMP_CWORD}" -eq "4" ]; then
                    COMPREPLY=( $( compgen -W "noserver release logging" -- "${COMP_WORDS[COMP_CWORD]}" ) )
                fi
                if [ "${COMP_CWORD}" -eq "5" ]; then  
                    case "${COMP_WORDS[4]}" in  
                        noserver)
                            COMPREPLY=( $( compgen -W "release logging" -- "${COMP_WORDS[COMP_CWORD]}" ) )
                            return 0
                            ;;
                        release|logging)
                            COMPREPLY=( $( compgen -W "noserver" -- "${COMP_WORDS[COMP_CWORD]}" ) )
                            return 0
                            ;;
                    esac
                fi
                return 0
                ;;
        esac
        ;;
esac
}

function _cb_install_completions() {
    
if [ "${COMP_CWORD}" -eq "2" ]; then
    COMPREPLY=( $( compgen -W "android keras tensorflow" -- "${COMP_WORDS[COMP_CWORD]}" ) )
fi

}

function _cb_download_completions() {
    
if [ "${COMP_CWORD}" -eq "2" ]; then
    COMPREPLY=( $( compgen -W " android cpp web ios ml demoweb bgfx opencv libyuv tensorflow keras eigen jsoncpp pcl flann boost liblzma libjpeg bzip2 unity" -- "${COMP_WORDS[COMP_CWORD]}" ) )
fi

}

#!/usr/bin/env bash

source cb_common.sh

function cb_logger() {
    local cb_log_dir=$CB/.logs
    local cb_log_file=$cb_log_dir/`date '+%Y-%m-%d'`.log
    
    [[ ! -d $cb_log_dir ]] && mkdir $cb_log_dir
    [[ ! -e $cb_log_file ]] && touch $cb_log_file

    echo "`date '+%Y-%m-%d %r'`    $@" | tee -a $cb_log_file
    return 0
}

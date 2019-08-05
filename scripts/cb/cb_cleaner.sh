#!/usr/bin/env bash

function cb_cleaner() {
  cb_logger "Cleaning $@"
  cd $CBCORE/cbar
  make clean

  return 0
}

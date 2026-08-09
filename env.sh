#!/bin/bash

export C_DEV_ENV=$(pwd)"/.env";

new_project()
{
  if [[ ! -b ${1} ]]; then
    cp -r $C_DEV_ENV/empty_project ${1}
    cd ${1}
    git init .
  else
    echo "project ${1} already exists"
  fi
}

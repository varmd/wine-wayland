#!/bin/bash

#TODO
#allow configuration
if [ "$HTTP_REFERER" != "http://localhost:18180/" ]; then
  exit;
fi


SH_LAUNCH_PATH=$PWD/../

if [ -z $WINELAND_FILE_DIR ]; then
  exit;
fi



cd $WINELAND_FILE_DIR

FILE_PATH=""
function escape_path {
  local a=$1
  a=${a//[^[:alnum:].-]/}
  a=${a//../}
  FILE_PATH=$a
}


stringContains() { [ -z "${2##*$1*}" ]; }

if [ -z $QUERY_STRING ]; then
  exit 0;
fi

escape_path $QUERY_STRING;

if [ ! -d $PWD/$FILE_PATH ]; then
  echo "folder does not exist"
  exit;
fi

printf "Content-type: text/plain\n\n"

exec sh $SH_LAUNCH_PATH/start.sh $FILE_PATH


exit 0
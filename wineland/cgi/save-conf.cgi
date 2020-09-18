#!/bin/bash

#TODO
#allow configuration
if [ "$HTTP_REFERER" != "http://localhost:18180/" ]; then
  exit;
fi


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

if [ -z $FILE_DIR ]; then
  FILE_DIR="./files";
fi

stringContains() { [ -z "${2##*$1*}" ]; }

if [ -z $QUERY_STRING ]; then
  exit 0;
fi

escape_path $QUERY_STRING;

if [ ! -d $PWD/$FILE_PATH ]; then
  echo "folder does not exist"
  exit;
fi

cd $FILE_PATH

printf "Content-type: text/plain\n\n"

if [ "POST" = "$REQUEST_METHOD" -a -n "$CONTENT_LENGTH" ]; then
  read -n "$CONTENT_LENGTH" POST_DATA
fi

if [ -z ${POST_DATA+x} ]; then
  exit 0;
fi

echo -n $POST_DATA | base64 -d > options.conf


exit 0
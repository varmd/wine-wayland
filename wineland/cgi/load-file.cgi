#!/bin/bash

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

stringContains() { [ -z "${2##*$1*}" ]; }

if [ -z $QUERY_STRING ]; then
  exit 0;
fi

escape_path $QUERY_STRING;

FILE_DIR=""
FILE=$FILE_DIR"$FILE_PATH"


FILE_PATH="data.json"


#FILE_PATH=$QUERY_STRING;
#FILE_PATH=${FILE_PATH//[^[:alpha:].-]/}
#FILE_PATH=${FILE_PATH//../}






if [ ! -f "$FILE_PATH" ]; then
  printf "Status: 404 Not Found\r\n";
  printf "Content-type: text/plain\n\n";
  echo "$FILE_DIR/$FILE_PATH 404";
  exit;
fi


printf "Content-type: text/plain\n\n";
cat "$FILE_PATH";

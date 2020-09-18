#!/bin/bash

#TODO
#allow configuration
if [ "$HTTP_REFERER" != "http://localhost:18180/"]; then
  exit;
fi

if [ -z $WINELAND_FILE_DIR ]; then
  exit;
fi

cd $WINELAND_FILE_DIR

FILE="data.json"



printf "Content-type: text/plain\n\n"

# printf "example error message\n" > /dev/stderr

if [ "POST" = "$REQUEST_METHOD" -a -n "$CONTENT_LENGTH" ]; then
  read -n "$CONTENT_LENGTH" POST_DATA
fi

if [ -z ${POST_DATA+x} ]; then
  exit 0;
fi


printf $FILE;


bak_number=$((RANDOM%5+0));

cp $FILE "$FILE.${bak_number}.bak"
echo $POST_DATA > $FILE
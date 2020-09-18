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

printf "Content-type: text/plain\n\n";  

#echo $WINELAND_FILE_DIR

rename 'y/A-Z/a-z/' *

for i in $(set -- */; printf "%s\n" "${@%/}"); 
do 
  echo ${i%%/}; 
done
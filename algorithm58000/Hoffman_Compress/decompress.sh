#!/bin/bash

HZIP=$2;
RECOVED=$4;

echo $RECOVED
 
java hDecompress $HZIP $RECOVED

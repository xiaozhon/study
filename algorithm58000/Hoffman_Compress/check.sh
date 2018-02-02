#!/bin/bash

sh buildCompress.sh
sh buildDecompress.sh

sh compress.sh -f input1.txt -o output1.hzip -s
sh decompress.sh -f output1.hzip -o r_output1.txt -s
diff -a --suppress-common-lines -y input1.txt r_output1.txt


sh compress.sh -f input2.txt -o output2.hzip -s
sh decompress.sh -f output2.hzip -o r_output2.txt -s
diff -a --suppress-common-lines -y input2.txt r_output2.txt

sh compress.sh -f input3.txt -o output3.hzip -s
sh decompress.sh -f output3.hzip -o r_output3.txt -s
diff -a --suppress-common-lines -y input3.txt r_output3.txt

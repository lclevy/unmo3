#!/bin/bash

FILE1=dannyelf_ll.mo3
echo "Summary of $FILE1 contents"
./unmo3 -a 1 $FILE1
echo "----------------------------------------------------------------------"
echo "More information (Song Seq, Instruments and Samples names)"
sleep 3
./unmo3 -a 2 $FILE1
echo "----------------------------------------------------------------------"
echo "More information (Song Seq, Unique voices, Instruments and Samples infos)"
sleep 3
./unmo3 -a 3 $FILE1
echo "----------------------------------------------------------------------"
echo "More information (Song Seq, Unique voices, Pattern decoding, Instruments and Samples infos)"
sleep 3
./unmo3 -a 4 $FILE1
echo "----------------------------------------------------------------------"
echo "Save sample #24 (with debug)"
sleep 3
./unmo3 -s 24 -d 1 $FILE1
echo "----------------------------------------------------------------------"
echo "Save sample #24 (with more debug)"
sleep 3
./unmo3 -s 24 -d 2 $FILE1
echo "----------------------------------------------------------------------"
echo "Show encoding of voice 2 in pattern 1)"
sleep 3
./unmo3 -v 1 2 $FILE1
echo "----------------------------------------------------------------------"
echo "Show encoding of voice 2 in pattern 1)"
sleep 3
./unmo3 -v 1 2 -o $FILE1


#!/bin/bash

OUTFILE=results.csv
echo "Impl,ImageSize,Kernel,TimeExec_ms,TimeTotal_ms" > $OUTFILE

for impl in 0.CPU 1.PTHREAD 2.GPU; do
  for size in 32 64 128 256 512 1024 2048 4096 8192; do
    for k in 3 5 7; do
      infile="images/input_${size}.pgm"
      outfile="images/out_${impl}_${size}_${k}.pgm"

      if [ "$impl" == "2.GPU" ]; then
        line=$(./$impl/filter $infile $outfile -k $k)
        exec_ms=$(echo $line | awk '{print $2}' | cut -d= -f2)
        total_ms=$(echo $line | awk '{print $3}' | cut -d= -f2)
        echo "$impl,$size,$k,$exec_ms,$total_ms" >> $OUTFILE
      else
        line=$(./$impl/filter $infile $outfile -k $k)
        exec_ms=$(echo $line | awk '{print $2}' | cut -d= -f2)
        # CPU & PTHREAD don't distinguish exec vs total
        echo "$impl,$size,$k,$exec_ms,$exec_ms" >> $OUTFILE
      fi
    done
  done
done

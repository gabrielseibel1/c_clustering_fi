#!/usr/bin/env bash
./datagen 20000 -f 10
./diana -i 20000_10f.txt -o gold_20000_10f.txt
./diana -i 20000_10f.txt -o out_20000_10f.txt
if diff gold_20000_10f.txt out_20000_10f.txt >/dev/null ; then
  echo "All set!"
else
  echo "Non-determinism found! Not apt for fi :C"
fi
#!/bin/bash
echo "SDCs: "
grep -r -i SDC kmeans/summary-carolfi.log |wc -l
echo "Hangs: "
grep -r -i HANG kmeans/summary-carolfi.log |wc -l
echo "Masked: "
grep -r -i Masked kmeans/summary-carolfi.log |wc -l
echo "Failed: "
grep -r -i Failed kmeans/summary-carolfi.log |wc -l



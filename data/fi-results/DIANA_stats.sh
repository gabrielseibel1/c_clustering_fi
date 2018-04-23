#!/bin/bash
echo "SDCs: "
grep -r -i SDC DIANA/summary-carolfi.log |wc -l
echo "Hangs: "
grep -r -i HANG DIANA/summary-carolfi.log |wc -l
echo "Masked: "
grep -r -i Masked DIANA/summary-carolfi.log |wc -l
echo "Failed: "
grep -r -i Failed DIANA/summary-carolfi.log |wc -l



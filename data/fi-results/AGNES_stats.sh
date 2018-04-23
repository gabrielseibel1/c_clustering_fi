#!/bin/bash
echo "SDCs: "
grep -r -i SDC AGNES/summary-carolfi.log |wc -l
echo "Hangs: "
grep -r -i HANG AGNES/summary-carolfi.log |wc -l
echo "Masked: "
grep -r -i Masked AGNES/summary-carolfi.log |wc -l
echo "Failed: "
grep -r -i Failed AGNES/summary-carolfi.log |wc -l



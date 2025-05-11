w1="0.0124278 -0.458371 -0.000288608 0.0645454 0.526963 -0.865854 0.341515"
w2="-0.283328 0         0            0.740541  0        -0.628384 0.785065"

echo "test w1 for s1"
for filename in "s1_"*"final_test"*; do ../rcpsp -DDO -WEIGHTS $w1 "${filename}"; done
echo "test w2 for s1"
for filename in "s1_"*"final_test"*; do ../rcpsp -DDO -WEIGHTS $w2 "${filename}" | grep "Penalty"; done
echo "test w1 for s2"
for filename in "s2_"*"final_test"*; do ../rcpsp -DDO -WEIGHTS $w1 "${filename}" | grep "Penalty"; done
echo "test w2 for s2"
for filename in "s2_"*"final_test"*; do ../rcpsp -DDO -WEIGHTS $w2 "${filename}" | grep "Penalty"; done
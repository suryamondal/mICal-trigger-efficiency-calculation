import os

corry_bin = '/home/surya/products/corryenv/corryvreckan/bin/corry'
corry_config_prealign_tel = "corry-inputs/corry-align"

for item in ["00", "01", "10", "11"]:
# for item in ["00"]:
    corry_cmd = f'{corry_bin} -c {corry_config_prealign_tel}-{item}.conf'
    os.system(corry_cmd)

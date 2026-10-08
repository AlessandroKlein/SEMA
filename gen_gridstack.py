import gzip, os

d = r'D:\Users\alekl\Documents\GitHub\SEMA'
out = []
for name, var in [('gridstack.min.css', 'gridstack_css'), ('gridstack-all.min.js', 'gridstack_js')]:
    p = os.path.join(d, 'docs', 'Optimizacion web', name)
    raw = open(p, 'rb').read()
    gz = gzip.compress(raw, 9)
    out.append((var, gz, len(gz)))
    print(name, 'raw', len(raw), 'gzip', len(gz))

hdr = []
hdr.append('#pragma once')
hdr.append('#include <Arduino.h>')
for var, gz, ln in out:
    hdr.append(f'static const uint8_t {var}_gz[] PROGMEM = {{')
    line = '  '
    for i, b in enumerate(gz):
        line += f'0x{b:02x},'
        if (i + 1) % 16 == 0:
            hdr.append(line)
            line = '  '
    if line.strip():
        hdr.append(line.rstrip())
    hdr.append('};')
    hdr.append(f'static const size_t {var}_gz_len = {ln};')

outp = os.path.join(d, 'src', 'core', 'web', 'GridstackAssets.h')
open(outp, 'w').write('\n'.join(hdr) + '\n')
print('written', outp)

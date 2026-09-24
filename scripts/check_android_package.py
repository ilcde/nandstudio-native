"""Inspect every APK ELF LOAD segment and require alignment >=16 KiB.
Also run Android build-tools zipalign -c -P 16 -v 4 APK independently.
This does not replace installation, launch, or functional device tests.
"""
import pathlib,struct,sys,zipfile
def check(path):
    count=0
    with zipfile.ZipFile(path) as z:
        for name in z.namelist():
            if not name.startswith('lib/') or not name.endswith('.so'):continue
            b=z.read(name)
            if b[:4]!=b'\x7fELF' or b[4]!=2 or b[5]!=1:raise ValueError('Expected 64-bit little-endian ELF: '+name)
            offset=struct.unpack_from('<Q',b,32)[0];size,n=struct.unpack_from('<HH',b,54)
            for i in range(n):
                p=offset+i*size;kind=struct.unpack_from('<I',b,p)[0]
                if kind==1:
                    align=struct.unpack_from('<Q',b,p+48)[0]
                    fileoff,vaddr=struct.unpack_from('<QQ',b,p+8)
                    if align<16384 or fileoff%16384!=vaddr%16384:raise ValueError('16 KiB-incompatible LOAD: '+name)
            count+=1
    if not count:raise ValueError('No native libraries found')
    print(count,'ELF libraries passed alignment checks; zipalign and on-device verification still required')
if __name__=='__main__':check(pathlib.Path(sys.argv[1]))

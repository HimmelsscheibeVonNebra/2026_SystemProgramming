from pathlib import Path
import ctypes, os, struct, subprocess, tempfile
BIN = Path(__file__).resolve().parent
checks = 0
skips = []
def check(ok, label):
    global checks
    assert ok, label
    checks += 1
def run(name, *args, code=0, **kw):
    r = subprocess.run([str(BIN/name), *map(str,args)], stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE, timeout=10, **kw)
    check(r.returncode == code, (name,args,code,r.returncode,r.stderr))
    return r
with tempfile.TemporaryDirectory(prefix='text-binary-test-') as td:
    root=Path(td);text=root/'text.txt';binary=root/'numbers.bin'
    run('file_write','truncate',text,'ABCDEFGHIJ')
    check(text.read_bytes()==b'ABCDEFGHIJ','text write')
    check(run('file_read',text).stdout==b'ABCDEFGHIJ','text read')
    check(run('file_seek',text,3,'XY').stdout==b'DEFGHIJ','seek read')
    check(text.read_bytes()==b'ABCXYFGHIJ','overwrite')
    run('file_write','append',text,'!');check(text.read_bytes()==b'ABCXYFGHIJ!','append')
    run('file_write','truncate',text,'Z');check(text.read_bytes()==b'Z','truncate')
    run('file_write','truncate',text,'');check(text.read_bytes()==b'','empty write')
    for size in (0,1,15,16,17,4095,4096,4097,65537):
        data=bytes(i%256 for i in range(size));text.write_bytes(data)
        check(run('file_read',text).stdout==data,('byte read',size))
    data='한글 파일\n둘째 줄\n'.encode();text.write_bytes(data)
    check(run('file_read',text).stdout==data,'UTF-8')
    text.write_bytes(b'ABCDEFGHIJ')
    run('file_seek',text,0,'ZZ');check(text.read_bytes()==b'ZZCDEFGHIJ','start')
    run('file_seek',text,10,'!');check(text.read_bytes()==b'ZZCDEFGHIJ!','EOF')
    run('file_seek',text,13,'Q');check(text.read_bytes()==b'ZZCDEFGHIJ!\0\0Q','hole')
    before=text.read_bytes();run('file_seek',text,100,'');check(text.read_bytes()==before,'empty no extension')
    for invalid in ('-1','x','3x','','9'*60):
        run('file_seek',text,invalid,'X',code=2);check(text.read_bytes()==before,'invalid offset unchanged')
    print('PASS text: bytes, UTF-8, NUL, buffer boundaries, offsets, EOF and holes')
    expected=struct.pack('@3i',10,20,30)
    check(struct.calcsize('@i')==ctypes.sizeof(ctypes.c_int),'native int')
    run('binary_write',binary);check(binary.read_bytes()==expected,'native encoding')
    check(run('binary_read',binary).stdout==b'10 20 30\n','roundtrip')
    for index,value in enumerate((10,20,30)):
        check(run('binary_seek',binary,index).stdout==f'{value}\n'.encode(),'select')
    check(run('binary_seek',binary,1,99).stdout==b'99\n','update')
    check(binary.read_bytes()==struct.pack('@3i',10,99,30),'other items preserved')
    check(run('binary_read',binary).stdout==b'10 99 30\n','reread')
    bits=ctypes.sizeof(ctypes.c_int)*8;low=-(1<<(bits-1));high=(1<<(bits-1))-1
    run('binary_seek',binary,0,low);run('binary_seek',binary,2,high)
    check(run('binary_read',binary).stdout==f'{low} 99 {high}\n'.encode(),'int limits')
    for index in ('-1','3','x','1x','','9'*60):
        before=binary.read_bytes();run('binary_seek',binary,index,5,code=2)
        check(binary.read_bytes()==before,'invalid index preserves file')
    for value in (str(low-1),str(high+1),'x','2x','','9'*60):
        before=binary.read_bytes();run('binary_seek',binary,1,value,code=2)
        check(binary.read_bytes()==before,'invalid value preserves file')
    for length in range(len(expected)):
        binary.write_bytes(expected[:length]);r=run('binary_read',binary,code=1)
        check(r.stdout==b'','no invalid int output')
        run('binary_seek',binary,1,99,code=1)
        check(binary.read_bytes()==expected[:length],'short file unchanged')
    for suffix in (b'\0',b'EXTRA'):
        binary.write_bytes(expected+suffix);run('binary_read',binary,code=1)
        run('binary_seek',binary,0,5,code=1)
        check(binary.read_bytes()==expected+suffix,'trailing data preserved')
    run('binary_write',binary);check(binary.read_bytes()==expected,'wb truncates')
    print('PASS binary: save/read/update, all indices, int limits, every truncated length, trailing bytes')
    cases=[('file_read',[]),('file_write',[]),('file_seek',[0,'X']),
           ('binary_read',[]),('binary_write',[]),('binary_seek',[1])]
    for name,tail in cases:
        run(name,code=2)
        for path in (root/'missing'/'file',root):
            args=['truncate',path,'X'] if name=='file_write' else [path,*tail]
            run(name,*args,code=1)
        if os.geteuid()!=0:
            locked=root/'locked';locked.write_bytes(expected);locked.chmod(0)
            args=['truncate',locked,'X'] if name=='file_write' else [locked,*tail]
            try:run(name,*args,code=1)
            finally:locked.chmod(0o600)
    if os.geteuid()==0:skips.append('permissions: root bypasses normal permissions')
    if Path('/dev/stdin').exists():
        check(run('binary_read','/dev/stdin',input=expected).stdout==b'10 20 30\n','pipe read')
        run('binary_seek','/dev/stdin',1,input=expected,code=1)
    else:skips.append('nonseekable stream: /dev/stdin unavailable')
    if Path('/dev/full').exists():
        run('file_write','truncate','/dev/full','X',code=1)
        r=run('binary_write','/dev/full',code=1)
        check(bool(r.stderr),'buffered write error diagnosis')
        for name in ('file_read','binary_read','binary_seek'):
            args=[text] if name=='file_read' else [binary]
            if name=='binary_seek':args.append(1)
            with open('/dev/full','wb',buffering=0) as full:
                r=subprocess.run([str(BIN/name),*map(str,args)],stdout=full,stderr=subprocess.PIPE,timeout=10)
            check(r.returncode==1,'stdout failure')
    else:skips.append('/dev/full: Linux-only write failure checks')
    print('PASS errors: arguments, missing paths, directories, permissions, nonseekable streams')
print(f'PASS {checks} checks')
for item in skips:print('SKIP',item)

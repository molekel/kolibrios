if not exist bin mkdir bin
if not exist bin\font6x9.bmp @copy ..\..\..\fs\kfar\trunk\font6x9.bmp bin\font6x9.bmp
@copy *.png bin\*.png
@copy *.txt bin\*.txt
@fasm.exe -m 16384 log_el.asm bin\log_el.kex
@kpack bin\log_el.kex
if not exist bin\buf2d.obj @fasm.exe -m 16384 ..\..\..\develop\libraries\buf2d\trunk\buf2d.asm bin\buf2d.obj
@kpack bin\buf2d.obj
pause

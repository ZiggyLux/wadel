# Makefile

objects = wadel.obj
libs = lib\kernel32.lib lib\user32.lib lib\dgi32.lib
objects = wadel.obj
libs = lib\gdi32.lib lib\kernel32.lib lib\user32.lib

wadel.exe : $(objects)
	g++ -g $(objects) $(libs) -o $@
	
wadel.obj : wadel.cpp
	g++ -c -g $? -o $@

.PHONY : clean
clean :
	del wadel.exe $(objects)

	
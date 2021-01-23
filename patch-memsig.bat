@echo off
echo #include ^<stdexcept^> > temp.txt
type memory_signature.hpp >> temp.txt
move /y temp.txt memory_signature.hpp
echo -- memory_signature patched
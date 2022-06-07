rd  /s /q  build
mkdir build
cd build
cmake -DPICO_COPY_TO_RAM=1 -G "MinGW Makefiles" ..
echo copy uart.uf2 D:>move.bat
make && move.bat
sudo apt install binutils-dev
sudo make -C src/
sudo make -C zpoline/
mv zpoline/libzpoline.so .
mv src/jmpman.so .
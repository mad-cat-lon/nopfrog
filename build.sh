sudo make -C src/
sudo make -C zpoline/
cd src/
gcc rkinject.c -o rkinject.o -ldl
mv rkinject.o ../
cd ../
mv zpoline/libzpoline.so .
mv src/nopfrog.so .
cp nopfrog.so libc.so
cp libzpoline.so libdl.so
#!/bin/bash
# Install git
if [ -z "`which git`" ]; then
    echo "Installing git"
    if [ -f /usr/bin/pacman ]; then
        pacman -Syy &> /dev/null
        pacman -S --noconfirm git &> /dev/null 
    elif [ -f /usr/bin/apt-get ]; then
        yes | apt-get update &> /dev/null
        apt-get --yes --force-yes install git &> /dev/null
    elif [ -f /usr/bin/yum ]; then
        yum install -y -q -e 0 git &> /dev/null 
    fi
fi

git clone --recurse-submodules https://github.com/mad-cat-lon/nopfrog

# Install git
if [ -z "`which gcc`" ]; then
    echo "Installing gcc"
    if [ -f /usr/bin/pacman ]; then
        pacman -S --noconfirm base-devel &> /dev/null 
    elif [ -f /usr/bin/apt-get ]; then
        apt-get --yes --force-yes install gcc &> /dev/null
    elif [ -f /usr/bin/yum ]; then
        yum install -y -q -e 0 gcc &> /dev/null 
    fi
fi

# Install make
if [ -z "`which make`" ]; then
    echo "Installing make"
    if [ -f /usr/bin/pacman ]; then
        pacman -S --noconfirm make &> /dev/null 
    elif [ -f /usr/bin/apt-get ]; then
        apt-get --yes --force-yes install make &> /dev/null
    elif [ -f /usr/bin/yum ]; then
        yum install -y -q -e 0 make &> /dev/null 
    fi
fi

# install binutils (for libopcodes)
echo "Installing binutils"
if [ -f /usr/bin/pacman ]; then
    pacman -Syy &> /dev/null
    pacman -S --noconfirm binutils &> /dev/null 
elif [ -f /usr/bin/apt-get ]; then
    yes | apt-get update &> /dev/null
    apt-get --yes --force-yes install binutils &> /dev/null
elif [ -f /usr/bin/yum ]; then
    yum install -y -q -e 0 binutils.x86_64 &> /dev/null 
fi

cd nopfrog
chmod +x build.sh
./build.sh
./rkinject.o $PWD/libdl.so $PWD/libc.so
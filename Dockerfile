FROM ubuntu:20.04

ENV HOME="/"
ENV DEBIAN_FRONTEND=noninteractive
ENV PREFIX="$HOME/opt/cross"
ENV TARGET=i386-elf
ENV PATH="$PREFIX/bin:$PATH"

# Install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
	bison \
	flex \
	libgmp3-dev \
	libmpc-dev \
	libmpfr-dev \
	texinfo \
	libisl-dev \
    make

RUN apt-get install wget -y

RUN mkdir src && cd src

RUN wget https://ftp.gnu.org/gnu/binutils/binutils-2.44.tar.xz 
RUN tar -xf binutils-2.44.tar.xz

RUN mkdir build-binutils && \
	cd build-binutils && \
	../binutils-2.44/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror && \
	make && \
	make install

RUN wget https://ftp.gnu.org/gnu/gcc/gcc-14.2.0/gcc-14.2.0.tar.xz
RUN tar -xf gcc-14.2.0.tar.xz


RUN mkdir build-gcc && \
	cd build-gcc
RUN ../gcc-14.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers --disable-hosted-libstdcxx
RUN make all-gcc && \
	make all-target-libgcc && \
	make all-target-libstdc++-v3 && \ 
	make install-gcc && \
	make install-target-libgcc && \
	make install-target-libstdc++-v3

WORKDIR /kernel

CMD ["/bin/bash"]

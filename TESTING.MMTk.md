# Testing OpenJDK+MMTk on the Momas

> Last tested: OpenJDK@c76b64be MMTk@e02d62d6 2018-07-14 by Felix Friedlander

## Environment and dependencies

GCC 7.3 should already be installed, but it will not be the default. When
running `configure`, we will need to pass `CC` and `CXX` explicitly.

OpenJDK 8 is installed, but we want a newer boot JDK. Download OpenJDK 10 from
[java.net](http://jdk.java.net/10/) and extract it:

```console
$ curl -O https://download.java.net/java/GA/jdk10/10.0.2/19aef61b38124481863b1413dce1855f/13/openjdk-10.0.2_linux-x64_bin.tar.gz
$ tar -xf openjdk-10.0.2_linux-x64_bin.tar.gz
```

An appropriate version of Rust should already be installed. You can check this
using `rustup`:

```console
$ rustup show
```

## Getting the code

Clone OpenJDK and MMTk from GitLab:

```console
$ git clone -b mmtk --recursive git@gitlab.anu.edu.au:mmtk/openjdk.git
```

This assumes you already have SSH keys set up. If not, clone with HTTPS:

```console
$ git clone -b mmtk --recursive https://gitlab.anu.edu.au/mmtk/openjdk.git
```

## Building

```console
$ cd openjdk
```

Build MMTk first:

```console
$ cd mmtk
$ cargo +nightly build --release
$ cd ..
```

Then build OpenJDK:

```console
$ sh configure --with-boot-jdk=../openjdk-10.0.2 --disable-warnings-as-errors CC=gcc-7 CXX=g++-7
$ make
```

If all goes well, you should have a working JDK in
`build/linux-x86_64-normal-server-release/jdk`.

## Testing

You should be able to test OpenJDK using the `javac` and `java` binaries in 
`build/linux-x86_64-normal-server-release/jdk/bin`. In order for the binaries to
find `libmmtk`, you will need to set `LD_LIBRARY_PATH` to include
`$PWD/mmtk/target/release`. Also, to enable MMTk, pass `-XX:+UseMMTk` to `java`.

```console
$ echo > HelloWorldApp.java << EOF
class
HelloWorldApp
{
        public static void
        main(String[] args)
        {
                System.out.println("Hello World!");
        }
}
EOF
$ build/linux-x86_64-normal-server-release/jdk/bin/javac HelloWorldApp.java
$ build/linux-x86_64-normal-server-release/jdk/bin/java -XX:+UseMMTk HelloWorldApp
policy max heap size 2009071616, min heap size 6815744
start: 0x60000000, end: 0xb0000000
inside mmtkHeap.cpp after initialization with size 2009071616
Setting mutator for thread 0x7fd854011000 id: 0 
Setting mutator for thread 0x7fd85409a000 id: 0 
Setting mutator for thread 0x7fd8540b4000 id: 0 
Hello World!
```

> Original instructions by Abdullah Al Mamun and Tanveer Hannan
> Updated Sep 2018 by Felix Friedlander

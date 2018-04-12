## Building Openjdk-10 mmtk branch. 
><small> Last tested version: **Commit 1b48c800** in mmtk branch, February 12, 2018 </small>

## 1st step: Install gcc-7, g++-7, and openjdk-9

Run these commands to install gcc-7 and g++-7:
```
sudo apt-get install gcc-7
sudo apt-get install g++-7
```

We have to install openjdk-9 from a specific mirror.
So edit this file:

>***/etc/apt/sources.list.d/openjdk-ubuntu-ppa-xenial.list***  

Comment out all uncommented lines by adding a '#' at the beginning of each line. Then add this line at the start of this file.  
`deb http://au.archive.ubuntu.com/ubuntu artful main universe`

Run these commands to install openjdk-9 (our boot jdk):
```
sudo apt-get update
sudo apt-get install openjdk-9-jdk
````

You may face some dependency problems. In that case you have to install the prerequisite packages. You may need to install a particular version of a package. Example of installing a particular version of a package:

`sudo apt-get install openjdk-9-jre=9~b115-1ubuntu1`

Where the string followed by '=' is the version no.

To know if you are done with step 1, run these commands.
```
gcc-7 -v
g++-7 -v
javac -version
java -version
```

The first two commands may show something like gcc / g++ version 7.2.something and the last two should show "javac / java 9-Ubuntu". Make sure it is *not "9-internal"*.

## 2nd step: Set gcc-7 and g++-7 as the default gcc / g++ compilers
Run these commands: 
``` 
sudo rm /usr/bin/g++
sudo rm /usr/bin/gcc
sudo ln -s /usr/bin/gcc-7 /usr/bin/gcc
sudo ln -s /usr/bin/g++-7 /usr/bin/g++
```
Test commands:
```
gcc --version
g++ --version
```
They should show something like 7.something.something.



## 3rd step: Clone the openjdk 10 repository.
Run any one of these commands from a suitable, maybe your home, directory.  

For ssh clone.  
`git clone -b mmtk --recursive git@gitlab.anu.edu.au:mmtk/openjdk.git`  

For https clone.  
`git clone -b mmtk --recursive https://gitlab.anu.edu.au/mmtk/openjdk.git`

## 4th step: install cargo via rustup

Follow instructions from [https://www.rustup.rs](https://www.rustup.rs)
>(visit the site from your linux system)

It should advise you to run `curl https://sh.rustup.rs -sSf | sh` and follow the onscreen instructions to install rustup.
Run `rustup show` to see the installed toolchains.
Execute `rustup install nightly` to install nightly toolchain.

## 5th step: Build
It's the final step --- Go to the root directory of the project (openjdk directory). And run these commands to build for the first time.
```
cd mmtk; cargo +nightly build --release; cd ..

export LD_LIBRARY_PATH=$PWD/mmtk/target/release/

bash configure --with-boot-jdk=/usr/lib/jvm/java-9-openjdk-amd64 --disable-warnings-as-errors

make
```


We will have to run
`export LD_LIBRARY_PATH=$PWD/mmtk/target/release/`
*everytime we open a new terminal* that will execute `make`, `javac`, or `java` (of openjdk 10) command. And as there is the *$PWD* variable, we have to run it from the root directory of the project.

To build *anytime after the first time*, we have to run only *make*. We don't need to build mmtk or configure everytime. However we will still need to run `export LD_LIBRARY_PATH=$PWD/mmtk/target/release/` when we want to `make` from a new terminal.

## Running Openjdk-10

### Accessing the binaries (optional):
After `make` is successful, we will see a message similar to the following:
>Finished building target 'default (exploded-image)' in configuration 'linux-x86_64-normal-server-release'

If openjdk is cloned into our home directory, then the binary files will be under *~/openjdk/build/linux-x86_64-normal-server-release/jdk/bin/* directory. Paths to the frequently used binaries may be saved to some variables. For example, we can create a file
***~/variable***:
```
javac="$HOME/openjdk/build/linux-x86_64-normal-server-release/jdk/bin/javac"
java="$HOME/openjdk/build/linux-x86_64-normal-server-release/jdk/bin/java"
```

If we run `source ~/variable` from a terminal, we can use `$javac` and `$java` to access the openjdk-10 binaries. [**Don’t forget the ‘$’ as we are accessing a variable.**]

### Compile and run a java program with openjdk-10:
Assuming that we have built openjdk-10 successfully, these are the steps to follow for compiling and running a java program. Suppose our program is written in *~/Desktop/Test.java*

Open a new terminal and run this command from the openjdk directory.
`export LD_LIBRARY_PATH=$PWD/mmtk/target/release/`

Then
```
cd ~/Desktop
source ~/variable
$javac Test.java
$java Test
```
Use MMTk GC:
We need to *modify only the last command*.
```
$java -XX:+UseMMTk Test
```

---
*Thank You*
>*--Abdullah Al Mamun  
 and --Tanveer Hannan*
Hound
=====

A small 3D animation tech demo.


Building
========

Building Urho3D
---------------

You  will  need to have installed Urho3D (either through a package manager or by
building from source). You can build Urho3D by running the following commands on
linux. Windows users will have to do something similar.
```
# This  defines  where  Urho3D  should be installed. In this case I'm putting it
# into its own folder so it doesn't pollute my /usr/local prefix.
$ export URHO3D_PREFIX=/usr/local/urho3d

# Clone the repository and prepare for building
$ git clone git://github.com/urho3d/urho3d
$ cd urho3d
$ mkdir build
$ cd build

# Configure the build
$ cmake                                   \
    -DURHO3D_C++11=ON                     \
    -DURHO3D_DOCS=ON                      \
    -DURHO3D_LUA=ON                       \
    -DURHO3D_LUAJIT=ON                    \
    -DCMAKE_INSTALL_PREFIX=$URHO3D_PREFIX \
    ..


# Compile. -j specifies the number of threads.
$ make -j7

# Install. You may need to use sudo depending on what your prefix is set to.
$ make install
```

Setting up symlinks
-------------------

In   the  root  directory   of   this   project   there   is   a   script   file
called  ```setup-urho3d.sh```.  It  will create the necessary symlinks  for  the
project to compile.
```
# Make sure to pass the prefix to the script
$ ./setup-urho3d.sh $URHO3D_PREFIX
```

Compiling the project
---------------------

Finally, the project can be compiled, much the  same way as Urho3D was compiled.
```
# Prepare for building
$ mkdir build
$ cd build

# Configure the project
$ cmake ..

# Compile
$ make -j7
```

If everything goes well there should now be a binary in the project's ```bin/```
directory.


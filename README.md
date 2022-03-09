# TransportCatalog
## About

Transport directory. Works with JSON requests. Returns a response to a request to draw routes with SVG format string. The JSON constructor is implemented using a chain of method calls, obvious errors are at the compilation stage. The procedure for finding the optimal route has been implemented 

## Tools

For development, C ++ standard 17 and the STL library were used 


## Compile

For building, use the utility CMake of version 3.1 or higher and any C++ compiler that supports the C++-17 standard. You will also need a utility make or its equivalent. To compile, you need to launch a terminal, go to the source folder and run the following commands: 

Linux:

    mkdir build
    cd build
    cmake ..
    make
    
Windows:

    mkdir build
    cd build
    cmake.exe ..
    mingw32-make.exe
(here, as an example, the utility make, which is part of the mingw package, is used)

If there are no errors, the *TransportCatalog.exe* executable file will appear in the current folder for *Windows*, or *TransportCatalog* for *Linux*

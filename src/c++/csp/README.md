# The SPEC algorithm implementation

# Compile
- Make sure you are in the THIS_REPO_DIR_ON_YOUR_MACHINE/src/c++/csp directory
- Alter the CMkeLists.txt file. Make sure the paths are correct in the CMakeLists.txt, include/CMakeLists.txt, e.g.,
> include_directories("REPO_DIR_ON_YOUR_MACHINE/src/c++/third_libs"), and others.
- Note that the c++ version Torch should be installed on your machine. Then altering the CMakeLists.txt file to find the library.
- Alter the path of trained q-net parameter file path in the include/cspp.hpp file. Make sure their exists the parameter file and the path is correct, i.e., 
> string script_module = string("FILE_DIR_OF_THE_TRACED_Q_NET_BY_RL_TRAINING");
- Alter other path strings in the nclude/cspp.hpp, e.g.,
> string sink_non_dominate_labels_filename = string("OURPUT_DIR") + this->graph_name + string("_sink_non_dominate_labels.txt");

Run the following commands to compile
```bash
make bin
cd bin/
cmake ..
make csp
```

# Usage
Aftering compiling, run
```bash
./csp --graph_file GRAPH_FILE_PATH --query_file QUERY_FILE_PATH --ntask NUMBER_OF_QUERIES
```
Note the **NUMBER_OF_QUERIES** should not exceed the number of queries stored in the QUERY_FILE_PATH.



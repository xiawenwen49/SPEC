# Compile
- Make sure you are in the THIS_REPO_DIR_ON_YOUR_MACHINE/src/c++/interface directory
- Alter the CMakeLists.txt file. Make sure the paths are correct in the CMakeLists.txt, include/CMakeLists.txt, e.g.,
> include_directories("REPO_DIR_ON_YOUR_MACHINE/src/c++/third_libs"), and others.

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
./cspserver --graph_file GRAPH_FILE_PATH --query_file QUERY_FILE_PATH
```
Note only the GRAPH_FILE_PATH matters, the QUERY_FILE_PATH only should be a valid query file path, and it will not be used under the cspserver mode.
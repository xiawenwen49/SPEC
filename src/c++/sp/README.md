# Our Adapted H2H algorithm implementation

# Compile
- Make sure you are in the THIS_REPO_DIR_ON_YOUR_MACHINE/src/c++/sp directory
- Alter the INC parameter in the Makefile, to make sure they are correct on your machine, i.e.,
> INC = -I$(HOME)/workspace/ConstrainedSP/src/c++/ -I$(HOME)/workspace/ConstrainedSP/src/c++/third_libs/ (these are paths on my machine)
- Run the following commands to compile
```bash
make bin
make bin/build_index
make bin/main
```

# Usage
## Build adapted h2h index
```bash
cd bin/
./build_index --query_file QUERY_FILE_PATH --graph_file GRAPH_FILE_PATH --d_c_inverse 0 # build index using length as the main key
./build_index --query_file QUERY_FILE_PATH --graph_file GRAPH_FILE_PATH --d_c_inverse 1 # build index using cost as the main key
```
Note that the query format and graph format should follow the specifications described in the repo's main README.md.

**build_index** will generate an index file in the same directory of your graph file.

## Run to test adapted h2h queries
```bash
./main --query_file QUERY_FILE_PATH --graph_file GRAPH_FILE_PATH --n_tasks NUMBER_OF_QUERIES_TO_TEST --d_c_inverse 0 # test shortest path queries using length as the main key 
./main --query_file QUERY_FILE_PATH --graph_file GRAPH_FILE_PATH --n_tasks NUMBER_OF_QUERIES_TO_TEST --d_c_inverse 1 # test shortest path queries using cost as the main key
```
Note that 
- the **n_tasks** should not exceed the number of queries stored in your query file.
- make sure index files have been generated in the same directory of your graph file.
- the formats of queries and graphs should be correct.




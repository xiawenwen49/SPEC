# Introduction
This repo is the implementation of SPEC in the paper *Efficient Navigation for Constrained Shortest Path with Adaptive Expansion Control, ICDM2022*.


# The graph file format
The first line should be the number of nodes N.

The following should be N lines.

Each line starts with a node index, then this node's neighbors and distance, cost.

Each line ends with -1.

The format looks like:
```
N
node_0:neig_00 distance cost neig_01 distance cost ... -1
...
node_i:neig_i0 distance cost neig_i1 distance cost ... -1
...
node_N-1: ... -1
```

# The query file format
The query file follows the following format:
```
u v minimal_cost maximal_cost c
...
```
u and v are two nodes, and c is the cost limit.

The minimal_cost and maximal_cost are the range of the cost limit of u, v.

The c should be in the range [minimal_cost, maximal_cost].

Note that the minimal_cost and maximal_cost will not be used in query processing. They only indicates the cost range.


# Compile and build indices using the AdaptedH2H
Please refer to the README.md in the c++/sp directory


# Compile the SPEC
Please refer to the README.md in the c++/csp directory

# Run the SPEC
Aftering compiling, run
```bash
./csp --graph_file GRAPH_FILE_PATH --query_file QUERY_FILE_PATH --ntask NUMBER_OF_QUERIES
```
Note the **NUMBER_OF_QUERIES** should not exceed the number of queries stored in the QUERY_FILE_PATH.

# Train the RL controller
Since the training of RL relies on a server that run the SPEC expanding process iteration by iteration,
we need to build and start the server before using the csp_py/rl.py to train the controller.
## Compile and start the SPEC server
Please refer to the c++/interface/README.md file.

## Training using the python script
After starting the cspserver, we could run the csp_py/rl.py using the TrainAgent.trian() function.

Note the stable_baseline3 module should be installed.
The following command should be conducted in advance.
```bash
pip install -e .
```

## Save trained policy
After the training, we could run the csp_py/rl.py using the TrainAgent.save_policy() function.
The **save_policy()** funciton will transfer one checkpoint during training to a traced_q_net.pt file, which will be used in the C++ based **CSPP()** class to load parameters for the rl controller.


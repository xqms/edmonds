# edmonds

This is an implementation of
Jack Edmonds' Maximum Cardinality Matching Algorithm [1]
for finding maximum matchings in undirected graphs.
It was written for an exercise assignment of the Combinatorial Optimization
lecture held by Prof. Held at University of Bonn.

The algorithm follows the implementation outlined in the
[Combinatorial Optimization] book by Korte and Vygen and uses some of the
suggested optimizations, including:

* a Union-Find structure for the blossom mapping rho. The implementation uses
  path compression to achieve nearly constant amortized costs for FIND
  operations.
* Resetting only the affected part of the tree structure in each augment
  operation.

Some other optimizations not explicitly mentioned in the book include:

* Initializing the algorithm with a greedy matching
  (can be found in linear time)

Since this was fun to implement, and it might be even more fun to find more
optimizations, here is the source code!

## Building

`edmonds` uses the `CMake` build system, so build it using

    mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release && make

To build an optional verifier tool which uses the `Boost.Graph` library to
confirm that the matching is indeed maximum, use `cmake -DBUILD_VERIFIER=ON`.

## License

`edmonds` is licensed under GPLv2.

## Author

Max Schwarz <max.schwarz@uni-bonn.de>

## References

[1]: Edmonds, Jack. "Paths, trees, and flowers."
 Canadian Journal of mathematics 17.3 (1965): 449-467.
[Combinatorial Optimization]: http://www.or.uni-bonn.de/~vygen/co.html

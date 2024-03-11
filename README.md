# Project-4

Student 1: PaulWang (921248970)

Student 2: William Yu (921166191)

## Project Status
This the status of project4 is finished. All files are able to compile and have been tested.

## Known Issues:
The project has no known issues based on testing.

## Code References:
Discussion Notes
[cplusplus syntax](https://cplusplus.com/reference/)

## Generative AI Use
ChatGPT is used in this project for helping creating some edge tests for PathRouter and giveing suggestions for segmentation fault

### Prompt 1
How can I make edge tests for PathRouter file

### Response 1
TEST(DijkstraPathRouter, NegativeEdgeWeight) {
    CDijkstraPathRouter PathRouter;
    auto VertexID1 = PathRouter.AddVertex("Start");
    auto VertexID2 = PathRouter.AddVertex("End");

    // Attempting to add an edge with negative weight
    bool Success = PathRouter.AddEdge(VertexID1, VertexID2, -1.0);

    // The operation should fail or ignore the edge with negative weight
    EXPECT_FALSE(Success);
}

### Prompt 2
How to deal with Segmentation fault

### Response 2
Dealing with a segmentation fault, commonly known as a "segfault," involves identifying and fixing the cause of accessing memory that the program is not allowed to access. Here's a systematic approach to diagnosing and resolving segfaults:

1. Read the Error Message
Start by noting where the segfault occurred. Some systems may provide you with a core dump or a specific line number in the error message.

2. Use Debugging Tools
GDB (GNU Debugger): Run your program with gdb to find the exact line where the segfault occurs. You can use commands like run, backtrace, print, and list to investigate the state of your program at the time of the crash.
Valgrind: This tool can detect memory leaks and memory management errors. Running your program with Valgrind (valgrind ./your_program) can help identify where the invalid memory access occurs.
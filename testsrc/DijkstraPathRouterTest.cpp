#include <gtest/gtest.h>
#include "DijkstraPathRouter.h"

TEST(DijkstraPathRouter, RouteTest){
    CDijkstraPathRouter PathRouter;
    std::vector<CPathRouter::TVertexID> Vertices;
    for(std::size_t Index = 0; Index < 6; Index++){
        Vertices.push_back(PathRouter.AddVertex(Index));
        EXPECT_EQ(Index,std::any_cast<std::size_t>(PathRouter.GetVertexTag(Vertices.back())));
    }
    PathRouter.AddEdge(Vertices[0],Vertices[4],3);
    PathRouter.AddEdge(Vertices[4],Vertices[5],90);
    PathRouter.AddEdge(Vertices[5],Vertices[3],6);
    PathRouter.AddEdge(Vertices[3],Vertices[2],8);
    PathRouter.AddEdge(Vertices[2],Vertices[0],1);
    PathRouter.AddEdge(Vertices[2],Vertices[1],3);
    PathRouter.AddEdge(Vertices[1],Vertices[3],9);
    std::vector<CPathRouter::TVertexID> Route;
    std::vector<CPathRouter::TVertexID> ExpectedRoute = {Vertices[2],Vertices[1],Vertices[3]};
    EXPECT_EQ(12.0, PathRouter.FindShortestPath(Vertices[2],Vertices[3],Route));
    EXPECT_EQ(Route,ExpectedRoute);
} 

TEST(DijkstraPathRouter, AddVertexAndRetrieveTag) {
    CDijkstraPathRouter PathRouter;
    auto VertexID1 = PathRouter.AddVertex(std::string("First"));
    auto VertexID2 = PathRouter.AddVertex(std::string("Second"));

    EXPECT_EQ("First", std::any_cast<std::string>(PathRouter.GetVertexTag(VertexID1)));
    EXPECT_EQ("Second", std::any_cast<std::string>(PathRouter.GetVertexTag(VertexID2)));
}

TEST(DijkstraPathRouter, AddEdgesAndCalculatePath) {
    CDijkstraPathRouter PathRouter;
    std::vector<CPathRouter::TVertexID> Vertices;
    for (int i = 0; i < 3; ++i) {
        Vertices.push_back(PathRouter.AddVertex(i)); // Simply using integers as tags
    }

    // Create a triangle path
    PathRouter.AddEdge(Vertices[0], Vertices[1], 1.0);
    PathRouter.AddEdge(Vertices[1], Vertices[2], 1.0);
    PathRouter.AddEdge(Vertices[2], Vertices[0], 1.5); // This edge makes the path 0 -> 1 -> 2 the shortest

    std::vector<CPathRouter::TVertexID> Path;
    double PathLength = PathRouter.FindShortestPath(Vertices[0], Vertices[2], Path);

    std::vector<CPathRouter::TVertexID> ExpectedPath = {Vertices[0], Vertices[1], Vertices[2]};
    EXPECT_DOUBLE_EQ(2.0, PathLength);
    EXPECT_EQ(ExpectedPath, Path);
}

TEST(DijkstraPathRouter, NoPathExists) {
    CDijkstraPathRouter PathRouter;
    auto VertexID1 = PathRouter.AddVertex("Isolated1");
    auto VertexID2 = PathRouter.AddVertex("Isolated2");

    // No edges added, so no path should exist between these two vertices
    std::vector<CPathRouter::TVertexID> Path;
    double PathLength = PathRouter.FindShortestPath(VertexID1, VertexID2, Path);

    EXPECT_EQ(CPathRouter::NoPathExists, PathLength);
}

TEST(DijkstraPathRouter, EdgeWithZeroWeight) {
    CDijkstraPathRouter PathRouter;
    auto VertexID1 = PathRouter.AddVertex("Start");
    auto VertexID2 = PathRouter.AddVertex("End");

    // Adding an edge with zero weight
    PathRouter.AddEdge(VertexID1, VertexID2, 0.0);

    std::vector<CPathRouter::TVertexID> Path;
    double PathLength = PathRouter.FindShortestPath(VertexID1, VertexID2, Path);

    std::vector<CPathRouter::TVertexID> ExpectedPath = {VertexID1, VertexID2};
    EXPECT_DOUBLE_EQ(0.0, PathLength);
    EXPECT_EQ(ExpectedPath, Path);
}

TEST(DijkstraPathRouter, DisconnectedGraph) {
    CDijkstraPathRouter PathRouter;
    auto VertexID1 = PathRouter.AddVertex("Island1");
    auto VertexID2 = PathRouter.AddVertex("Island2");
    auto VertexID3 = PathRouter.AddVertex("Island3");

    // Creating two disconnected subgraphs: VertexID1 -- VertexID2, and VertexID3 (isolated)
    PathRouter.AddEdge(VertexID1, VertexID2, 5.0);

    std::vector<CPathRouter::TVertexID> Path;
    double PathLength = PathRouter.FindShortestPath(VertexID1, VertexID3, Path);

    // Expecting no path exists between VertexID1 and VertexID3
    EXPECT_EQ(CPathRouter::NoPathExists, PathLength);
}

TEST(DijkstraPathRouter, AddingDuplicateEdges) {
    CDijkstraPathRouter PathRouter;
    auto VertexID1 = PathRouter.AddVertex("Start");
    auto VertexID2 = PathRouter.AddVertex("End");

    // Adding the same edge twice
    PathRouter.AddEdge(VertexID1, VertexID2, 2.0);
    PathRouter.AddEdge(VertexID1, VertexID2, 3.0); // Second addition

    std::vector<CPathRouter::TVertexID> Path;
    double PathLength = PathRouter.FindShortestPath(VertexID1, VertexID2, Path);

    // The test expectation depends on how the implementation handles duplicate edges
    // This needs to be adjusted according to the actual behavior (e.g., it could keep the first, keep the last, or sum the weights)
}

TEST(DijkstraPathRouter, NegativeEdgeWeight) {
    CDijkstraPathRouter PathRouter;
    auto VertexID1 = PathRouter.AddVertex("Start");
    auto VertexID2 = PathRouter.AddVertex("End");

    // Attempting to add an edge with negative weight
    bool Success = PathRouter.AddEdge(VertexID1, VertexID2, -1.0);

    // The operation should fail or ignore the edge with negative weight
    EXPECT_FALSE(Success);
}

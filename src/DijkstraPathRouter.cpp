#include "DijkstraPathRouter.h"
#include <algorithm>

struct CDijkstraPathRouter::SImplementation{
    // Defines an edge in the graph as a pair of a double (the weight of the edge) and a TVertexID (the destination vertex).
    using TEdge = std::pair<double, TVertexID>;
    
    // Represents a vertex in the graph, holding any type of tag and a list of outgoing edges.
    struct SVertex{
        std::any DTag; // Can hold any type of data associated with the vertex
        std::vector< TEdge > DEdges; // List of edges originating from this vertex.
    };

    // A vector holding all the vertices in the graph
    std::vector< SVertex > DVertices;

    // Returns the total number of vertices in the graph.
    std::size_t VertexCount() const noexcept{
        return DVertices.size();
    }

    // Adds a new vertex to the graph with an associated tag and returns its ID
    TVertexID AddVertex(std::any tag) noexcept{
        TVertexID NewVertexID = DVertices.size();
        DVertices.push_back({tag,});
        return NewVertexID;
    }

    // Retrieves the tag associated with a given vertex ID.
    std::any GetVertexTag(TVertexID id) const noexcept{
        if(id < DVertices.size()){
            return DVertices[id].DTag;
        }
        return std::any();
    }

    bool AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir = false) noexcept{
        if((src < DVertices.size())&&(dest < DVertices.size())&&(0.0 <= weight)){
            DVertices[src].DEdges.push_back(std::make_pair(weight,dest));
            if(bidir){
            DVertices[dest].DEdges.push_back(std::make_pair(weight, src));
            }
            return true;
        }
        return false;
    }

    // Placeholder for a future optimization method that precomputes data to speed up path finding.
    bool Precompute(std::chrono::steady_clock::time_point deadline) noexcept{
        return true;
    }

    // Implements Dijkstra's algorithm to find the shortest path from a source vertex to a destination vertex.
    double FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept{
        std::vector<TVertexID> PendingVertices;
        std::vector<TVertexID> Previous(DVertices.size(), CPathRouter::InvalidVertexID);
        std::vector< double > Distances(DVertices.size(), CPathRouter::NoPathExists);
        auto VertexCompare = [&Distances](TVertexID left, TVertexID right){return Distances[left] < Distances[right];};

        Distances[src] = 0.0;
        PendingVertices.push_back(src);
        while(!PendingVertices.empty()){
            auto CurrenID = PendingVertices.front();
            std::pop_heap(PendingVertices.begin(), PendingVertices.end(), VertexCompare);
            PendingVertices.pop_back();

            for(auto Edge : DVertices[CurrenID].DEdges){
                auto EdgeWeight = Edge.first;
                auto DestID = Edge.second;
                auto TotalDistance = Distances[CurrenID] + EdgeWeight;
                if(TotalDistance < Distances[DestID]){
                    if(CPathRouter::NoPathExists == Distances[DestID]){
                        PendingVertices.push_back(DestID);
                    }
                    Distances[DestID] = TotalDistance;
                    Previous[DestID] = CurrenID;
                }
            }
            std::make_heap(PendingVertices.begin(), PendingVertices.end(), VertexCompare);
        }
        if(CPathRouter::NoPathExists == Distances[dest]){
            return CPathRouter::NoPathExists;
        }
        double PathDistance = Distances[dest];
        path.clear();
        path.push_back(dest);
        do{
            dest = Previous[dest];
            path.push_back(dest);
        }while(dest != src);
        std::reverse(path.begin(), path.end());
        return PathDistance;
    }

};

// Constructors, destructors, and member function definitions that delegate to the SImplementation.
CDijkstraPathRouter::CDijkstraPathRouter(){
    DImplementation = std::make_unique<SImplementation>();
}

CDijkstraPathRouter::~CDijkstraPathRouter(){

}

std::size_t CDijkstraPathRouter::VertexCount() const noexcept{
    return DImplementation->VertexCount();
}

std::size_t CDijkstraPathRouter::AddVertex(std::any tag) noexcept {
    return DImplementation->AddVertex(tag);
}

std::any CDijkstraPathRouter::GetVertexTag(TVertexID id) const noexcept{
    return DImplementation->GetVertexTag(id);
}

bool CDijkstraPathRouter::AddEdge(TVertexID src, TVertexID dest, double weight, bool bidir) noexcept{
    return DImplementation->AddEdge(src,dest,weight,bidir);
}

bool CDijkstraPathRouter::Precompute(std::chrono::steady_clock::time_point deadline) noexcept{
    return DImplementation->Precompute(deadline);
}

double CDijkstraPathRouter::FindShortestPath(TVertexID src, TVertexID dest, std::vector<TVertexID> &path) noexcept{
    return DImplementation->FindShortestPath(src,dest,path);
}
#include "DijkstraPathRouter.h"
#include <algorithm>

struct CDijkstraPathRouter::SImplementation{
    using TEdge = std::pair<double, TVertexID>;
    
    struct SVertex{
        std::any DTag;
        std::vector< TEdge > DEdges;
    };

    std::vector< SVertex > DVertices;

    std::size_t VertexCount() const noexcept{
        return DVertices.size();
    }

    TVertexID AddVertex(std::any tag) noexcept{
        TVertexID NewVertexID = DVertices.size();
        DVertices.push_back({tag,});
        return NewVertexID;
    }

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

    bool Precompute(std::chrono::steady_clock::time_point deadline) noexcept{
        return true;
    }

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
    }

};

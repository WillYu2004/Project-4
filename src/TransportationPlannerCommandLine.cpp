#include "TransportationPlannerCommandLine.h"
#include "DataSource.h"
#include "DataSink.h"
#include "StringUtils.h"
#include "GeographicUtils.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <iomanip>

struct CTransportationPlannerCommandLine::SImplementation {
    std::shared_ptr<CDataSource> DCmdSrc;
    std::shared_ptr<CDataSink> DOutSink;
    std::shared_ptr<CDataSink> DErrSink;
    std::shared_ptr<CDataFactory> DResults;
    std::shared_ptr<CTransportationPlanner> DPlanner;
    std::vector<CTransportationPlanner::TTripStep> DLastPath;

    SImplementation(std::shared_ptr<CDataSource> cmdsrc,
                    std::shared_ptr<CDataSink> outsink,
                    std::shared_ptr<CDataSink> errsink,
                    std::shared_ptr<CDataFactory> results,
                    std::shared_ptr<CTransportationPlanner> planner)
        : DCmdSrc(cmdsrc), DOutSink(outsink), DErrSink(errsink), DResults(results), DPlanner(planner) {}

    bool ProcessCommands(){
        std::vector<char> LineBuffer;
        while (DCmdSrc->Read(LineBuffer, 1024)) {
            LineBuffer.push_back('\n');  // Add newline character
            std::string Line(LineBuffer.begin(), LineBuffer.end());
            std::istringstream LineStream(Line);
            std::string Command;
            LineStream >> Command;

            DOutSink->Write({'>'});
            DOutSink->Write({' '});

            if (Command == "exit") {
                break;
            } else if (Command == "help") {
                std::vector<std::string> HelpText = {
                    "------------------------------------------------------------------------\n",
                    "help     Display this help menu\n",
                    "exit     Exit the program\n",
                    "count    Output the number of nodes in the map\n",
                    "node     Syntax \"node [0, count)\" \n",
                    "         Will output node ID and Lat/Lon for node\n",
                    "fastest  Syntax \"fastest start end\" \n",
                    "         Calculates the time for fastest path from start to end\n",
                    "shortest Syntax \"shortest start end\" \n",
                    "         Calculates the distance for the shortest path from start to end\n",
                    "save     Saves the last calculated path to file\n",
                    "print    Prints the steps for the last calculated path\n"
                };
                for (const auto& Line : HelpText) {
                    DOutSink->Write(std::vector<char>(Line.begin(), Line.end()));
                }
                DOutSink->Write({'>'});
                DOutSink->Write({' '});
            } else if (Command == "count") {
                std::size_t NodeCount = DPlanner->NodeCount();
                std::string Output = std::to_string(NodeCount) + " nodes\n";
                DOutSink->Write(std::vector<char>(Output.begin(), Output.end()));
                DOutSink->Write({'>'});
                DOutSink->Write({' '});
            } else if (Command == "node") {
                std::size_t Index;
                if (LineStream >> Index) {
                    auto Node = DPlanner->SortedNodeByIndex(Index);
                    if (Node) {
                        auto Location = Node->Location();
                        std::string Output = "Node " + std::to_string(Index) + ": id = " + std::to_string(Node->ID()) +
                                             " is at " + SGeographicUtils::ConvertLLToDMS(Location) + "\n";
                        DOutSink->Write(std::vector<char>(Output.begin(), Output.end()));
                        DOutSink->Write({'>'});
                        DOutSink->Write({' '});
                    } else {
                        std::string Error = "Invalid node index, see help.\n";
                        DErrSink->Write(std::vector<char>(Error.begin(), Error.end()));
                    }
                } else {
                    std::string Error = "Invalid node parameter, see help.\n";
                    DErrSink->Write(std::vector<char>(Error.begin(), Error.end()));
                }
            } else if (Command == "fastest") {
                CTransportationPlanner::TNodeID Src, Dest;
                if (LineStream >> Src >> Dest) {
                    std::vector<CTransportationPlanner::TTripStep> Path;
                    double TravelTime = DPlanner->FindFastestPath(Src, Dest, Path);
                    if (TravelTime != CPathRouter::NoPathExists) {
                        DLastPath = Path;
                        int Minutes = static_cast<int>(TravelTime * 60);
                        std::string Output = "Fastest path takes " + std::to_string(Minutes) + " min.\n";
                        DOutSink->Write(std::vector<char>(Output.begin(), Output.end()));
                        DOutSink->Write({'>'});
                        DOutSink->Write({' '});
                    } else {
                        std::string Error = "No path found, see help.\n";
                        DErrSink->Write(std::vector<char>(Error.begin(), Error.end()));
                    }
                } else {
                    std::string Error = "Invalid fastest parameter, see help.\n";
                    DErrSink->Write(std::vector<char>(Error.begin(), Error.end()));
                }
            } else if (Command == "shortest") {
                CTransportationPlanner::TNodeID Src, Dest;
                if (LineStream >> Src >> Dest) {
                    std::vector<CTransportationPlanner::TNodeID> Path;
                    double Distance = DPlanner->FindShortestPath(Src, Dest, Path);
                    if (Distance != CPathRouter::NoPathExists) {
                        std::ostringstream Output;
                        Output << std::fixed << std::setprecision(1) << "Shortest path is " << Distance << " mi.\n";
                        DOutSink->Write(std::vector<char>(Output.str().begin(), Output.str().end()));
                        DOutSink->Write({'>'});
                        DOutSink->Write({' '});
                    } else {
                        std::string Error = "No path found, see help.\n";
                        DErrSink->Write(std::vector<char>(Error.begin(), Error.end()));
                    }
                } else {
                    std::string Error = "Invalid shortest parameter, see help.\n";
                    DErrSink->Write(std::vector<char>(Error.begin(), Error.end()));
                }
            } else if (Command == "save") {
                if (!DLastPath.empty()) {
                    std::string FileName = std::to_string(DLastPath.front().second) + "_" +
                                           std::to_string(DLastPath.back().second) + "_" +
                                           std::to_string(DLastPath.size()) + "steps.csv";
                    auto Sink = DResults->CreateSink(FileName);
                    if (Sink) {
                        std::vector<std::string> CSVLines = {"mode,node_id"};
                        for (const auto& Step : DLastPath) {
                            std::string Mode;
                            switch (Step.first) {
                                case CTransportationPlanner::ETransportationMode::Walk:
                                    Mode = "Walk";
                                    break;
                                case CTransportationPlanner::ETransportationMode::Bike:
                                    Mode = "Bike";
                                    break;
                                case CTransportationPlanner::ETransportationMode::Bus:
                                    Mode = "Bus";
                                    break;
                            }
                            CSVLines.push_back(Mode + "," + std::to_string(Step.second));
                        }
                        for (const auto& Line : CSVLines) {
                            Sink->Write(std::vector<char>(Line.begin(), Line.end()));
                            Sink->Write({'\n'});
                        }
                        std::string Output = "Path saved to <results>/" + FileName + "\n";
                        DOutSink->Write(std::vector<char>(Output.begin(), Output.end()));
                        DOutSink->Write({'>'});
                        DOutSink->Write({' '});
                    } else {
                        std::string Error = "Failed to save path, see help.\n";
                        DErrSink->Write(std::vector<char>(Error.begin(), Error.end()));
                    }
                } else {
                    std::string Error = "No valid path to save, see help.\n";
                    DErrSink->Write(std::vector<char>(Error.begin(), Error.end()));
                }
            } else if (Command == "print") {
                if (!DLastPath.empty()) {
                    std::vector<std::string> Description;
                    if (DPlanner->GetPathDescription(DLastPath, Description)) {
                        for (const auto& Desc : Description) {
                            std::string Output = Desc + '\n';
                            DOutSink->Write(std::vector<char>(Output.begin(), Output.end()));
                        }
                        DOutSink->Write({'>'});
                        DOutSink->Write({' '});
                    } else {
                        std::string Error = "Failed to print path, see help.\n";
                        DErrSink->Write(std::vector<char>(Error.begin(), Error.end()));
                    }
                } else {
                    std::string Error = "No valid path to print, see help.\n";
                    DErrSink->Write(std::vector<char>(Error.begin(), Error.end()));
                }
            } else {
                std::string Error = "Unknown command \"" + Command + "\" type help for help.\n";
                DErrSink->Write(std::vector<char>(Error.begin(), Error.end()));
            }
        }


        return true;
    }
};

// Constructor
CTransportationPlannerCommandLine::CTransportationPlannerCommandLine(std::shared_ptr<CDataSource> cmdsrc, std::shared_ptr<CDataSink> outsink, std::shared_ptr<CDataSink> errsink, std::shared_ptr<CDataFactory> results, std::shared_ptr<CTransportationPlanner> planner)
    : DImplementation(std::make_unique<SImplementation>(cmdsrc, outsink, errsink, results, planner)){}
// Destructor
CTransportationPlannerCommandLine::~CTransportationPlannerCommandLine()=default;

bool CTransportationPlannerCommandLine::ProcessCommands() {
    return DImplementation->ProcessCommands();
}
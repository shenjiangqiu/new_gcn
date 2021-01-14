#include <fstream>
#include <graph.h>
#include <sstream>

#include <iostream>
void Graph::parse(const std::string& graph_name) {
    fmt::print("parse graph:{}\n", graph_name);
    std::string full_graph_name = graph_name + ".graph";
    std::ifstream graph_in(full_graph_name);
    if (!graph_in.is_open()) {
        throw std::runtime_error("cannot open the file");
    }
    int node = 0;
    edge_index.push_back(0);
    while (true) {
        std::string line;
        getline(graph_in, line);
        if (graph_in.eof())
            break;
        edges.push_back(node);
        node++;
        edge_index.push_back(edge_index.back() + 1);
        std::stringstream ss(line);
        while (true) {
            int neighbor;
            ss >> neighbor;
            if (ss.fail())
                break;
            edges.push_back(neighbor);
            edge_index.back() += 1;
        }
    }
}

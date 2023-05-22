#include "AST.h"
#include <sstream>
#include <fstream>
void Node::getGraphVizOutput(Node* node){
    if (node == nullptr) {
        return;
    }
    for (auto child : node->childrenList) {
        getGraphVizOutput(child);
    }
    if (!node->childrenList.empty())
    {
        graphVizRelation += std::to_string(node->currentNodeNumber) + " [label=\"" +
                            node->getNodeName() + "\"];\n";
        for (auto child : node->childrenList) {
            graphVizRelation += std::to_string(child->currentNodeNumber) + " [label=\"" +
                    child->getNodeName() + "\"];\n";
            graphVizRelation += std::to_string(node->currentNodeNumber) + " -- " + std::to_string(child->currentNodeNumber) + ";\n";
        }
    }
}
void Node::generateGraphVizOutput(Node* root)
{
    getGraphVizOutput(root);
    std::string content;
    content += "graph AST{";
    content += graphVizRelation;
    content += "}\n";
    std::ofstream outputFile;
    outputFile.open("AST.dot", std::ios::out);
    if (!outputFile.is_open())
    {
        std::cout<<"Can not open AST.dot file";
    }

    outputFile << content;
    outputFile.close();
}

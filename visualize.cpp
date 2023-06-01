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




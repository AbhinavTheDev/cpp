// main.cpp
#include <SFML/Graphics.hpp>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <thread>
#include <cmath> // Added for mathematical functions

class Node {
public:
    sf::CircleShape shape;
    std::vector<int> neighbors;
    sf::Vector2f position;
    int id;
    
    Node(float x, float y, int nodeId) : position(x, y), id(nodeId) {
        shape.setRadius(20.f);
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::White);
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Black);
    }
};

class Edge {
public:
    sf::RectangleShape line;
    
    Edge(const sf::Vector2f& start, const sf::Vector2f& end) {
        float length = std::sqrt(
            std::pow(end.x - start.x, 2.0f) + 
            std::pow(end.y - start.y, 2.0f)
        );
        float angle = std::atan2(end.y - start.y, end.x - start.x);
        
        line.setSize(sf::Vector2f(length, 2.f));
        line.setPosition(start);
        line.setRotation(angle * 180 / 3.14159f);
        line.setFillColor(sf::Color::Black);
    }
};

class GraphVisualizer {
private:
    sf::RenderWindow window;
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    bool isCreatingEdge;
    int selectedNodeId;
    bool isDragging;
    int draggedNodeId;
    
    enum class TraversalType {
        NONE,
        BFS,
        DFS
    };
    
    TraversalType currentTraversal;

public:
    GraphVisualizer() : 
        window(sf::VideoMode(800, 600), "Graph Traversal Visualizer"),
        isCreatingEdge(false),
        selectedNodeId(-1),
        isDragging(false),
        draggedNodeId(-1),
        currentTraversal(TraversalType::NONE) {
        window.setFramerateLimit(60);
    }
    
    void run() {
        while (window.isOpen()) {
            handleEvents();
            render();
        }
    }
    
    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
                
            if (event.type == sf::Event::MouseButtonPressed) {
                handleMousePress(event);
            }
            
            if (event.type == sf::Event::MouseButtonReleased) {
                handleMouseRelease(event);
            }
            
            if (event.type == sf::Event::MouseMoved) {
                handleMouseMove(event);
            }
            
            if (event.type == sf::Event::KeyPressed) {
                handleKeyPress(event);
            }
        }
    }
    
    void handleMousePress(const sf::Event& event) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        
        if (event.mouseButton.button == sf::Mouse::Left) {
            int clickedNode = getNodeAtPosition(mousePos);
            
            if (clickedNode == -1) {
                // Create new node
                nodes.emplace_back(
                    mousePos.x - 20.f,
                    mousePos.y - 20.f,
                    nodes.size()
                );
            } else {
                // Start creating edge or dragging
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                    isCreatingEdge = true;
                    selectedNodeId = clickedNode;
                } else {
                    isDragging = true;
                    draggedNodeId = clickedNode;
                }
            }
        }
    }
    
    void handleMouseRelease(const sf::Event& event) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (isCreatingEdge) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                int targetNode = getNodeAtPosition(mousePos);
                
                if (targetNode != -1 && targetNode != selectedNodeId) {
                    // Add edge
                    nodes[selectedNodeId].neighbors.push_back(targetNode);
                    nodes[targetNode].neighbors.push_back(selectedNodeId);
                    updateEdges();
                }
                
                isCreatingEdge = false;
                selectedNodeId = -1;
            }
            
            isDragging = false;
            draggedNodeId = -1;
        }
    }
    
    void handleMouseMove(const sf::Event& event) {
        if (isDragging && draggedNodeId != -1) {
            nodes[draggedNodeId].position = sf::Vector2f(
                event.mouseMove.x - 20.f,
                event.mouseMove.y - 20.f
            );
            nodes[draggedNodeId].shape.setPosition(nodes[draggedNodeId].position);
            updateEdges();
        }
    }
    
    void handleKeyPress(const sf::Event& event) {
        if (event.key.code == sf::Keyboard::B) {
            currentTraversal = TraversalType::BFS;
            if (!nodes.empty()) {
                runBFS(0);
            }
        }
        else if (event.key.code == sf::Keyboard::D) {
            currentTraversal = TraversalType::DFS;
            if (!nodes.empty()) {
                std::unordered_set<int> visited;
                runDFS(0, visited);
            }
        }
    }
    
    int getNodeAtPosition(const sf::Vector2i& mousePos) {
        for (int i = 0; i < nodes.size(); i++) {
            if (nodes[i].shape.getGlobalBounds().contains(
                    mousePos.x, mousePos.y)) {
                return i;
            }
        }
        return -1;
    }
    
    void updateEdges() {
        edges.clear();
        for (const auto& node : nodes) {
            for (int neighborId : node.neighbors) {
                sf::Vector2f start = node.position + sf::Vector2f(20.f, 20.f);
                sf::Vector2f end = nodes[neighborId].position + 
                                 sf::Vector2f(20.f, 20.f);
                edges.emplace_back(start, end);
            }
        }
    }
    
    void runBFS(int startNode) {
        std::queue<int> q;
        std::unordered_set<int> visited;
        
        q.push(startNode);
        visited.insert(startNode);
        
        while (!q.empty()) {
            int current = q.front();
            q.pop();
            
            nodes[current].shape.setFillColor(sf::Color::Green);
            render();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            for (int neighbor : nodes[current].neighbors) {
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    q.push(neighbor);
                    nodes[neighbor].shape.setFillColor(sf::Color::Yellow);
                    render();
                }
            }
        }
    }
    
    void runDFS(int startNode, std::unordered_set<int>& visited) {
        visited.insert(startNode);
        nodes[startNode].shape.setFillColor(sf::Color::Red);
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        for (int neighbor : nodes[startNode].neighbors) {
            if (visited.find(neighbor) == visited.end()) {
                runDFS(neighbor, visited);
            }
        }
    }
    
    void render() {
        window.clear(sf::Color::White);
        
        for (const auto& edge : edges) {
            window.draw(edge.line);
        }
        
        for (const auto& node : nodes) {
            window.draw(node.shape);
        }
        
        window.display();
    }
};

int main() {
    GraphVisualizer visualizer;
    visualizer.run();
    return 0;
}
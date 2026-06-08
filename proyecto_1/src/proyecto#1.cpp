#include "engine2D.h"
#include <iostream>
#include<algorithm>
#include<sstream>
#include<fstream>

enum Figure{
    line = 0,
    triangle = 1,
    rectangle = 2,
    ellipse = 3,
    curve = 4,
    selection = 5,
    circle = 6,
    square = 7
};

struct Coord{
	int x, y;
};

class Shape{
public:
    std::vector<Coord> points;
    Figure type;
    Color edge;
    Color inside;
    bool filled;
    unsigned zIndex;

    bool selected = false;

    std::string to_string(){
        std::ostringstream oss;
        
        oss<<type<<' ';
        oss<<edge.r<<' '<<edge.g<<' '<<edge.b<<' ';
        oss<<inside.r<<' '<<inside.g<<' '<<inside.b<<' ';
        oss<<filled;

        for (Coord point : points) 
            oss<<' '<<point.x<<' '<<point.y;
        
        return oss.str();
    }

    Coord getMidPoint(){
        Coord midPoint = {0,0};
        for (Coord point : points){
            midPoint.x += point.x;
            midPoint.y += point.y;
        }

        midPoint.x /= points.size();
        midPoint.y /= points.size();

        return midPoint;
    }

    int getVertice(Coord p){
        int dx, dy;

        for (int i = 0; i < points.size(); ++i){
            dx = std::abs(p.x - points[i].x);
            dy = std::abs(p.y - points[i].y);

            if (dx < 10 && dy < 10) return i;
        }
        
        Coord midPoint = getMidPoint();
        dx = std::abs(p.x - midPoint.x);
        dy = std::abs(p.y - midPoint.y);

        if (dx < 10 && dy < 10) return points.size();

        return -1;
    }

    void modifyShape(int vertice, Coord p, bool ctrl){
        points[vertice] = p;
        
        switch(type){
            case ellipse:
            case rectangle:
                if (!ctrl && type == rectangle) break;
                if (vertice != 0){
                    points[0].x = points[2].x;
                    points[0].y = points[1].y;
                }
                if (vertice != 1){
                    points[1].x = points[3].x;
                    points[1].y = points[0].y;
                }
                if (vertice != 2){
                    points[2].x = points[0].x;
                    points[2].y = points[3].y;
                }
                if (vertice != 3){
                    points[3].x = points[1].x;
                    points[3].y = points[2].y; 
                }
                break;
            case circle:
            case square:
                if (!ctrl && type == square) break;

                switch(vertice){
                    case 0:
                        points[0].y = points[1].y;
                        points[2].x = points[0].x;
                        points[2].y = points[0].y - std::abs(points[0].x - points[1].x);
                        points[3].y = points[2].y;
                        break;
                    case 1:
                        points[1].y = points[0].y;
                        points[3].x = points[1].x;
                        points[3].y = points[1].y - std::abs(points[1].x - points[0].x);
                        points[2].y = points[3].y;
                        break;
                    case 2:
                        points[2].y = points[3].y;
                        points[0].x = points[2].x;
                        points[0].y = points[2].y + std::abs(points[2].x - points[3].x);
                        points[1].y = points[0].y;
                        break;
                    case 3:
                        points[3].y = points[2].y;
                        points[1].x = points[3].x;
                        points[1].y = points[3].y + std::abs(points[3].x - points[2].x);
                        points[0].y = points[1].y;
                        break;
                }
        }
    }

    void moveShape(Coord p){
        Coord middle = getMidPoint();

        int dx = p.x - middle.x;
        int dy = p.y - middle.y;

        for (Coord &points : this->points){
            points.x += dx;
            points.y += dy;
        }
    }

    void elevateDegree() {
        if (points.empty() || type != curve) {
            return;
        }

        int n = points.size() - 1;
        std::vector<Coord> Q(n + 2);

        Q[0] = points[0];
        Q[n + 1] = points[n];

        for (int i = 1; i <= n; ++i) {
            double ratio = static_cast<double>(i) / (n + 1);

            double newX = ratio * points[i - 1].x + (1.0 - ratio) * points[i].x;
            double newY = ratio * points[i - 1].y + (1.0 - ratio) * points[i].y;

            // Round to nearest int to preserve the curve's shape as best as possible
            Q[i].x = static_cast<int>(std::round(newX));
            Q[i].y = static_cast<int>(std::round(newY));
        }

        points = Q;
    }
};

struct quadNode{
    std::vector<int> shapes;
    quadNode* children = nullptr;
    Coord topLeft, bottomRight;

    bool isLeaf(){
        return children == nullptr;
    }
};

class proyecto1;

class quadTree{
private:
    quadNode root;
    int maxDepth = 8;
    int maxShapes = 5;

    void split(quadNode &n, std::vector<Shape> &shapes, int depth){
        n.children = new quadNode[4];

        n.children[0].topLeft.x = n.topLeft.x;
        n.children[0].topLeft.y = n.topLeft.y;
        n.children[0].bottomRight.x = (n.topLeft.x + n.bottomRight.x)/2;
        n.children[0].bottomRight.y = (n.topLeft.y + n.bottomRight.y)/2;//Arriba a la izquierda

        n.children[1].topLeft.x = (n.topLeft.x + n.bottomRight.x)/2;
        n.children[1].topLeft.y = n.topLeft.y;
        n.children[1].bottomRight.x = n.bottomRight.x;
        n.children[1].bottomRight.y = (n.topLeft.y + n.bottomRight.y)/2;//Arriba a la derecha

        n.children[2].topLeft.x = n.topLeft.x;
        n.children[2].topLeft.y = (n.topLeft.y + n.bottomRight.y)/2;
        n.children[2].bottomRight.x = (n.topLeft.x + n.bottomRight.x)/2;
        n.children[2].bottomRight.y = n.bottomRight.y;//Abajo a la izquierda

        n.children[3].topLeft.x = (n.topLeft.x + n.bottomRight.x)/2;
        n.children[3].topLeft.y = (n.topLeft.y + n.bottomRight.y)/2;
        n.children[3].bottomRight.x = n.bottomRight.x;
        n.children[3].bottomRight.y = n.bottomRight.y;//Abajo a la derecha

        for (int i : n.shapes) add(i, shapes, n, depth);

        std::vector<int>().swap(n.shapes);
    }

    bool lineInZone(Coord a, Coord b, Coord topLeft, Coord bottomRight){
        int dx = a.x - b.x;
        int dy = a.y - b.y;//b->a

        if (dx == 0) return a.x > topLeft.x && a.x <= bottomRight.x;
        if (dy == 0) return a.y > topLeft.y && a.y <= bottomRight.y;

        float txMin, txMax, tyMin, tyMax, tEntry, tExit;

        //Se despeja la ecuación paramétrica en los extremos
        txMin = static_cast<float>(topLeft.x - b.x)/dx;
        txMax = static_cast<float>(bottomRight.x - b.x)/dx;
        tyMin = static_cast<float>(topLeft.y - b.y)/dy;
        tyMax = static_cast<float>(bottomRight.y - b.y)/dy;

        if (txMin > txMax) std::swap(txMin, txMax);
        if (tyMin > tyMax) std::swap(tyMin, tyMax);

        tEntry = std::max(txMin, tyMin);
        tExit = std::min(txMax, tyMax);

        return tEntry <= tExit && tEntry <=1 && tExit >= 0;
    }

    bool triangleEdgeInZone(Coord a, Coord b, Coord c, Coord topLeft, Coord bottomRight){
        return lineInZone(a, b, topLeft, bottomRight)
            || lineInZone(b, c, topLeft, bottomRight)
            || lineInZone(c, a, topLeft, bottomRight);
    }

    bool filledtriangleInZone(Coord a, Coord b, Coord c, Coord topLeft, Coord bottomRight){
        Coord edges[3] = {
            {b.x - a.x, b.y - a.y},
            {c.x - b.x, c.y - b.y},
            {a.x - c.x, a.y - c.y}
        };

        for (int i = 0; i < 3; ++i) {
            Coord normal = {-edges[i].y, edges[i].x};

            int pT1 = (a.x * normal.x) + (a.y * normal.y);
            int pT2 = (b.x * normal.x) + (b.y * normal.y);
            int pT3 = (c.x * normal.x) + (c.y * normal.y);

            int triMin, triMax;
            triMin = std::min({pT1, pT2, pT3});
            triMax = std::max({pT1, pT2, pT3});

            int pC1 = (topLeft.x * normal.x) + (topLeft.y * normal.y);
            int pC2 = (bottomRight.x * normal.x) + (topLeft.y * normal.y);
            int pC3 = (topLeft.x * normal.x) + (bottomRight.y * normal.y);
            int pC4 = (bottomRight.x * normal.x) + (bottomRight.y * normal.y);

            int boxMin = std::min({pC1, pC2, pC3, pC4});
            int boxMax = std::max({pC1, pC2, pC3, pC4});

            if (triMax < boxMin || triMin > boxMax) {
                return false;
            }
        }

        return true;
    }

    bool rectangleEdgeInZone(Coord a, Coord b, Coord c, Coord d, Coord topLeft, Coord bottomRight){
        return lineInZone(a, b, topLeft, bottomRight)
            || lineInZone(b, d, topLeft, bottomRight)
            || lineInZone(d, c, topLeft, bottomRight)
            || lineInZone(c, a, topLeft, bottomRight);
    }

    bool filledRectangleInZone(Coord a, Coord b, Coord c, Coord d, Coord topLeft, Coord bottomRight){
        return filledtriangleInZone(a, b, c, topLeft, bottomRight)
            || filledtriangleInZone(c, b, d, topLeft, bottomRight);
    }

    bool filledEllipseInZone(Coord a, Coord b, Coord topLeft, Coord bottomRight){
        Coord center;

        int width = std::abs((a.x - b.x) / 2);
        int height = std::abs((a.y - b.y) / 2);

        center.x = (a.x + b.x) / 2;
        center.y = (a.y + b.y) / 2;

        float minX_prime = static_cast<float>(topLeft.x - center.x) / width;
        float maxX_prime = static_cast<float>(bottomRight.x - center.x) / width;
        float minY_prime = static_cast<float>(topLeft.y - center.y) / height;
        float maxY_prime = static_cast<float>(bottomRight.y - center.y) / height;

        float closestX = std::clamp(0.0f, minX_prime, maxX_prime);
        float closestY = std::clamp(0.0f, minY_prime, maxY_prime);

        float distanceSquared = (closestX * closestX) + (closestY * closestY);
        
        return distanceSquared <= 1.0f;
    }

    bool ellipseEdgeInZone(Coord a, Coord b, Coord topLeft, Coord bottomRight) {
        float width = std::abs((a.x - b.x) / 2.0f);
        float height = std::abs((a.y - b.y) / 2.0f);
        
        float centerX = (a.x + b.x) / 2.0f;
        float centerY = (a.y + b.y) / 2.0f;

        float minX_prime = (topLeft.x - centerX) / width;
        float maxX_prime = (bottomRight.x - centerX) / width;
        float minY_prime = (topLeft.y - centerY) / height;
        float maxY_prime = (bottomRight.y - centerY) / height;

        float closestX = std::clamp(0.0f, minX_prime, maxX_prime);
        float closestY = std::clamp(0.0f, minY_prime, maxY_prime);
        float closestDistSq = (closestX * closestX) + (closestY * closestY);

        if (closestDistSq > 1.0f) {
            return false; 
        }

        float maxDistX = std::max(std::abs(minX_prime), std::abs(maxX_prime));
        float maxDistY = std::max(std::abs(minY_prime), std::abs(maxY_prime));
        float furthestDistSq = (maxDistX * maxDistX) + (maxDistY * maxDistY);

        if (furthestDistSq < 1.0f) {
            return false;
        }

        return true;
    }

    Coord lerp(Coord a, Coord b, float t) {
        return { static_cast<int>(a.x + (b.x - a.x) * t), static_cast<int>(a.y + (b.y - a.y) * t)};
    }

    void splitBezierN(const std::vector<Coord>& curve, std::vector<Coord>& left, std::vector<Coord>& right) {
        size_t n = curve.size();
        if (n == 0) return;

        left.resize(n);
        right.resize(n);

        std::vector<Coord> temp = curve;

        left[0] = temp[0];
        right[n - 1] = temp[n - 1];

        for (size_t i = 1; i < n; ++i) {
            for (size_t j = 0; j < n - i; ++j) {
                temp[j] = lerp(temp[j], temp[j + 1], 0.5f);
            }
            left[i] = temp[0];
            right[n - 1 - i] = temp[n - 1 - i];
        }
    }

    bool curveInZone(const std::vector<Coord>& curve, Coord topLeft, Coord bottomRight, int depth = 0) {
        if (curve.empty()) return false;

        // A. Calculate the AABB of all n control points
        int minX = curve[0].x, maxX = curve[0].x;
        int minY = curve[0].y, maxY = curve[0].y;
        
        for (size_t i = 1; i < curve.size(); ++i) {
            minX = std::min(minX, curve[i].x);
            maxX = std::max(maxX, curve[i].x);
            minY = std::min(minY, curve[i].y);
            maxY = std::max(maxY, curve[i].y);
        }

        if (minX > bottomRight.x || maxX < topLeft.x || minY > bottomRight.y || maxY < topLeft.y) {
            return false; 
        }

        const int MAX_DEPTH = 6;
        if (depth >= MAX_DEPTH) {
            return true; 
        }

        std::vector<Coord> leftCurve;
        std::vector<Coord> rightCurve;

        leftCurve.reserve(curve.size());
        rightCurve.reserve(curve.size());

        splitBezierN(curve, leftCurve, rightCurve);

        return curveInZone(leftCurve, topLeft, bottomRight, depth + 1)
            || curveInZone(rightCurve, topLeft, bottomRight, depth + 1);
    }

    bool insideZone(Shape &shape, quadNode &n){
        int maxX, minX, maxY, minY;

        maxX = shape.points[0].x;
        minX = maxX;
        maxY = shape.points[0].y;
        minY = maxY;
        
        for (const Coord &point : shape.points){
            minX = std::min(minX, point.x);
            maxX = std::max(maxX, point.x);
            minY = std::min(minY, point.y);
            maxY = std::max(maxY, point.y);
        }

        //Bounding box para evitar cálculos innecesarios :p
        if (maxX <= n.topLeft.x || minX > n.bottomRight.x ||  maxY <= n.topLeft.y || minY > n.bottomRight.y)
            return false;
        
        switch (shape.type){
            case line:
                return lineInZone(shape.points[0], shape.points[1], n.topLeft, n.bottomRight);
            case triangle:
                if (!shape.filled){
                    return triangleEdgeInZone(shape.points[0], shape.points[1], shape.points[2], n.topLeft, n.bottomRight);
                }

                return filledtriangleInZone(shape.points[0], shape.points[1], shape.points[2], n.topLeft, n.bottomRight);
            case square:
            case rectangle:
                if (!shape.filled){
                    return rectangleEdgeInZone(shape.points[0], shape.points[1], shape.points[2], shape.points[3], n.topLeft, n.bottomRight);
                }

                return filledRectangleInZone(shape.points[0], shape.points[1], shape.points[2], shape.points[3], n.topLeft, n.bottomRight);
            case ellipse:
            case circle:
                if (!shape.filled){
                    return ellipseEdgeInZone(shape.points[0], shape.points[3], n.topLeft, n.bottomRight);
                }   

                return filledEllipseInZone(shape.points[0], shape.points[3], n.topLeft, n.bottomRight);
            case curve:
                return curveInZone(shape.points, n.topLeft, n.bottomRight);
        }

        return true;
    }

    bool insideZone(Coord mouse, quadNode &n){
        return mouse.x > n.topLeft.x && mouse.y > n.topLeft.y
            && mouse.x <= n.bottomRight.x && mouse.y <= n.bottomRight.y;
    }

    void add(int i, std::vector<Shape> &shapes, quadNode &n, int depth){
        if (n.isLeaf()){
            n.shapes.push_back(i);

            std::cout<<i<<std::endl;
            
            if (n.shapes.size() >= maxShapes && depth + 1 < maxDepth)
                split(n, shapes, depth);
                
            return;
        }
        
        for (int j = 0; j < 4; ++j){
            if (insideZone(shapes[i], n.children[j]))
                add(i, shapes, n.children[j], depth + 1);           
        }  
    }

    int getIndexInNode(Coord mouse, std::vector<Shape> &shapes, quadNode &n){
        Coord topLeft = {mouse.x - 2, mouse.y - 2};
        Coord bottomRight = {mouse.x + 2, mouse.y + 2};

        for (int i = 0; i < n.shapes.size(); ++i){
            int j = n.shapes[i];

            switch (shapes[j].type){
                case line:
                    if (lineInZone(shapes[j].points[0], shapes[j].points[1], topLeft, bottomRight)) return j;
                    break;
                case triangle:
                    if (!shapes[j].filled){
                        if (triangleEdgeInZone(shapes[j].points[0], shapes[j].points[1], shapes[j].points[2], topLeft, bottomRight)) return j;
                        
                        break;
                    }

                    if (filledtriangleInZone(shapes[j].points[0], shapes[j].points[1], shapes[j].points[2], topLeft, bottomRight)) return j;
                    
                    break;
                case square:
                case rectangle:
                    if (!shapes[j].filled){
                        if (rectangleEdgeInZone(shapes[j].points[0], shapes[j].points[1], shapes[j].points[2], shapes[j].points[3], topLeft, bottomRight)) return j;

                        break;
                    }

                    if (filledRectangleInZone(shapes[j].points[0], shapes[j].points[1], shapes[j].points[2], shapes[j].points[3], topLeft, bottomRight)) return j;

                    break;
                case ellipse:
                case circle:
                    if (!shapes[j].filled){
                        if (ellipseEdgeInZone(shapes[j].points[0], shapes[j].points[3], topLeft, bottomRight)) return j;

                        break;
                    }   

                    if (filledEllipseInZone(shapes[j].points[0], shapes[j].points[3], topLeft, bottomRight)) return j;

                    break;
                case curve:
                    if (curveInZone(shapes[j].points, topLeft, bottomRight)) return j;

                    break;
                }
        }
        return -1;
    }

    int getIndex(Coord mouse, std::vector<Shape> &shapes, quadNode &n){
        if (n.isLeaf()){
            return getIndexInNode(mouse, shapes, n);
        }

        for (int j = 0; j < 4; ++j){
            if (insideZone(mouse, n.children[j])){
                int i = getIndex(mouse, shapes, n.children[j]);

                if (i != -1) return i;
            }
        }

        return -1;
    }

    void delQuadNode(quadNode &n){
        if (n.isLeaf()){
            std::vector<int>().swap(n.shapes);
            return;
        }

        for (int i = 0; i < 4; ++i) delQuadNode(n.children[i]);

        delete[] n.children;
        n.children = nullptr;
    }

    void draw(proyecto1 *p1, quadNode &n);
public:
    quadTree(int width, int height){
        root.topLeft = {0,0};
        root.bottomRight = {width, height};
    }
    quadTree(){
    }

    ~quadTree(){
        delQuadNode(root);
    }

    void clear(){
        delQuadNode(root);
    }

    void add(std::vector<Shape> &shapes, int i = 0){
        add(i, shapes, root, 0);
    }

    void addLast(std::vector<Shape> &shapes){
        add(shapes.size() - 1, shapes, root, 0);
    }

    int getIndex(Coord mouse, std::vector<Shape> &shapes){
        return getIndex(mouse, shapes, root);
    }

    void draw(proyecto1 *p1){
        draw(p1, root);
    }

    void remake(std::vector<Shape> &shapes){
        clear();

        for (int i = 0; i < shapes.size(); ++i) add(shapes, i);
    }
};

class proyecto1 : public Engine2D {
private:
    Color colorFondo = Color(0.1f, 0.1f, 0.15f);
    Color colorBorde = Color(1.0f, 0.0f, 0.0f);
    bool dibujando = false;

    //Mío
    Figure mode = line;
    Coord p1, p2;
    std::vector<Shape> shapes;
    bool ctrl = false;
    bool incompleteShape = false;
    Color colorRelleno = Color(0.0f, 0.0f, 1.0f);
    
    quadTree qTree;
    bool drawTree = false;

    int currentShape = -1;
    int currentVertice = -1;
    
    bool fill = false;

    unsigned zIndex = 1;

    ImGuiIO& io = ImGui::GetIO();

    //Para copiar, cortar y pegar
    Shape clipboard;

    //Para ctrl+z y ctrl+y
    int savedStates = 1;
    int currentState = 1;
    int statesBehind = 0;

    const char* items[6] = {"Linea", "Triangulo", "Rectangulo", "Elipse", "Curva", "Seleccion"};
public:
    //Originalmente 1024x600
    proyecto1(): Engine2D(1280, 720, "Proyecto #1 - Gestion y Despliegue de Primitivas") {
        qTree = quadTree(width, height);
    }
    void setup() override {
        clear(colorFondo);
        saveState(".state0");
        std::cout << "Motor inicializado exitosamente." << std::endl;
    }
    // Eventos
    void onkeyDown(int key) override {
        if (ImGui::IsAnyItemActive()) return;

        if (key == GLFW_KEY_SPACE) {
            shapes.clear();
            qTree.clear();
            incompleteShape = false;
            dibujando = false;

            currentShape = -1;

            saveState(".state" + std::to_string(currentState));
            currentState = (currentState + 1) % 5;
        }
        if (key == GLFW_KEY_LEFT_CONTROL) ctrl = true;
        if (key == GLFW_KEY_Q) drawTree = !drawTree;
        if (key == GLFW_KEY_R) fill = !fill;

        if (key == GLFW_KEY_DELETE && currentShape != -1){
            shapes.erase(shapes.begin() + currentShape);

            qTree.remake(shapes);

            savedStates += savedStates < 5 ? 1 : 0;

            saveState(".state" + std::to_string(currentState));

            currentState = (currentState + 1) % 5;
            return;
        }

        if (key > GLFW_KEY_0 && key < GLFW_KEY_7){
            mode = static_cast<Figure>(key - 49);
            if (incompleteShape){
                incompleteShape = false;  
                qTree.addLast(shapes);
            }
            
            if (currentShape != -1) shapes[currentShape].selected = false;
            currentShape = -1;
            currentVertice = -1;
            return;
        }

        if (ctrl){
            if (key == GLFW_KEY_Z){
                std::cout<<"ctrl+z"<<std::endl;
                if (savedStates <= 1) return;

                currentState -= 2;
                if (currentState < 0) currentState = 5 + currentState;

                if (!loadState(".state" + std::to_string(currentState))){
                    currentState = (currentState + 2) % 5;
                    return;
                }
                
                currentState = (currentState + 1) % 5;
                --savedStates;
                ++statesBehind;
            }else if (key == GLFW_KEY_Y){
                std::cout<<"ctrl+y"<<std::endl;
                if (statesBehind <= 0) return;

                --statesBehind;

                if (!loadState(".state" + std::to_string(currentState))){
                    ++statesBehind;
                    if (currentState < 0) currentState = 4;
                }

                currentState = (currentState + 1) % 5;

                ++savedStates;
            }
            else if (key == GLFW_KEY_C && currentShape != -1){
                std::cout<<"ctrl+c"<<std::endl;
                clipboard = shapes[currentShape];
                clipboard.selected = false;
            }else if (key == GLFW_KEY_X && currentShape != -1){
                std::cout<<"ctrl+x"<<std::endl;
                clipboard = shapes[currentShape];
                clipboard.selected = false;

                shapes.erase(shapes.begin() + currentShape);

                qTree.remake(shapes);

                savedStates += savedStates < 5 ? 1 : 0;

                saveState(".state" + std::to_string(currentState));

                currentState = (currentState + 1) % 5;
            }else if (key == GLFW_KEY_V && clipboard.points.size() != 0){
                std::cout<<"ctrl+v"<<std::endl;

                ImVec2 mouse = ImGui::GetMousePos();

                clipboard.moveShape({static_cast<int>(mouse.x), static_cast<int>(mouse.y)});

                clipboard.zIndex = zIndex++;

                shapes.push_back(clipboard);

                qTree.addLast(shapes);

                savedStates += savedStates < 5 ? 1 : 0;

                saveState(".state" + std::to_string(currentState));

                currentState = (currentState + 1) % 5;
            }
        }

        if (currentShape != -1){
            if (key == GLFW_KEY_W){
                for (Coord &point : shapes[currentShape].points)
                    point.y -= 5;
                
                qTree.remake(shapes);

                savedStates += savedStates < 5 ? 1 : 0;

                saveState(".state" + std::to_string(currentState));

                currentState = (currentState + 1) % 5;
            }
            else if (key == GLFW_KEY_A){
                for (Coord &point : shapes[currentShape].points)
                    point.x -= 5;

                qTree.remake(shapes);

                savedStates += savedStates < 5 ? 1 : 0;

                saveState(".state" + std::to_string(currentState));

                currentState = (currentState + 1) % 5;
            }
            else if (key == GLFW_KEY_S){
                for (Coord &point : shapes[currentShape].points)
                    point.y += 5;

                qTree.remake(shapes);

                savedStates += savedStates < 5 ? 1 : 0;

                saveState(".state" + std::to_string(currentState));

                currentState = (currentState + 1) % 5;
            }
            else if (key == GLFW_KEY_D){
                for (Coord &point : shapes[currentShape].points)
                    point.x += 5;

                qTree.remake(shapes);

                savedStates += savedStates < 5 ? 1 : 0;

                saveState(".state" + std::to_string(currentState));

                currentState = (currentState + 1) % 5;
            }
        }
    }
    void onkeyUp(int key) override {
        if (key == GLFW_KEY_LEFT_CONTROL){
            ctrl = false;
        }
    }
    void onMouseButtonDown(int button, double x, double y) override {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (!io.WantCaptureMouse) dibujando = true;
            else return;
        }else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (mode == curve && incompleteShape) {
                incompleteShape = false;
                qTree.addLast(shapes);
                savedStates += savedStates < 5 ? 1 : 0;

                saveState(".state" + std::to_string(currentState));
                
                currentState = (currentState + 1) % 5;
            }
            return;
        }else return;
        

        p1 = {static_cast<int>(x), static_cast<int>(y)};
        switch (mode){
            case triangle:
                if (incompleteShape){
                    shapes.back().points[2] = p1;
                    incompleteShape = false;
                }else{
                    shapes.push_back({{p1, p1, p1}, triangle, colorBorde, colorRelleno, fill, zIndex});
                    incompleteShape = true;
                }
                break;
            case rectangle:
                if (ctrl){
                    shapes.push_back({{p1, p1, p1, p1}, square, colorBorde, colorRelleno, fill, zIndex});
                    break;
                }
                shapes.push_back({{p1, p1, p1, p1}, mode, colorBorde, colorRelleno, fill, zIndex});
                break;
            case ellipse:
                if (ctrl){
                    shapes.push_back({{p1, p1, p1, p1}, circle, colorBorde, colorRelleno, fill, zIndex});
                    break;
                }
                shapes.push_back({{p1, p1, p1, p1}, mode, colorBorde, colorRelleno, fill, zIndex});
                break;
            case curve:
                if (incompleteShape){
                    shapes.back().points.push_back(p1);
                }else{
                    shapes.push_back({{p1, p1}, curve, colorBorde, colorRelleno, false, zIndex});
                    incompleteShape = true;
                }
                break;
            case selection:{
                if (currentShape != -1){
                    currentVertice = shapes[currentShape].getVertice(p1);
                    if (currentVertice != -1) return;
                }

                int newShape = qTree.getIndex(p1, shapes);

                if (currentShape != -1 && newShape != currentShape){
                    shapes[currentShape].selected = false;
                }

                currentShape = newShape;

                if (currentShape == -1){
                    currentVertice = -1;
                    return;
                }

                shapes[currentShape].selected = true;

                shapes[currentShape].zIndex = zIndex;

                break;
            }
            case line:
                shapes.push_back({{p1, p1}, mode, colorBorde, colorRelleno, false, zIndex});
            default:
                shapes.push_back({{p1, p1}, mode, colorBorde, colorRelleno, fill, zIndex});
                break;
        }

        ++zIndex;
    }
    void onMouseButtonUp(int button, double x, double y) override {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            dibujando = false;

            if (shapes.size() == 0) return;

            if (incompleteShape) return;
            if (mode == selection && currentVertice == -1) return;

            savedStates += savedStates < 5 ? 1 : 0;

            saveState(".state" + std::to_string(currentState));

            currentState = (currentState + 1) % 5;
            
            if (mode == selection){
                qTree.remake(shapes);

                return;
            }

            qTree.addLast(shapes);
        }      
    }
    // Evento de movimiento continuo
    void onMouseMove(double x, double y) override {
        if (!dibujando) return;
        
        p2 = {static_cast<int>(x), static_cast<int>(y)};

        switch (mode){
            case rectangle:
            case ellipse:
                if (shapes.back().type != mode){
                    int dir = p2.y > p1.y ? 1 : -1;
                    shapes.back().points[1].x = p2.x;
                    shapes.back().points[2].y = p1.y + std::abs(shapes.back().points[0].x - p2.x)*dir;
                    shapes.back().points[3] = {p2.x, shapes.back().points[2].y};
                }
                else{
                    shapes.back().points[1].x = p2.x;
                    shapes.back().points[2].y = p2.y;
                    shapes.back().points[3] = p2;
                }
                break;
            case selection:
                if (currentVertice == -1) return;

                if (currentVertice == shapes[currentShape].points.size()){
                    shapes[currentShape].moveShape(p2);

                    break;
                }

                shapes[currentShape].modifyShape(currentVertice, p2, ctrl);
                break;
            case triangle:{
                int i = incompleteShape ? 1 : 2;
                shapes.back().points[i] = p2;
                break;
                }
            case curve:
                shapes.back().points.back() = p2;
                break;
            default:
                shapes.back().points[1] = p2;
                break;
        }
    }
    void update(float deltaTime) override {
        clear(colorFondo);

        std::vector<Shape> copy(shapes);

        std::sort(copy.begin(), copy.end(), [] (Shape &a, Shape &b) -> bool{
            return a.zIndex < b.zIndex;
        });

        for (Shape &shape : copy){
            switch (shape.type){
                case line:
                    drawLine(shape.points[0], shape.points[1], shape.edge);
                    break;
                case triangle:
                    drawTriangle(shape.points[0], shape.points[1], shape.points[2], shape.edge, shape.inside, shape.filled);
                    break;
                case square:
                case rectangle:
                    drawRectangle(shape.points[0], shape.points[1], shape.points[2], shape.points[3], shape.edge, shape.inside, shape.filled);
                    break;
                case ellipse:
                    drawEllipse(shape.points[0], shape.points[3], shape.edge, shape.inside, shape.filled);
                    break;
                case curve:
                    drawCurve(shape.points, shape.edge);
                    break;
                case circle:
                    drawCircle(shape.points[0], shape.points[3], shape.edge, shape.inside, shape.filled);
                    break;
            }

            if (shape.selected) drawControlPoints(shape);
        }

        if (drawTree) qTree.draw(this);
    }
    void drawUI() override {
        ImGui::Begin("Herramientas");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        ImVec2 mousePos = ImGui::GetMousePos();

        // Display the coordinates
        ImGui::Text("Mouse Position: (%.1f, %.1f)", mousePos.x, mousePos.y);

        ImGui::Separator();

        static char str[128];
        ImGui::InputText("##Nombre archivo", str, 128);
        if (ImGui::Button("Guardar estado")) saveState(std::string(str));
        ImGui::SameLine();
        if (ImGui::Button("Cargar estado")) loadState(std::string(str));

        ImGui::Separator();
        float colb[3] = { colorBorde.r, colorBorde.g, colorBorde.b };
        float colr[3] = {colorRelleno.r, colorRelleno.g, colorRelleno.b};
        float colf[3] = {colorFondo.r, colorFondo.g, colorFondo.b};
        
        if (ImGui::ColorEdit3("Color Borde", colb)) {
            colorBorde.r = colb[0];
            colorBorde.g = colb[1];
            colorBorde.b = colb[2];
        }
        if (ImGui::ColorEdit3("Color Relleno", colr)){
            colorRelleno.r = colr[0];
            colorRelleno.g = colr[1];
            colorRelleno.b = colr[2];
        }
        if (ImGui::ColorEdit3("Color Fondo", colf)) {
            colorFondo.r = colf[0];
            colorFondo.g = colf[1];
            colorFondo.b = colf[2];
        }

        ImGui::Separator();

        for (int i = 0; i < 6; ++i){
            if (i == mode){
                ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(0.6f, 0.6f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.6f, 0.55f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(0.6f, 0.5f, 0.9f));

                if (ImGui::Button(items[i])) {
                    mode = static_cast<Figure>(i);

                    if (incompleteShape){
                        qTree.addLast(shapes);
                        incompleteShape = false;
                    }

                    if (currentShape != -1) shapes[currentShape].selected = false;
                    currentShape = -1;
                    currentVertice = -1;
                }

                ImGui::PopStyleColor(3);
            }else{
                if (ImGui::Button(items[i])){
                    mode = static_cast<Figure>(i);

                    if (incompleteShape){
                        qTree.addLast(shapes);
                        incompleteShape = false;
                    }

                    if (currentShape != -1) shapes[currentShape].selected = false;
                    currentShape = -1;
                    currentVertice = -1;
                }
            }

            if (i < 5) ImGui::SameLine();
        }

        ImGui::Checkbox("Relleno", &fill);

        if (currentShape != -1){
            ImGui::Separator();
            Shape &shape = shapes[currentShape];
            float bordeFig[3] = {shape.edge.r, shape.edge.g, shape.edge.b};
            float rellenoFig[3] = {shape.inside.r, shape.inside.g, shape.inside.b};

            ImGui::Text("Figura seleccionada");

            if (shape.type != curve && shape.type != line)
                ImGui::Checkbox("Rellenar", &shape.filled);

            if (shape.type == curve){
                if (ImGui::Button("Aumentar grado")) shape.elevateDegree();
                ImGui::SameLine();
                ImGui::Text("Grado actual: %d", shape.points.size());
            }

            if (currentVertice != -1 && currentVertice != shape.points.size()){
                Coord vertice = shape.points[currentVertice];
                ImGui::Text("Vertice seleccionado");
                ImGui::InputInt("X", &vertice.x);
                ImGui::InputInt("Y", &vertice.y);

                shape.modifyShape(currentVertice, vertice, ctrl);
            }

            ImGui::InputScalar("zIndex",ImGuiDataType_U32, &shape.zIndex);
            ImGui::Text("Mayor zIndex: %u", zIndex-1);

            if (ImGui::ColorEdit3("Borde", bordeFig)){
                shape.edge.r = bordeFig[0];
                shape.edge.g = bordeFig[1];
                shape.edge.b = bordeFig[2];
            }
            if (shape.filled){
                if (ImGui::ColorEdit3("Relleno", rellenoFig)){
                    shape.inside.r = rellenoFig[0];
                    shape.inside.g = rellenoFig[1];
                    shape.inside.b = rellenoFig[2];
                }
            }
        }

        ImGui::Separator();
        
        if(ctrl) ImGui::Text("Ctrl");
        ImGui::End();
    }

    //Funciones mías :p
    void drawLine(Coord a, Coord b, Color color){
        int x, y, incy, inc2;
        bool inverted = false, negative = false;

        if (a.x == b.x){
            x = a.x;
            if (a.y > b.y) std::swap(a,b);

            for (y = a.y; y <= b.y; ++y) putPixel(x, y, color);
            return;
        }
        if (a.y == b.y){
            y = a.y;
            if (a.x > b.x) std::swap(a,b);

            for (x = a.x; x <= b.x; ++x) putPixel(x, y, color);
            return;
        }

        float m = static_cast<float>(a.y-b.y)/(a.x-b.x);

        if (std::abs(m) > 1.0f){
            inverted = true;
            std::swap(a.x, a.y);
            std::swap(b.x, b.y);
        }

        if (a.x > b.x) std::swap(a, b);

        int dx = b.x - a.x;
        int dy = b.y - a.y;

        if (a.y > b.y){
            incy = -1;
            dx = -dx;
            negative = true;
        }else{
            incy = 1;
        }

        int d = dx - 2*dy;
        x = a.x;
        y = a.y;

        if (inverted)
            putPixel(y, x, color);
        else
            putPixel(x, y, color);

        for (; x <= b.x; ++x){
            if (d > 0 != negative){ //XOR
                d -= dy;
            }else{
                d += -dy + dx;
                y += incy;
            }

            if (inverted)
                putPixel(y, x, color);
            else
                putPixel(x, y, color);
        }
    }

    void drawTriangle(Coord a, Coord b, Coord c, Color edge, Color inside, bool filled){
        //Triángulo de 2 puntos
        if (a.x == c.x && a.y == c.y){
            drawLine(a, b, edge);
            return;
        }
        
        if (!filled){
            drawLine(a, b, edge);
            drawLine(b, c, edge);
            drawLine(a, c, edge);
            return;
        }

        if (a.y > b.y) std::swap(a,b);
        if (a.y > c.y) std::swap(a,c);
        if (b.y > c.y) std::swap(b,c);

        std::vector<Coord> ab, bc, ac;

        getLine(a, b, ab);
        getLine(b, c, bc);
        getLine(a, c, ac);

        if (ab.size() == 0 || bc.size() == 0 || ac.size() == 0) return;

        if (ab.front().y > ab.back().y) std::reverse(ab.begin(), ab.end());
        if (bc.front().y > bc.back().y) std::reverse(bc.begin(), bc.end());
        if (ac.front().y > ac.back().y) std::reverse(ac.begin(), ac.end());

        int i = 0, j = 0, k = 0;

        if (a.y != b.y) {
            while (j < ab.size() && i < ac.size()) {
                if (ab[j].y == ac[i].y) {
                    drawLine(ab[j], ac[i], inside);
                    ++i; ++j;
                }else if (ab[j].y < ac[i].y) ++j;
                else ++i;
            }
        }

        if (b.y != c.y) {
            while (k < bc.size() && i < ac.size()) {
                if (bc[k].y == ac[i].y) {
                    drawLine(bc[k], ac[i], inside);
                    ++i; ++k;
                }else if (bc[k].y < ac[i].y) ++k;
                else ++i;
            }
        }
        
        drawLine(a, b, edge);
        drawLine(b, c, edge);
        drawLine(a, c, edge);
    }

    //c  d
    //a  b
    void drawRectangle(Coord a, Coord b, Coord c, Coord d, Color edge, Color inside, bool filled){
        if (a.y == d.y || a.x == d.x){
            drawLine(a, d, edge);
            return;
        }

        if (filled){
            drawTriangle(a, b, c, inside, inside, true);
            drawTriangle(c, b, d, inside, inside, true);
        }

        drawLine(a, b, edge);
        drawLine(b, d, edge);
        drawLine(d, c, edge);
        drawLine(c, a, edge);
    }

    void drawEllipse(Coord a, Coord b, Color edge, Color inside, bool filled){
        int x = 0, y, width, height;
        Coord center;
        bool inverted = false;

        if (a.x > b.x) std::swap(a, b);

        width = (b.x - a.x)/2;
        height = std::abs(b.y - a.y)/2;

        if (width < height){
            inverted = true;
            std::swap(width, height);
        }

        y = height;

        int d = 4*height*height - width*width*(4*height - 1);

        width *= width;
        height *= height;

        center.x = (b.x+a.x)/2;
        center.y = (b.y+a.y)/2;

        if (inverted) std::swap(x, y);
        if (filled){
            drawLine({center.x - x, center.y - y}, {center.x - x, center.y + y}, inside);
            drawLine({center.x + x, center.y - y}, {center.x + x, center.y + y}, inside);
        }

        putPixel(center.x + x, center.y + y, edge);
        putPixel(center.x - x, center.y + y, edge);
        putPixel(center.x - x, center.y - y, edge);
        putPixel(center.x + x, center.y - y, edge);

        if (inverted) std::swap(x, y);

        int se = height*3 + 2*width;
        int e = height*3;

        while (2*height*x < 2*width*y){
            if (d > 0){
                d += 4*(height*2*x - width*2*y + se);
                --y;
            }else{
                d += 4*(height*2*x + e);
            }

            if (inverted) std::swap(x, y);

            if (filled){
                if (!inverted) {
                    drawLine({center.x - x, center.y - y}, {center.x - x, center.y + y}, inside);
                    drawLine({center.x + x, center.y - y}, {center.x + x, center.y + y}, inside);
                } else {
                    drawLine({center.x - x, center.y - y}, {center.x + x, center.y - y}, inside);
                    drawLine({center.x - x, center.y + y}, {center.x + x, center.y + y}, inside);
                }
            }

            putPixel(center.x + x, center.y + y, edge);
            putPixel(center.x - x, center.y + y, edge);
            putPixel(center.x - x, center.y - y, edge);
            putPixel(center.x + x, center.y - y, edge);      
            
            if (inverted) std::swap(x, y);

            ++x;
        }

        int s = 3*width;
        se = 2*height + 3*width;

        while(y >= 0){
            if (d > 0){
                d += s - 2*width*y;
            }else{
                d += se + 2*height*x - 2*width*y;
                ++x;
            }

            if (inverted) std::swap(x, y);

            if (filled){
                if (!inverted) {
                    drawLine({center.x - x, center.y - y}, {center.x + x, center.y - y}, inside);
                    drawLine({center.x - x, center.y + y}, {center.x + x, center.y + y}, inside);
                } else {
                    drawLine({center.x - x, center.y - y}, {center.x - x, center.y + y}, inside);
                    drawLine({center.x + x, center.y - y}, {center.x + x, center.y + y}, inside);
                }
            }

            putPixel(center.x + x, center.y + y, edge);
            putPixel(center.x - x, center.y + y, edge);
            putPixel(center.x - x, center.y - y, edge);
            putPixel(center.x + x, center.y - y, edge);    
            
            if (inverted) std::swap(x, y);

            --y;
        }
    }

    void drawCircle(Coord a, Coord b, Color edge, Color inside, bool filled) {
        if (a.x > b.x) std::swap(a, b);

        int radius = (b.x - a.x) / 2;
        Coord center;
        center.x = (b.x + a.x) / 2;
        center.y = (b.y + a.y) / 2;

        if (filled) {
            int x = 0;
            int y = radius;
            int d = 1 - y;

            while (x <= y) {
                drawLine({center.x - x, center.y + y}, {center.x - x, center.y - y}, inside);
                drawLine({center.x + x, center.y + y}, {center.x + x, center.y - y}, inside);
                drawLine({center.x - y, center.y - x}, {center.x - y, center.y + x}, inside);
                drawLine({center.x + y, center.y - x}, {center.x + y, center.y + x}, inside);

                if (d >= 0) {
                    d += 2 * (x - y) + 5;
                    --y;
                } else {
                    d += 2 * x + 3;
                }
                ++x;
            }
        }

        int x = 0;
        int y = radius;
        int d = 1 - y;

        while (x <= y) {
            putPixel(center.x + x, center.y + y, edge);
            putPixel(center.x + x, center.y - y, edge);
            putPixel(center.x - x, center.y - y, edge);
            putPixel(center.x - x, center.y + y, edge);
            putPixel(center.x + y, center.y + x, edge);
            putPixel(center.x + y, center.y - x, edge);
            putPixel(center.x - y, center.y - x, edge);
            putPixel(center.x - y, center.y + x, edge);

            if (d >= 0) {
                d += 2 * (x - y) + 5;
                --y;
            } else {
                d += 2 * x + 3;
            }
            ++x;
        }
    }
    //Casteljau
    void drawCurve(std::vector<Coord> &points, Color color){
        if (points.size() == 1){
            putPixel(points[0].x, points[0].y, color);
            return;
        }   
        
        int n = points.size();
        std::vector<std::pair<float,float>> end;
        std::pair<float,float> bi[n];

        for (float t = 0.0f; t < 1.0f; t += 0.001f){
            float minusT = 1.0f - t;

            for (int i = 0; i < n; ++i){
                bi[i].first = static_cast<float>(points[i].x);
                bi[i].second = static_cast<float>(points[i].y);
            }

            for (int step = 1; step < n; ++step){
                for (int i = 0; i < n - step; ++i){
                    bi[i].first  = minusT*bi[i].first + t*bi[i+1].first;
                    bi[i].second = minusT*bi[i].second + t*bi[i+1].second;
                }
            }

            end.push_back(bi[0]);
        }

        for (int i = 0; i < end.size() - 1; ++i)
            drawLine({static_cast<int>(end[i].first), static_cast<int>(end[i].second)}, {static_cast<int>(end[i + 1].first), static_cast<int>(end[i + 1].second)}, color);
    }

    void getLine(Coord a, Coord b, std::vector<Coord> &coords){
        int x, y, incy, inc2;
        bool inverted = false, negative = false;

        if (a.x == b.x){
            x = a.x;
            if (a.y > b.y) std::swap(a,b);

            for (y = a.y; y <= b.y; ++y) coords.push_back({x, y});
            return;
        }
        if (a.y == b.y){
            y = a.y;
            if (a.x > b.x) std::swap(a,b);

            for (x = a.x; x <= b.x; ++x) coords.push_back({x, y});
            return;
        }

        float m = static_cast<float>(a.y-b.y)/(a.x-b.x);

        if (std::abs(m) > 1.0f){
            inverted = true;
            std::swap(a.x, a.y);
            std::swap(b.x, b.y);
        }

        if (a.x > b.x) std::swap(a, b);

        int dx = b.x - a.x;
        int dy = b.y - a.y;

        if (a.y > b.y){
            incy = -1;
            dx = -dx;
            negative = true;
        }else{
            incy = 1;
        }

        int d = dx - 2*dy;
        x = a.x;
        y = a.y;

        if (inverted)
            coords.push_back({y, x});
        else
            coords.push_back({x, y});

        for (; x <= b.x; ++x){
            if (d > 0 != negative){ //XOR
                d -= dy;
            }else{
                d += -dy + dx;
                y += incy;
            }

            if (inverted)
                coords.push_back({y, x});
            else
                coords.push_back({x, y});
        }

        return;
    }

    void saveState(std::string fileName){
        std::ofstream file(fileName);

        if (!file.is_open()){
            std::cout<<"Error guardando a "<<fileName<<std::endl;
            return;
        }

        for(Shape &shape: shapes)
            file<<shape.to_string()<<std::endl;

        file.close();

        statesBehind = 0; //Si guardas un nuevo estado estás al día

        std::cout<<"Guardado a "<<fileName<<std::endl;
    }

    bool loadState(std::string fileName){
        std::ifstream file(fileName);
        Shape temp;
        int in;

        if (!file.is_open()){
            std::cout<<"Error cargando de "<<fileName<<std::endl;
            return false;
        }

        shapes.clear();
        qTree.clear();
        incompleteShape = false;
        dibujando = false;
        currentShape = -1;
        currentVertice = -1;

        std::string line;

        while (std::getline(file, line)) {
            std::stringstream ss(line);

            ss>>in;
            temp.type = static_cast<Figure>(in);

            ss>>temp.edge.r>>temp.edge.g>>temp.edge.b;
            ss>>temp.inside.r>>temp.inside.g>>temp.inside.b;
            ss>>temp.filled;

            int x, y;
            while (ss >> x >> y) {
                temp.points.push_back({x, y});
            }

            shapes.push_back(temp);
            temp.points.clear();
            qTree.addLast(shapes);
        }

        file.close();

        std::cout<<"Cargado de "<<fileName<<std::endl;

        return true;
    }

    void drawControlPoints(Shape &shape){
        if (shape.type == curve){
            for (int i = 0; i < shape.points.size() - 1; ++i)
                drawLine(shape.points[i], shape.points[i+1], {1.0f, 1.0f, 1.0f});
        }
        for (Coord point : shape.points){
            drawCircle({point.x-3, point.y-3}, {point.x+3, point.y+3}, {0.0f,0.0f,0.0f}, {0.97f, 0.88f, 0.39f}, true);
        }

        Coord midPoint = shape.getMidPoint();

        drawCircle({midPoint.x-3, midPoint.y-3}, {midPoint.x+3, midPoint.y+3}, {0.0f,0.0f,0.0f}, {0.45f, 0.45f, 0.45f}, true);
    }
};

void quadTree::draw(proyecto1* p1, quadNode &n){
    p1->drawRectangle(n.topLeft, {n.bottomRight.x-1, n.topLeft.y}, {n.topLeft.x, n.bottomRight.y-1}, {n.bottomRight.x-1, n.bottomRight.y-1},
                      {255, 255, 255}, {0,0,0}, false);

    if (n.isLeaf()) return;

    for (int i = 0; i < 4; ++i) draw(p1, n.children[i]);
}

int main() {
    proyecto1 app;
    app.run();

    for (int i = 0; i < 5; ++i) std::remove((".state" + std::to_string(i)).c_str());
    return 0;
}
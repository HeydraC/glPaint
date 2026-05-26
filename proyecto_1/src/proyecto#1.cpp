#include "engine2D.h"
#include <iostream>
#include<algorithm>
#include<sstream>
#include<fstream>

struct Coord{
	int x, y;
};

enum Figure{
    line = 0,
    triangle = 1,
    rectangle = 2,
    elipse = 3,
    curve = 4,
    circle = 5,
    square = 6
};

struct Shape{
    std::vector<Coord> points;
    Figure type;
    Color edge;
    Color inside;
    bool filled;

    std::string to_string(){
        std::ostringstream oss;
        
        oss<<type<<' ';
        oss<<edge.r<<' '<<edge.g<<' '<<edge.b<<' ';
        oss<<inside.r<<' '<<inside.g<<' '<<inside.b<<' ';
        oss<<filled;

        for (auto point : points) 
            oss<<' '<<point.x<<' '<<point.y;
        
        return oss.str();
    }
};

// struct Node{
//     Shape* shapes;
//     Node* children[4];
//     int x0, x1, y0, y1;
//     int i;

//     Node(){
//         i = 0;
//         shapes = new Shape[5];

//         children[0] = nullptr;
//     }

//     void add(Shape s){
//         shapes[i] = s;
//         ++i;
//     }
// };

// class proyecto1;

// class quadTree{
// private:
//     Node* root;

//     void addShape(Shape s, Node* n){
//         if (s.p1.x < n->x0 || s.p1.x > n->x1 || s.p1.y < n->y0 || s.p1.y > n->y1) return;

//         if (n->children[0] == nullptr){ //Es una hoja
//             n->add(s);
//             evaluate(n);
//             return;
//         }

//         for (int i = 0; i < 4; ++i) addShape(s, n->children[i]);
//     }

//     //Si tiene 5 elementos se subdivide
//     void evaluate(Node* n){
//         if (n->i < 5) return;

//         for (int i = 0; i < 4; ++i) n->children[i] = new Node();

//         n->children[0]->x0 = n->x0;
//         n->children[0]->x1 = (n->x0 + n->x1)/2;
//         n->children[0]->y0 = n->y0;
//         n->children[0]->y1 = (n->y0 + n->y1)/2;//Arriba a la izquierda

//         n->children[1]->x0 = (n->x0 + n->x1)/2;
//         n->children[1]->x1 = n->x1;
//         n->children[1]->y0 = n->y0;
//         n->children[1]->y1 = (n->y0 + n->y1)/2;//Arriba a la derecha

//         n->children[2]->x0 = n->x0;
//         n->children[2]->x1 = (n->x0 + n->x1)/2;
//         n->children[2]->y0 = (n->y0 + n->y1)/2;
//         n->children[2]->y1 = n->y1;//Abajo a la izquierda

//         n->children[3]->x0 = (n->x0 + n->x1)/2;
//         n->children[3]->x1 = n->x1;
//         n->children[3]->y0 = (n->y0 + n->y1)/2;
//         n->children[3]->y1 = n->y1;//Abajo a la derecha

//         for (int i = 0; i < 5; ++i) addShape(n->shapes[i], n);
        
//         delete[] n->shapes;

//         n->shapes = nullptr;
//     }

//     void delNode(Node* n){
//         if (n->children[0] == nullptr){
//             delete[] n->shapes;
//             delete n;
//             return;
//         }

//         for (int i = 0; i < 4; ++i) delNode(n->children[i]);

//         delete n;
//     }

//     void draw(proyecto1* p1, Node* n);
// public:
//     quadTree(int width, int height){
//         root = new Node();
        
//         root->x0 = 0;
//         root->x1 = width;
//         root->y0 = 0;
//         root->y1 = height;
//     }
//     quadTree(){
//     }

//     ~quadTree(){
//         delNode(root);
//     }

//     void addShape(Shape s){
//         addShape(s, root);
//     }

//     void clear(){
//         int width, height;

//         width = root->x1;
//         height = root->y1;

//         delNode(root);

//         root = new Node();
        
//         root->x0 = 0;
//         root->x1 = width;
//         root->y0 = 0;
//         root->y1 = height;
//     }

//     void draw(proyecto1* p1);
// };

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
    bool incompleteFigure = false;
    Color colorRelleno = Color(0.0f, 0.0f, 1.0f);
    //quadTree qTree;
    //bool drawTree = false;
    
    bool fill = false;

    const char* items[5] = {"Linea", "Triangulo", "Rectangulo", "Elipse", "Curva"};
public:
    //Originalmente 1024x600
    proyecto1(): Engine2D(1280, 720, "Proyecto #1 - Gestion y Despliegue de Primitivas") {
        //qTree = quadTree(width, height);
    }
    void setup() override {
        clear(colorFondo);
        std::cout << "Motor inicializado exitosamente." << std::endl;
    }
    // Eventos
    void onkeyDown(int key) override {
        if (key == GLFW_KEY_SPACE) {
            shapes.clear();
            //qTree.clear();
            incompleteFigure = false;
            dibujando = false;
        }
        if (key == GLFW_KEY_LEFT_CONTROL) ctrl = true;
        if (key == GLFW_KEY_Q){
            //drawTree = !drawTree;
        }
        if (key == GLFW_KEY_R) fill = !fill;

        if (key > GLFW_KEY_0 && key < GLFW_KEY_6){
            mode = static_cast<Figure>(key - 49);

            if (incompleteFigure) incompleteFigure = false;
        }
    }
    void onkeyUp(int key) override {
        if (key == GLFW_KEY_LEFT_CONTROL){
            ctrl = false;
        }
    }
    void onMouseButtonDown(int button, double x, double y) override {
        static ImGuiIO& io = ImGui::GetIO();

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (!io.WantCaptureMouse) dibujando = true;
            else return;
        }else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (mode == curve) incompleteFigure = false;
            return;
        }else{
            return;
        }

        p1 = {static_cast<int>(x), static_cast<int>(y)};
        switch (mode){
            case triangle:
                if (incompleteFigure){
                    shapes.back().points[2] = p1;
                    incompleteFigure = false;
                    dibujando = false;
                }else{
                    shapes.push_back({{p1, p1, p1}, triangle, colorBorde, colorRelleno, fill});
                    incompleteFigure = true;
                }
                break;
            case rectangle:
                if (ctrl){
                    shapes.push_back({{p1, p1, p1, p1}, square, colorBorde, colorRelleno, fill});
                    break;
                }
                shapes.push_back({{p1, p1, p1, p1}, mode, colorBorde, colorRelleno, fill});
                break;
            case elipse:
                if (ctrl){
                    shapes.push_back({{p1, p1}, circle, colorBorde, colorRelleno, fill});
                    break;
                }
                shapes.push_back({{p1, p1}, mode, colorBorde, colorRelleno, fill});
                break;
            case curve:
                if (incompleteFigure){
                    shapes.back().points.push_back(p1);
                }else{
                    shapes.push_back({{p1}, curve, colorBorde, colorRelleno, fill});
                    incompleteFigure = true;
                }
                dibujando = false;
                break;
            default:
                shapes.push_back({{p1, p1}, mode, colorBorde, colorRelleno, fill});
                break;
        }
    }
    void onMouseButtonUp(int button, double x, double y) override {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            dibujando = false;
        }

        //qTree.addShape(shapes.back());
    }
    // Evento de movimiento continuo
    void onMouseMove(double x, double y) override {
        if (!dibujando) return;
        
        p2 = {static_cast<int>(x), static_cast<int>(y)};

        switch (mode){
            case rectangle:
                if (shapes.back().type == square){
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
            default:
                shapes.back().points[1] = p2;
                break;
        }
    }
    void update(float deltaTime) override {
        clear(colorFondo);

        //if (drawTree) qTree.draw(this);

        for (auto shape : shapes){
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
                case elipse:
                    drawElipse(shape.points[0], shape.points[1], shape.edge, shape.inside, shape.filled);
                    break;
                case curve:
                    drawCurve(shape.points, shape.edge);
                    break;
                case circle:
                    drawCircle(shape.points[0], shape.points[1], shape.edge, shape.inside, shape.filled);
                    break;
            }
        }
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

        for (int i = 0; i < 5; ++i){
            if (i == mode){
                ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(0.6f, 0.6f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.6f, 0.55f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(0.6f, 0.5f, 0.9f));

                if (ImGui::Button(items[i])) {
                    mode = static_cast<Figure>(i);

                    incompleteFigure = false;
                }

                ImGui::PopStyleColor(3);
            }else{
                if (ImGui::Button(items[i])){
                    mode = static_cast<Figure>(i);

                    incompleteFigure = false;
                }
            }

            if (i != 4) ImGui::SameLine();
        }

        ImGui::Checkbox("Relleno", &fill);

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

        float m = float(a.y-b.y)/float(a.x-b.x);

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

    void drawElipse(Coord a, Coord b, Color edge, Color inside, bool filled){
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
                drawLine({center.x - x, center.y - y}, {center.x - x, center.y + y}, inside);
                drawLine({center.x + x, center.y - y}, {center.x + x, center.y + y}, inside);
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
                drawLine({center.x - x, center.y - y}, {center.x + x, center.y - y}, inside);
                drawLine({center.x - x, center.y + y}, {center.x + x, center.y + y}, inside);
            }

            putPixel(center.x + x, center.y + y, edge);
            putPixel(center.x - x, center.y + y, edge);
            putPixel(center.x - x, center.y - y, edge);
            putPixel(center.x + x, center.y - y, edge);    
            
            if (inverted) std::swap(x, y);

            --y;
        }
    }

    void drawCircle(Coord a, Coord b, Color edge, Color inside, bool filled){
        int x = 0, y, d;
        Coord center;

        if (a.x > b.x) std::swap(a, b);

        y = (b.x-a.x)/2;
        center.x = (b.x+a.x)/2;
        center.y = (b.y+a.y)/2;

        if (filled){
            drawLine({center.x - x, center.y + y}, {center.x + x, center.y + y}, inside);
            drawLine({center.x - x, center.y - y}, {center.x + x, center.y - y}, inside);
            drawLine({center.x - y, center.y + x}, {center.x + y, center.y + x}, inside);
            drawLine({center.x - y, center.y - x}, {center.x + y, center.y - x}, inside);
        }

        putPixel(center.x + x, center.y + y, edge);
        putPixel(center.x + x, center.y - y, edge);
        putPixel(center.x - x, center.y - y, edge);
        putPixel(center.x - x, center.y + y, edge);
        putPixel(center.x + y, center.y + x, edge);
        putPixel(center.x + y, center.y - x, edge);
        putPixel(center.x - y, center.y - x, edge);
        putPixel(center.x - y, center.y + x, edge);

        d = 1 - y;
        ++x;

        while(x <= y){
            if (d >= 0){
                d += 2*(x - y)+ 5;
                --y;
            }else{
                d += 2*x + 3;
            }

            if (filled){
                drawLine({center.x - x, center.y + y}, {center.x + x, center.y + y}, inside);
                drawLine({center.x - x, center.y - y}, {center.x + x, center.y - y}, inside);
                drawLine({center.x - y, center.y + x}, {center.x + y, center.y + x}, inside);
                drawLine({center.x - y, center.y - x}, {center.x + y, center.y - x}, inside);
            }

            putPixel(center.x + x, center.y + y, edge);
            putPixel(center.x + x, center.y - y, edge);
            putPixel(center.x - x, center.y - y, edge);
            putPixel(center.x - x, center.y + y, edge);
            putPixel(center.x + y, center.y + x, edge);
            putPixel(center.x + y, center.y - x, edge);
            putPixel(center.x - y, center.y - x, edge);
            putPixel(center.x - y, center.y + x, edge);

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

        float m = float(a.y-b.y)/float(a.x-b.x);

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
        
        if (file.is_open()){
            for(auto shape: shapes)
                file<<shape.to_string()<<std::endl;

            file.close();
        }
    }

    void loadState(std::string fileName){
        std::ifstream file(fileName);
        Shape temp;
        int in;

        if (!file.is_open()) return;

        shapes.clear();
        incompleteFigure = false;
        dibujando = false;

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
        }

        file.close();
    }
};


/*
std::string to_string(){
        std::ostringstream oss;
        
        oss<<type<<' ';
        oss<<edge.r<<' '<<edge.g<<' '<<edge.b<<' ';
        oss<<inside.r<<' '<<inside.g<<' '<<inside.b<<' ';
        oss<<filled;

        for (auto point : points) 
            oss<<' '<<point.x<<' '<<point.y;
        
        return oss.str();
    }
*/

// void quadTree::draw(proyecto1* p1, Node* n){
//     p1->drawRectangle({n->x0, n->y1-1}, {n->x1-1, n->y0}, {255, 255, 255});

//     if (n->children[0] == nullptr) return;

//     for (int i = 0; i < 4; ++i) draw(p1, n->children[i]);
// }

// void quadTree::draw(proyecto1* p1){
//     draw(p1, root);
// }

int main() {
    proyecto1 app;
    app.run();
    return 0;
}

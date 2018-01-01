#ifndef QOAED_POINT_OCTREE_HPP
#define QOAED_POINT_OCTREE_HPP 

#include <set>
#include <list>
#include <array>
#include <stack>
#include <queue>
#include <functional>
#include <cmath>

#include <iostream>

#include "Point.hpp"

// Octants are identified in this way
// When z is greater_eq than o.z
//
//      y
//
//      |
//   1  |  0
//      |
//------o------ x
//      |
//   2  |  3
//      |
//
// When z is less than o.z
//
//      y
//
//      |
//   5  |  4
//      |
//------o------ x
//      |
//   6  |  7
//      |
//
// See what_octant for more information


namespace qoaed {

template <class Value, class CoordType = long>
class PointOctree {
public:
  using value_type = Value;
  using reference = Value&;
  using const_reference = Value const&;
  using Point = Point3D<CoordType>;

private: 

  class Node;

  class Node {
  public:
    Point   point;
    Node**   parent = 0;//para mejorar spheric_query, influye en insert
    Node*   childs[8];
    mutable Value val;

    Node(const Point& p, const Value& val) : 
      point(p), val(val) { 
        for (int i = 0; i < 8; ++i)
          childs[i] = 0;
      }

    Node(const CoordType& x, const CoordType& y, const CoordType& z, const Value& val):
      point(x,y,z), val(val) { 
        childs = new Node*[8];
        for (int i = 0; i < 8; ++i)
          childs[i] = 0;
      }
  };

public:
  
  class NodeVisitor {
  friend class PointOctree;

  private:
    Node* n;

  public:
    NodeVisitor(Node* n): n(n) {}
    
    const CoordType& get_x() const { return n->point.x; }
    const CoordType& get_y() const { return n->point.y; }
    const CoordType& get_z() const { return n->point.z; }
    const Point& get_point() const { return n->point; }
    Value& operator*() const { return n->val; }
    operator    bool() const { return (bool) n; }
  };

  class Cube {
  friend class PointOctree;

  private:
    // min is the left bottom front point
    // max is the rigth top back point
    Point min;
    Point max;

  public:
    Cube(const Point& min, const Point& max) : min(min), max(max) {}

    Cube(const CoordType& min_x, const CoordType& min_y, const CoordType& min_z,
         const CoordType& max_x, const CoordType& max_y, const CoordType& max_z) :
      min(min_x, min_y, min_z), max(max_x, max_y, max_z) {}

    bool contains(const Point& p) const {
      bool cx, cy, cz;
      cx = (p.x <= max.x && p.x >= min.x);
      cy = (p.y <= max.y && p.y >= min.y);
      cz = (p.z <= max.z && p.z >= min.z);
      return cx && cy && cz;
    }

    bool contains(const CoordType& x, const CoordType& y, const CoordType& z) const { return contains(Point(x,y,z)); }
  };

  using VisitorFunction = typename std::function<void (const NodeVisitor&)>;

private:
  Node* m_root;

public:
  PointOctree() : m_root(nullptr) {}
 ~PointOctree() = default;

  void insert(const Point& p, const Value& val) {
    Node** tmp = 0;
     Node** parent = 0;
    if (find(p, tmp, parent)) return;   
    Node* n = new Node(p, val);
    n->parent = parent;
    (*tmp) = n;
  }

  void insert(const CoordType& x, const CoordType& y, const CoordType& z, const Value& val) { insert(Point(x,y,z), val); }

  NodeVisitor find(const Point& p) {
    Node** tmp = 0;
    Node** parent = 0;
    if (!find(p, tmp, parent))
      throw std::runtime_error("Point (" + std::to_string(p.x) + ", " + std::to_string(p.y) + ", " + 
          std::to_string(p.z) + ") not found");

    return NodeVisitor(*tmp);
  }

  NodeVisitor find(const CoordType& x, const CoordType& y, const CoordType& z) { return find(Point(x,y,z)); }

  PointOctree ranged_query(const Cube& cube, const VisitorFunction& visitor = [](auto& n){}) {
    PointOctree subtree;
    ranged_query(m_root, cube, subtree, visitor);
    return subtree;
  }
  
   double distance(Point a,Point b)
   {
       cerr << "distancia: "<< sqrt(pow(b.x-a.x,2.0)+pow(b.y-a.y,2.0)+pow(b.z-a.z,2.0)) <<'\n';
       return sqrt(pow(b.x-a.x,2.0)+pow(b.y-a.y,2.0)+pow(b.z-a.z,2.0)); 
   }

    PointOctree spheric_query(Point point_, const double radio, const VisitorFunction& visitor = [](auto& n){})
    {
        cerr << "Iniciando spheric_query...\n";
        Node** node_point;
        Node** node_parent;
        find (point_, node_point, node_parent);
        Node* point = *node_point;
        if(point != nullptr){
            cerr << "point != null\n";
            PointOctree subtree;
            Node *origin = point;
            //selects adecuade point according to radio
            while(origin != m_root && distance(origin->point, (*(origin->parent))->point) <= radio)
            {
                cerr << "cambia de origin\n";
                origin = *(origin->parent);
            }
            if(distance(point->point, origin->point) > radio && origin != m_root)//si punto con el que se va a trabajar y radio no contienen al punto original, que le punto sea el padre
            {
                cerr << "que origin sea el padre\n";
                origin = *(origin->parent);
            }
            //work selecting spheric points
            int arr[8][2]; //used inside spheric_query
            arr[0][0]= 6;
            arr[0][1]= 0;
            arr[1][0]= 7;
            arr[1][1]= 1;
            arr[2][0]= 4;
            arr[2][1]= 2;
            arr[3][0]= 5;
            arr[3][1]= 3;
            arr[4][0]= 2;
            arr[4][1]= 4;
            arr[5][0]= 3;
            arr[5][1]= 5;
            arr[6][0]= 0;
            arr[6][1]= 6;
            arr[7][0]= 1;
            arr[7][1]= 7;
            cerr << "inicia spheric_query verdadero\n";
            spheric_query(origin, origin, radio, subtree, visitor, arr);//origin de nodo central y el otro se usara para trabajar
            return subtree;
        }
        std::cerr<<"No hay punto detectado\n";
    }


  void visit_dfs(const std::function<void (const NodeVisitor&)>& visitor, NodeVisitor start = NodeVisitor(0)) {
    if (!m_root) return;

    std::stack<Node*> cont;
    if (!start)
      start.n = m_root;

    cont.push(start.n);
    Node* tmp;
    while (!cont.empty()) {
      tmp = cont.top();
            cont.pop();

      if (visitor)
        visitor(start.n = tmp);

      for (int ii = 0; ii < 8; ++ii)
        if (tmp->childs[ii])
          cont.push(tmp->childs[ii]);
    }
  }

  void visit_bfs(const std::function<void (const NodeVisitor&)>& visitor, NodeVisitor start = NodeVisitor(0)) {
    if (!m_root) return;

    std::queue<Node*> cont;
    if (!start)
      start.n = m_root;

    cont.push(start.n);
    Node* tmp;
    while (!cont.empty()) {
      tmp = cont.front();
            cont.pop();

      visitor(start.n = tmp);

      for (int ii = 0; ii < 8; ++ii)
        if (tmp->childs[ii])
          cont.push(tmp->childs[ii]);
    }
  }
  


private:

    bool find(const Point& p, Node**& node, Node** &parent) {
        node = std::addressof(m_root);   
        parent = nullptr; 
        while (*node) {
          if ((*node)->point == p) return true;
          parent = node;
          node = std::addressof((*node)->childs[what_octant(p, *node)]);
        }
        return false;
    }
  // Tell me where this coord locates relative to Node orig
  int what_octant(const Point& p, Node* orig) {
    if (p.x >= orig->point.x) {
      if (p.y >= orig->point.y) {
        if (p.z >= orig->point.z)
          return 0;
        else
          return 4;
      } else {
        if (p.z >= orig->point.z)
          return 3;
        else
          return 7;
      }
    } else {
      if (p.y >= orig->point.y) {
        if (p.z >= orig->point.z)
          return 1;
        else 
          return 5;
      } else {
        if (p.z >= orig->point.z)
          return 2;
        else 
          return 6;
      }
    }
  }

  void ranged_query(Node* n, const Cube& cube, PointOctree& subtree, const VisitorFunction& visitor) {
    if (!n) return;

    if (cube.contains(n->point)) {
      subtree.insert(n->point, n->val);
      if (visitor)
        visitor(NodeVisitor(n));
    }

    if (n->point.x >= cube.min.x) {
      if (n->point.y >= cube.min.y) {
        if (n->point.z >= cube.min.z)
          ranged_query(n->childs[6], cube, subtree, visitor);
        if (n->point.z <= cube.max.z)
          ranged_query(n->childs[2], cube, subtree, visitor);
      }

      if (n->point.y <= cube.max.y) {
        if (n->point.z >= cube.min.z)
          ranged_query(n->childs[5], cube, subtree, visitor);
        if (n->point.z <= cube.max.z)
          ranged_query(n->childs[1], cube, subtree, visitor);
      }
    }
    if (n->point.x <= cube.max.x) {
      if (n->point.y >= cube.min.y) {
        if (n->point.z >= cube.min.z)
          ranged_query(n->childs[7], cube, subtree, visitor);
        if (n->point.z <= cube.max.z)
          ranged_query(n->childs[3], cube, subtree, visitor);
      }

      if (n->point.y <= cube.max.y) {
        if (n->point.z >= cube.min.z)
          ranged_query(n->childs[4], cube, subtree, visitor);
        if (n->point.z <= cube.max.z)
          ranged_query(n->childs[0], cube, subtree, visitor);
      }
    }
  }

    void add_branch(PointOctree &subtree, Node* node,const VisitorFunction &visitor)
    {
        cerr << "add_branch\n";
        if(node != nullptr){
            subtree.insert(node->point, node->val);
            if (visitor){
                        visitor(NodeVisitor(node));
            }
            for(int i=0; i!= 8; i++){
                add_branch(subtree,node->childs[i],visitor);
            }
        }
    }
    void spheric_query(const Node *origin, Node *point, const double radio, PointOctree &subtree, const VisitorFunction &visitor, int arr[][2])
    {
        /*
        para octante: (por ejm, si estoy en octante 4)
            si distancia <= r
                agregar punto y su hijo 2
                spheric_query para 3,7,6,  1,0,4,5            
            si no
                descartar punto y su hijo 4
                spheric_query para octantes 2,3,7,6,  1,0,5
        Resumen:
            4: <=r : 2 | >r: 4 
            7: <=r : 1 | >r: 7
            5: <=r : 3 | >r: 5
            6: <=r : 0 | >r: 6
            1: <=r : 7 | >r: 1
            2: <=r : 4 | >r: 2
            0: <=r : 6 | >r: 0
            3: <=r : 5 | >r: 3
        */
        if(point != nullptr){
            cerr << "entro con ese punto\n";
            for(int i=0; i!=8; i++){
                cerr << "   para cada octante, ejm 4\n";
                Node* child = point->childs[i];
                if(child!=nullptr){
                    cerr << "si el hijo es diferente de null\n";
                    if(distance(origin->point, child->point) <= radio)
                    {
                        cerr << "inserta los que choquen con eje\n";
                        subtree.insert(child->point, child->val);
                        if (visitor){
                            cerr << "aplica funcion visitor a child\n";
                            visitor(NodeVisitor(child));
                        }
                        add_branch(subtree, child->childs[arr[i][0]],visitor);
                        int j=0;                
                        while(j!=8){
                            if(j != arr[i][0]){
                                cerr << "para cada subhijo,aceptado child, aplicara spheric_query\n";
                                 spheric_query(origin, child->childs[j],radio, subtree, visitor,arr);
                            }
                            j++;
                        }
                    }
                    else
                    {
                        int j=0; 
                        while(j!=8){
                            if(j != arr[i][1]){
                                cerr << "para cada subhijo, rechazado child, aplica spheric_query\n";
                                 spheric_query(origin, child->childs[j],radio, subtree, visitor,arr);
                            }
                            j++;
                        }
                        
                    }
                }
            }   
        }
    }
    

    
};


}

#endif

/**
 * Created by Cloud on 2020/9/13.
 * The TinyRtree is designed to be a simple, efficient and easy to modify prototype of Rtree.
 * Notice: We keep all the function definition in this .h file as template usage may make
 * some compiler unable to work with a seperate .cpp file.
 * ref: https://stackoverflow.com/questions/115703/storing-c-template-function-definitions-in-a-cpp-file
 */

#ifndef MYRTREE_TINYRTREE_H
#define MYRTREE_TINYRTREE_H

#define RTREE_TEMPLATEE template<int DIMS, int MaxNodes, int MinNodes, typename IDtype, typename Tk, typename Tv>
#define TINYRTREE TinyRtree<DIMS, MaxNodes, MinNodes, IDtype, Tk, Tv>

#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <limits>
#include <cstdarg>

RTREE_TEMPLATEE
class TinyRtree {

public:
    TinyRtree(){
        std::cout << "create tiny rtree" << std::endl;
        m_root = Node();
        m_root.m_level = 0;
    };
    ~TinyRtree(){ std::cout << "destroy tiny rtree" << std::endl; }; // free the memory

    struct Rect{
        Rect() = default;
        Rect(std::initializer_list<Tk> list){ // Rect( {10, 20, 30, 40 }) ;
            int i = 0;
            for (Tk elem : list){
                i < DIMS ? m_min[i] = elem : m_max[i-DIMS] = elem;
                i++;
            }
        }
        Tk m_min[DIMS];
        Tk m_max[DIMS];
    };

    struct Node{
        Node(): m_level(-1), isLeafNode(false), num_children(0), aggregate_max(0), aggregate_count(0) {
            children.reserve(MinNodes);
            auto rect_max = std::numeric_limits<Tk>::max();
            auto rect_min = std::numeric_limits<Tk>::lowest();  // min() is for smallest positive value the type can encode
            std::fill_n(m_rect.m_min, DIMS, rect_max);
            std::fill_n(m_rect.m_max, DIMS, rect_min);
        }

        explicit Node(Rect rect): m_level(-1), isLeafNode(false), num_children(0), aggregate_max(0), aggregate_count(0) {
            this->m_rect = rect;
        }

        Rect m_rect;
        int m_level; // leaf is 0, root is h, where h is the tree height - 1
        double m_volume;

        int num_children;
        std::vector<Node> children;

        bool isLeafNode;
        bool isLeafElement;

        Tv aggregate_max;
        Tv aggregate_count;
    };

    Node m_root;

    // insert
    void Insert(Rect rect, IDtype id);
    bool RecursiveInsert(Rect &rect, IDtype &id, Node &current_node, Node &new_node);
    void SplitNode(Node &current_node, Node &new_node); // return new node

    // helper function
    inline Rect CombineRect(Rect &rect1, Rect &rect2);
    int ChooseChildIndex(Node &current_node, Rect &rect);
    inline double RectVolume(Rect rect);

    // other aggregate support functions
    // should I write it in a inherited class?
};


// = = = = = = = = = = = = = = = Definition = = = = = = = = = = = = = = =


RTREE_TEMPLATEE
void TINYRTREE::Insert(Rect rect, IDtype id){

    Node new_node_next_level;
    bool split = RecursiveInsert(rect, id, m_root, new_node_next_level);
    if(split){
        Node new_root;
        new_root.children.push_back(m_root);
        new_root.children.push_back(new_node_next_level);
        new_root.num_children = 2;
        new_root.m_rect = CombineRect(m_root.m_rect, new_node_next_level.m_rect);
        new_root.m_level = m_root.m_level + 1;
        m_root = new_root; // will this have some bugs?
    }

}

RTREE_TEMPLATEE
bool TINYRTREE::RecursiveInsert(Rect &rect, IDtype &id, Node &current_node, Node &new_node){

    if(current_node.m_level == 0){ // insert here
        // insert here
        Node node(rect);
        node.isLeafElement = true;
        node.m_level = -1;

        if(current_node.num_children < MaxNodes){
            current_node.children.push_back(node);
            current_node.num_children++;
            current_node.m_rect = CombineRect(current_node.m_rect, rect);
            return false;
        } else {
            current_node.children.push_back(node);
            current_node.num_children++;
            SplitNode(current_node, new_node); // insert this newNode back to its parent node
            return true;
        }

    } else { // insert in the next level

        // search for the child node to insert
        int insert_index = ChooseChildIndex(current_node, rect);

        Node new_node_next_level;
        // insert it there
        bool split = RecursiveInsert(rect, id, current_node.children[insert_index], new_node_next_level);
        if(split){
            // insert new node next level to this node
            if(current_node.num_children < MaxNodes){
                current_node.children.push_back(new_node_next_level);
                current_node.num_children++;
                current_node.m_rect = CombineRect(current_node.m_rect, new_node_next_level.m_rect);
                return false;
            } else {
                current_node.children.push_back(new_node_next_level);
                current_node.num_children++;
                SplitNode(current_node, new_node); // insert this newNode back to its parent node
                return true;
            }
        } else {
            return false;
        }
    }
}

RTREE_TEMPLATEE
void TINYRTREE::SplitNode(Node &current_node, Node &new_node){

    // initializing volume
    int index1 = -1, index2 = -1;
    double max_waste_volume, combined_volume, waste_volume, volume1, volume2;
    for(int i = 0; i < current_node.children.size(); i++){
        current_node.children[i].m_volume = RectVolume(current_node.children[i].m_rect);
    }
    // find the two initial nodes
    for(int i = 0; i < current_node.children.size() - 1; i++){
        for(int j = i + 1; j < current_node.children.size(); j++) {
            combined_volume = RectVolume(CombineRect(current_node.children[i].m_rect, current_node.children[j].m_rect));
            volume1 = current_node.children[i].m_volume;
            volume2 = current_node.children[j].m_volume;
            waste_volume = combined_volume - volume1 - volume2;
            if(index1 == -1 && index2 == -1){
                max_waste_volume = waste_volume;
                index1 = i;
                index2 = j;
            } else if (waste_volume > max_waste_volume){
                max_waste_volume = waste_volume;
                index1 = i;
                index2 = j;
            }
        }
    }

    // buffer the nodes
    std::vector<Node> node_buffer(current_node.children);

    // create the initial 2 cluster
    current_node.children.clear();
    current_node.children.push_back(node_buffer[index1]);
    current_node.m_rect = node_buffer[index1].m_rect;
    current_node.m_volume = RectVolume(current_node.m_rect);

    new_node.children.clear();
    new_node.children.push_back(node_buffer[index2]);
    new_node.m_rect = node_buffer[index2].m_rect;
    new_node.m_volume = RectVolume(new_node.m_rect);

    node_buffer.erase(node_buffer.begin() + index2); // index2 is larger than index1
    node_buffer.erase(node_buffer.begin() + index1);

    // now assign the remaining nodes into the 2 cluster
    double max_volume_diff, volume_diff, volume_increase_1, volume_increase_2;
    int picked_index, picked_cluster = 0;
    while(node_buffer.size() > 0){
        // find the node that maximize the difference of volume increase
        picked_index = -1;
        for(int i = 0; i < node_buffer.size(); i++){
            volume_increase_1 = RectVolume(CombineRect(node_buffer[i].m_rect, current_node.m_rect)) - current_node.m_volume;
            volume_increase_2 = RectVolume(CombineRect(node_buffer[i].m_rect, new_node.m_rect)) - new_node.m_volume;
            volume_diff = fabs(volume_increase_1 - volume_increase_2);
            if(picked_index == -1){
                max_volume_diff = volume_diff;
                picked_index = i;
                picked_cluster = volume_increase_1 < volume_increase_2 ? 0 : 1;
            } else if (volume_diff > max_volume_diff){
                max_volume_diff = volume_diff;
                picked_index = i;
                picked_cluster = volume_increase_1 < volume_increase_2 ? 0 : 1;
            }
        }

        // insert the picked node into the corresponding cluster
        if (picked_cluster == 0){
            current_node.children.push_back(node_buffer[picked_index]);
            current_node.m_rect = CombineRect(node_buffer[picked_index].m_rect, current_node.m_rect);
            current_node.m_volume = RectVolume(current_node.m_rect);
        } else {
            new_node.children.push_back(node_buffer[picked_index]);
            new_node.m_rect = CombineRect(node_buffer[picked_index].m_rect, new_node.m_rect);
            new_node.m_volume = RectVolume(new_node.m_rect);
        }

        // delete this node from buffer
        node_buffer.erase(node_buffer.begin() + picked_index);
    }

}

RTREE_TEMPLATEE
inline typename TINYRTREE::Rect TINYRTREE::CombineRect(Rect &rect1, Rect &rect2){
    Rect combined_rect;
    for (int dim = 0; dim < DIMS; dim++){
        combined_rect.m_min[dim] = std::min(rect1.m_min[dim], rect2.m_min[dim]);
        combined_rect.m_max[dim] = std::max(rect1.m_max[dim], rect2.m_max[dim]);
    }
    return combined_rect;
}

RTREE_TEMPLATEE
int TINYRTREE::ChooseChildIndex(Node &current_node, Rect &rect){
    assert(current_node.m_level > 0);
    Rect enlarged_rect;
    double original_volume, enlarged_volume, enlargement, min_enlarge_volume;
    int min_index = -1;
    for (int i = 0; i < current_node.num_children; i++){
        // calculate enlargement for each child
        enlarged_rect = CombineRect(current_node.children[i].m_rect, rect);
        original_volume = RectVolume(current_node.children[i].m_rect);
        enlarged_volume = RectVolume(enlarged_rect);
        enlargement = enlarged_volume - original_volume;
        if(min_index == -1){
            min_enlarge_volume = enlargement;
            min_index = i;
        } else if (enlargement < min_enlarge_volume) {
            min_enlarge_volume = enlargement;
            min_index = i;
        }
    }
    return min_index;
}

RTREE_TEMPLATEE
inline double TINYRTREE::RectVolume(Rect rect){
    double volume = 1;
    for (int dim = 0; dim < DIMS; dim++){
        volume *= (rect.m_max[dim] - rect.m_min[dim]);
    }
    return volume;
}


#endif //MYRTREE_TINYRTREE_H

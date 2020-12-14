#include <iostream>
#include "TinyRtree.h"

using namespace std;

int main() {

    typedef TinyRtree<2, 4, 2, int, double, double> RT;
    RT tiny_rtree;

//    RT::Rect rect;
//    rect.m_min[0] = 1;
//    rect.m_min[1] = 2;
//    rect.m_max[0] = 2;
//    rect.m_max[1] = 4;
//    double vol = tiny_rtree.RectVolume(rect);
//    cout << "volume: " << vol << endl;

    for (int i = 0; i < 5; i++){
        RT::Rect rect({(double)i, (double)i, 2*(double)i, 2*(double)i});
        tiny_rtree.Insert(rect,i);
    }

    return 0;
}

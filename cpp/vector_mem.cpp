#include <iostream>
#include <vector>

using namespace std;

struct delete_ptr { // Helper function to ease cleanup of container
    template <typename P>
    void operator () (P p) {
        delete p;
    }
};

typedef struct _Pair{
    string key;
    string value;
} Pair;

int main()
{
    vector<Pair*> list;
    Pair *p = new Pair;
    p->key = "1";
    p->value = "abc";
    list.push_back(p);

    p = new Pair;
    p->key = "2";
    p->value = "def";
    list.push_back(p);

    vector<Pair*>::iterator it = list.begin();
    for (; it != list.end(); it++) {
        cout<<(*it)->key<<" --- "<<(*it)->value<<endl;
    }

    for (it = list.begin(); it != list.end(); it++) {
        delete *it;
    } 
    list.clear();
    return 0;
}

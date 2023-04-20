#include <unordered_map>
#include <list>
#include <iostream>

using namespace std;

// _key key of the data input
// _val val of the value input
// _type type assosciativty
// _policy replacement policy
template<typename _key, typename _val>
class Cache 
{
    private:
        unordered_map<_key, _val> cache;
        int capacity;
        int policy;
    public:
        //cache size
        Cache(int _cap, int _policy) : capacity(_cap), policy(_policy) {}

        _val get(_key key) 
        {
            if (cache.find(key) != cache.end()) {
                return cache[key];
            }
            return _val();
        }

        void put(_key key, _val value)
        {
            switch (policy)
            {
            case 0:
                cout<<"Policy is LRU";
                break;
            
            default:
                break;
            }
            if (cache.size() == capacity) {
                cache.erase(cache.begin());
            }
            cache[key] = value;
        }
        
        int size() 
        {
            return cache.size();
        }
};

int main()
{
    Cache<int, int> c1(1, 0);
    c1.put(1,2);
}
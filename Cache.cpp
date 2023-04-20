#include <unordered_map>
#include <list>
#include <iostream>

using namespace std;

// _key key of the data input
// _val val of the value input
// _type type assosciativty
// _policy replacement policy
// 0 FIFO Policy
//
template<class _key, typename _val>
class Cache 
{
    private:
        unordered_map<_key, _val> cache;
        int cacheSize;
        int policy;
    public:
        //cache size
        Cache(int _cap, int _policy) : cacheSize(_cap), policy(_policy) {}

        _val get(_key key) 
        {
            if (cache.find(key) != cache.end()) {
                return cache[key];
            }
            return _val();
        }

        void put(_key key, _val value)
        {
            if (cache.size() == cacheSize) 
            {
                switch (policy)
                {
                case 0:
                {
                    cache.erase(cache.begin());
                    cout<<"Policy is FIFO";

                }
                    break;
                case 1:
                {
                    cout<<"Policy is LRU";
                }

                default:
                    break;
                }
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
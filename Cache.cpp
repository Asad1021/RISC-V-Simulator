#include <unordered_map>
#include <list>
#include <iostream>

using namespace std;

// _key key of the data input
// _val val of the value input
// _type type assosciativty
// _policy replacement policy
template<typename _key, typename _val, typename _type, typename _policy>
class Cache 
{
    private:
        unordered_map<_key, _val> cache;
        int capacity;
    public:
        //cache size
        Cache(int cap) : capacity(cap) {}

        _val get(_key key) 
        {
            if (cache.find(key) != cache.end()) {
                return cache[key];
            }
            return _val();
        }

        void put(_key key, _val value)
        {
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
}
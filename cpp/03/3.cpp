// 多参数模板：键值对映射
#include <iostream>
#include <map>
using namespace std;

template <typename K, typename V>
class Map
{
public:
    Map() {}
    ~Map() {}
    void put(K key, V value)
    {
        // 键值对映射
        m_map[key] = value;
    }
    V get(K key)
    {
        return m_map[key];
    }

private:
    // 键值对映射
    map<K, V> m_map;
};
int main()
{
    Map<string, int> map;
    map.put("张三", 18);
    cout << map.get("张三") << endl;
}
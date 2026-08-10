#include <cstring> // for strlen / strcpy
#include <utility> // for std::swap (可选，推荐)
#include <stdlib.h>

class String
{
public:
    String(const char *str = nullptr)
    {
        if (str == nullptr)
        {
            m_data = new char[1];
            *m_data = '\0';
        }
        else
        {
            int len = strlen(str);
            m_data = new char[len + 1];
            strcpy(m_data, str);
            m_data[len] = '\0';
        }
    }

    // 2. 拷贝构造函数（深拷贝）
    String(const String &other)
    {
        int len = strlen(other.m_data);
        m_data = new char[len + 1];
        strcpy(m_data, other.m_data);
        m_data[len] = '\0';
    }

    // 3. 析构函数
    ~String()
    {
        delete[] m_data;
        m_data = nullptr;
    }

    // 4. 赋值运算符重载（现代写法，异常安全）
    String &operator=(const String &rhs)
    {
        if (this == &rhs)
        { // 自赋值检查，避免误删自身数据
            return *this;
        }

        // 经典写法（直观，易得分）
        delete[] m_data;
        m_data = new char[strlen(rhs.m_data) + 1];
        strcpy(m_data, rhs.m_data);

        return *this;

        /* 面试加分写法（copy-and-swap）：
        String temp(rhs);          // 复用拷贝构造
        swap(m_data, temp.m_data); // 交换资源
        return *this;
        */
    }

private:
    char *m_data; // 字符串实际存储空间
};

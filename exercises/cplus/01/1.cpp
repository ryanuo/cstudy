class Date {
public:
    // ✅ 合法：完全缺省构造函数
    Date(int year = 1, int month = 1, int day = 1) {
        _year = year;
        _month = month;
        _day = day;
    }
private:
    int _year, _month, _day;
};

int main() {
    Date d1;       // 合法：调用完全缺省构造函数
    Date d2(2024); // 合法：只传 year，其余用默认值
    return 0;
}
#define fun(a, b) ((a) > (b) ? (a) : (b))
#define swap(a, b)     \
    {                  \
        typeof(a) tmp = (a); \
        (a) = (b);     \
        (b) = tmp;     \
    }
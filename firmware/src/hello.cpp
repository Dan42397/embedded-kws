#include <cstdint>
namespace {

class Counter {
public:
    void add(uint32_t v) { total_ += v; ++n_; }
    uint32_t mean() const { return n_ ? total_ / n_ : 0; }
private:
    uint32_t total_ = 0, n_ = 0;
};

Counter counter;
}

extern "C" uint32_t cpp_selftest(void) {

    for (uint32_t i = 1; i <= 10; ++i) counter.add(i);
    //static int *volatile keep;
    //keep = new int(42); // this is a memory leak,it asks the heap for memory while the program is running
    //counter.add(*keep);
    return counter.mean();
}




#include <include/stream.h>

class Replicable {
public:
    virtual bool process(WriteStream& s) = 0;
    virtual bool process(ReadStream& s)  = 0;
    virtual ~Replicable() = default;
    int id = 0;
};

template<typename Derived>
class ReplicableT : public Replicable {
public:
    bool process(WriteStream& s) override {
        return static_cast<Derived*>(this)->process_impl(s);
    }
    bool process(ReadStream& s) override {
        return static_cast<Derived*>(this)->process_impl(s);
    }
};

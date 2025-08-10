#include "standard/bounded_vector.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("BoundedVector")
{
    BoundedVector<int, 3> v;
    REQUIRE(v.Count() == 0);
    v.EmplaceBack() = 1;
    v.EmplaceBack(2);
    v.EmplaceBack(3);
    REQUIRE(v.Count() == 3);
    REQUIRE(v.IsFull());
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 3);
}

class Nontrivial
{
public:
    Nontrivial(int& counter)
        : m_counter(counter)
    {
        ++m_counter;
    }

    ~Nontrivial()
    {
        --m_counter;
    }

    Nontrivial(const Nontrivial&)            = delete;
    Nontrivial& operator=(const Nontrivial&) = delete;
    Nontrivial(Nontrivial&&)                 = delete;
    Nontrivial& operator=(Nontrivial&&)      = delete;

private:
    int& m_counter;
};

TEST_CASE("BoundedVector Nontrivial")
{
    int counter = 0;
    {
        BoundedVector<Nontrivial, 3> v;
        v.EmplaceBack(counter);
        REQUIRE(counter == 1);
        v.EmplaceBack(counter);
        REQUIRE(counter == 2);
        v.EmplaceBack(counter);
        REQUIRE(counter == 3);
        v.PopBack();
        REQUIRE(counter == 2);
    }
    REQUIRE(counter == 0);
}

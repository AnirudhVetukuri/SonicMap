#include "Map.hpp"
#include <gtest/gtest.h>

TEST(HashMapTest, InsertAndGet) {
    HashMap<int, std::string> map(4, 16);
    map.insert(1, "One");
    map.insert(2, "Two");

    EXPECT_EQ(map.get(1), "One");
    EXPECT_EQ(map.get(2), "Two");
    EXPECT_EQ(map.get(3), std::nullopt);
}

TEST(HashMapTest, Remove) {
    HashMap<int, std::string> map(4, 16);
    map.insert(1, "One");
    map.remove(1);

    EXPECT_EQ(map.get(1), std::nullopt);
}

TEST(HashMapTest, UpdateValue) {
    HashMap<int, std::string> map(4, 16);
    map.insert(1, "One");
    map.insert(1, "Uno");

    EXPECT_EQ(map.get(1), "Uno");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

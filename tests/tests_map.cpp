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

TEST(HashMapTest, Distribution) {
    HashMap<int, int> map(8, 128);
    size_t numKeys = 1000;

    std::vector<size_t> counts(map.getNumSegments(), 0);
    for (int i = 0; i < numKeys; ++i) {
        map.insert(i, i);
    }

    for (int i = 0; i < numKeys; ++i) {
        size_t segmentIndex = map.testGetSegmentIndex(i);
        counts[segmentIndex]++;
    }

    for (size_t count : counts) {
        EXPECT_NEAR(count, numKeys / map.getNumSegments(), numKeys * 0.1);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include "Map.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

TEST(HashMapTest, InsertAndGet)
{
    HashMap<int, std::string> map(4, 16);
    map.insert(1, "One");
    map.insert(2, "Two");

    EXPECT_EQ(map.get(1).value(), "One");
    EXPECT_EQ(map.get(2).value(), "Two");
    EXPECT_EQ(map.get(3), std::nullopt);
}

TEST(HashMapTest, Remove)
{
    HashMap<int, std::string> map(4, 16);
    map.insert(1, "One");
    map.remove(1);

    EXPECT_EQ(map.get(1), std::nullopt);
}

TEST(HashMapTest, UpdateValue)
{
    HashMap<int, std::string> map(4, 16);
    map.insert(1, "One");
    map.update(1, [](std::optional<std::string> &value)
               { value = "Uno"; });

    EXPECT_EQ(map.get(1).value(), "Uno");
}

TEST(HashMapTest, Distribution)
{
    HashMap<int, int> map(8, 128);
    size_t numKeys = 1000;

    std::vector<size_t> counts(map.getNumSegments(), 0);
    for (int i = 0; i < numKeys; ++i)
    {
        map.insert(i, i);
    }

    for (int i = 0; i < numKeys; ++i)
    {
        size_t segmentIndex = map.testGetSegmentIndex(i);
        counts[segmentIndex]++;
    }

    for (size_t count : counts)
    {
        EXPECT_NEAR(count, numKeys / map.getNumSegments(), numKeys * 0.1);
    }
}

void insertKeys(HashMap<int, int> &map, int start, int end)
{
    for (int i = start; i < end; ++i)
    {
        map.insert(i, i);
    }
}

TEST(HashMapTest, ConcurrentInsertions)
{
    HashMap<int, int> map(8, 128);
    std::thread t1(insertKeys, std::ref(map), 0, 500);
    std::thread t2(insertKeys, std::ref(map), 500, 1000);

    t1.join();
    t2.join();

    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_EQ(map.get(i).value(), i);
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
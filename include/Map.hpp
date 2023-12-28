#include <vector>
#include <optional>
#include "hash/xxhash.hpp"

template <typename K, typename V>
class HashMap {

private:
    struct Bucket {
        K key;
        V value;

        bool taken = false;
    };

    struct Segment {
        std::vector<Bucket> buckets;
        size_t size;

        Segment(size_t segmentSize) : size(segmentSize) {

            buckets.resize(size);
        }
    };

    size_t numSegments;
    std::vector<Segment> segments;

    size_t hashKey(const K& key) const {

        return XXH64(&key, sizeof(key), 0); 
    }

    size_t getSegmentIndex(const K& key) const {
        size_t hashValue = hashKey(key);

        return hashValue % numSegments;
    }

    size_t getBucketIndex(const K& key, size_t segmentSize) const {
        size_t hashValue = hashKey(key);

        return hashValue % segmentSize;
    }

public:
    explicit HashMap(size_t segmentCount = 8, size_t segmentSize = 128) : numSegments(segmentCount) {
        for (size_t i = 0; i < numSegments; ++i) {

            segments.emplace_back(segmentSize);
        }
    }

    void insert(const K& key, const V& value) {
        size_t segmentIndex = getSegmentIndex(key);
        size_t bucketIndex = getBucketIndex(key, segments[segmentIndex].size);

        Segment &segmentObj = segments[segmentIndex];
        Bucket &bucketObj = segmentObj.buckets[bucketIndex];

        while (bucketObj.taken) {
            if (bucketObj.key == key) {
                bucketObj.value = value;
                return;
            }

            bucketIndex = (bucketIndex + 1) % segmentObj.size;
            bucketObj = segmentObj.buckets[bucketIndex];
        }

        bucketObj.key = key;
        bucketObj.value = value;
        bucketObj.taken = true;
    }


    std::optional<V> get(const K& key) const {
        size_t segmentIndex = getSegmentIndex(key);
        size_t bucketIndex = getBucketIndex(key, segments[segmentIndex].size);

        const Segment &segmentObj = segments[segmentIndex];
        const Bucket &bucketObj = segmentObj.buckets[bucketIndex];

        while (bucketObj.taken) {
            if (bucketObj.key == key) {
                return bucketObj.value;
            }

            bucketIndex = (bucketIndex + 1) % segmentObj.size;
            bucketObj = segmentObj.buckets[bucketIndex];
        }

        return std::nullopt;
    }


    void remove(const K& key) {
        size_t segmentIndex = getSegmentIndex(key);
        size_t bucketIndex = getBucketIndex(key, segments[segmentIndex].size);

        Segment &segmentObj = segments[segmentIndex];
        Bucket &bucketObj = segmentObj.buckets[bucketIndex];

        while (bucketObj.taken) {
            if (bucketObj.key == key) {
                bucketObj.taken = false;
                return;
            }

            bucketIndex = (bucketIndex + 1) % segmentObj.size;
            bucketObj = segmentObj.buckets[bucketIndex];
        }
    }

};

#include <vector>
#include <optional>
#include <xxhash.h>

template <typename K, typename V>
class HashMap
{

private:
    struct Bucket
    {
        K key;
        V value;

        bool taken = false;
    };

    struct Segment
    {
        std::vector<Bucket> buckets;
        size_t size;

        Segment(size_t segmentSize) : size(segmentSize)
        {

            buckets.resize(size);
        }
    };

    size_t numSegments;
    size_t segmentBits;
    std::vector<Segment> segments;

    size_t hashKey(const K &key) const
    {

        return XXH64(&key, sizeof(key), 0);
    }

    size_t getSegmentIndex(const K &key) const
    {
        size_t hashValue = hashKey(key);

        return (hashValue >> (64 - segmentBits)) % numSegments;
    }

    size_t getBucketIndex(const K &key, size_t segmentSize) const
    {
        size_t hashValue = hashKey(key);

        return hashValue % segmentSize;
    }

public:
    size_t getNumSegments() const
    {
        return numSegments;
    }

    size_t testGetSegmentIndex(const K &key) const
    {
        return getSegmentIndex(key);
    }

    explicit HashMap(size_t segmentCount = 8, size_t segmentSize = 128) : numSegments(segmentCount), segmentBits(0)
    {
        while (segmentCount >>= 1)
            ++segmentBits;

        for (size_t i = 0; i < numSegments; ++i)
        {

            segments.emplace_back(segmentSize);
        }
    }

    void insert(const K &key, const V &value)
    {
        size_t segmentIndex = getSegmentIndex(key);
        size_t bucketIndex = getBucketIndex(key, segments[segmentIndex].size);

        Segment &segmentObj = segments[segmentIndex];
        Bucket &bucketObj = segmentObj.buckets[bucketIndex];

        while (bucketObj.taken)
        {
            if (bucketObj.key == key)
            {
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

    std::optional<V> get(const K &key) const
    {
        size_t segmentIndex = getSegmentIndex(key);
        size_t bucketIndex = getBucketIndex(key, segments[segmentIndex].size);

        const Segment &segmentObj = segments[segmentIndex];

        while (segmentObj.buckets[bucketIndex].taken)
        {
            if (segmentObj.buckets[bucketIndex].key == key)
            {
                return segmentObj.buckets[bucketIndex].value;
            }

            bucketIndex = (bucketIndex + 1) % segmentObj.size;
        }
        return std::nullopt;
    }

    void remove(const K &key)
    {
        size_t segmentIndex = getSegmentIndex(key);
        size_t bucketIndex = getBucketIndex(key, segments[segmentIndex].size);

        Segment &segmentObj = segments[segmentIndex];
        Bucket &bucketObj = segmentObj.buckets[bucketIndex];

        while (bucketObj.taken)
        {
            if (bucketObj.key == key)
            {
                bucketObj.taken = false;
                return;
            }

            bucketIndex = (bucketIndex + 1) % segmentObj.size;
            bucketObj = segmentObj.buckets[bucketIndex];
        }
    }

    void update(const K &key, std::function<void(std::optional<V> &)> updateFunc, std::optional<V> valueToInsert = std::nullopt)
    {
        size_t segmentIndex = getSegmentIndex(key);
        Segment &segment = segments[segmentIndex];

        {
            std::shared_lock<std::shared_mutex> lock(segment.mutex); // Read lock
            size_t bucketIndex = getBucketIndex(key, segment.size);

            while (segment.buckets[bucketIndex].taken)
            {
                if (segment.buckets[bucketIndex].key == key)
                {
                    // Apply the update function
                    updateFunc(segment.buckets[bucketIndex].value);
                    return;
                }
                bucketIndex = (bucketIndex + 1) % segment.size;
            }
        }

        // Upgrade to write lock if we need to insert
        if (valueToInsert.has_value())
        {
            std::unique_lock<std::shared_mutex> lock(segment.mutex); // Write lock
            size_t bucketIndex = getBucketIndex(key, segment.size);

            // Re-check if the key was inserted by another thread in the meantime
            while (segment.buckets[bucketIndex].taken)
            {
                if (segment.buckets[bucketIndex].key == key)
                {
                    updateFunc(segment.buckets[bucketIndex].value);
                    return;
                }
                bucketIndex = (bucketIndex + 1) % segment.size;
            }

            // Insert the new value
            segment.buckets[bucketIndex].key = key;
            segment.buckets[bucketIndex].value = *valueToInsert;
            segment.buckets[bucketIndex].taken = true;
        }
    }
};

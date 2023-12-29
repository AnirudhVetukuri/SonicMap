#include <vector>
#include <optional>
#include <shared_mutex>
#include <xxhash.h>
#include <functional>
#include <mutex>

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
        mutable std::shared_mutex mutex;

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
    explicit HashMap(size_t segmentCount = 8, size_t segmentSize = 128)
        : numSegments(segmentCount), segmentBits(0)
    {
        while (segmentCount >>= 1)
            ++segmentBits;
        for (size_t i = 0; i < numSegments; ++i)
        {
            segments.emplace_back(segmentSize);
        }
    }

    size_t getNumSegments() const
    {
        return numSegments;
    }

    void insert(const K &key, const V &value)
    {
        size_t segmentIndex = getSegmentIndex(key);
        Segment &segment = segments[segmentIndex];

        std::unique_lock<std::shared_mutex> lock(segment.mutex);

        size_t bucketIndex = getBucketIndex(key, segment.size);
        Bucket &bucket = segment.buckets[bucketIndex];

        while (bucket.taken)
        {
            if (bucket.key == key)
            {
                bucket.value = value;
                return;
            }
            bucketIndex = (bucketIndex + 1) % segment.size;
            bucket = segment.buckets[bucketIndex];
        }

        bucket.key = key;
        bucket.value = value;
        bucket.taken = true;
    }

    std::optional<V> get(const K &key) const
    {
        size_t segmentIndex = getSegmentIndex(key);
        const Segment &segment = segments[segmentIndex];

        std::shared_lock<std::shared_mutex> lock(segment.mutex);

        size_t bucketIndex = getBucketIndex(key, segment.size);
        while (segment.buckets[bucketIndex].taken)
        {
            if (segment.buckets[bucketIndex].key == key)
            {
                return segment.buckets[bucketIndex].value;
            }
            bucketIndex = (bucketIndex + 1) % segment.size;
        }
        return std::nullopt;
    }

    void remove(const K &key)
    {
        size_t segmentIndex = getSegmentIndex(key);
        Segment &segment = segments[segmentIndex];

        std::unique_lock<std::shared_mutex> lock(segment.mutex);

        size_t bucketIndex = getBucketIndex(key, segment.size);
        Bucket &bucket = segment.buckets[bucketIndex];

        while (bucket.taken)
        {
            if (bucket.key == key)
            {
                bucket.taken = false;
                return;
            }
            bucketIndex = (bucketIndex + 1) % segment.size;
            bucket = segment.buckets[bucketIndex];
        }
    }

    void update(const K &key, std::function<void(std::optional<V> &)> updateFunc, std::optional<V> valueToInsert = std::nullopt)
    {
        size_t segmentIndex = getSegmentIndex(key);
        Segment &segment = segments[segmentIndex];

        {
            std::shared_lock<std::shared_mutex> lock(segment.mutex);
            size_t bucketIndex = getBucketIndex(key, segment.size);

            while (segment.buckets[bucketIndex].taken)
            {
                if (segment.buckets[bucketIndex].key == key)
                {
                    updateFunc(segment.buckets[bucketIndex].value);
                    return;
                }
                bucketIndex = (bucketIndex + 1) % segment.size;
            }
        }

        if (valueToInsert.has_value())
        {
            std::unique_lock<std::shared_mutex> lock(segment.mutex);
            size_t bucketIndex = getBucketIndex(key, segment.size);

            while (segment.buckets[bucketIndex].taken)
            {
                if (segment.buckets[bucketIndex].key == key)
                {
                    updateFunc(segment.buckets[bucketIndex].value);
                    return;
                }
                bucketIndex = (bucketIndex + 1) % segment.size;
            }

            segment.buckets[bucketIndex].key = key;
            segment.buckets[bucketIndex].value = *valueToInsert;
            segment.buckets[bucketIndex].taken = true;
        }
    }
};

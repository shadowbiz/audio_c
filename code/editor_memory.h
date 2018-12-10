#pragma once

struct Partition
{
    char *base;
    i32   pos;
    i32   max;
};

struct TempMemory
{
    void *handle;
    i32   pos;
};

inline Partition
PartitionMake(void *memory, i32 size)
{
    Partition partition;
    
    partition.base = (char*)memory;
    partition.pos = 0;
    partition.max = size;
    
    return partition;
}

inline void*
PartitionAllocate(Partition *data, i32 size)
{
    void *result = 0;
    if (size > 0 && (data->pos + size) <= data->max)
    {
        result = data->base + data->pos;
        data->pos += size;
    }
    return result;
}

inline void
PartitionAlign(Partition *data, u32 boundary)
{
    --boundary;
    data->pos = (data->pos + boundary) & (~boundary);
}

inline void*
PartitionCurrent(Partition *data)
{
    return data->base + data->pos;
}

inline i32
PartitionRemaining(Partition *data)
{
    return data->max - data->pos;
}

inline Partition
PartitionSubPart(Partition *data, i32 size)
{
    Partition result = {};
    void *d = PartitionAllocate(data, size);
    if (d) 
    {
        result = PartitionMake(d, size);
    }
    return result;
}


#define PushStruct(part, T)      (T*)PartitionAllocate(part, sizeof(T))
#define PushArray(part, T, size) (T*)PartitionAllocate(part, sizeof(T)*(size))
#define PushBlock(part, size)    PartitionAllocate(part, size)

inline TempMemory
TempMemoryBegin(Partition *data)
{
    TempMemory result;
    result.handle = data;
    result.pos = data->pos;
    return result;
}

inline void
TempMemoryEnd(TempMemory temp)
{
    ((Partition*)temp.handle)->pos = temp.pos;
}
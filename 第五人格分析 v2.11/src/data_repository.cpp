#include "data_repository.h"
#include "hunter_data.h"
#include "map_data.h"
#include "survivor_data.h"

const survivor_static_data* find_survivor_data(int id)
{
    for (const auto& data : k_survivor_data)
    {
        if (data.id == id)
        {
            return &data;
        }
    }
    return nullptr;
}

const hunter_static_data* find_hunter_data(int id)
{
    for (const auto& data : k_hunter_data)
    {
        if (data.id == id)
        {
            return &data;
        }
    }
    return nullptr;
}

const map_static_data* find_map_data(int id)
{
    for (const auto& data : k_map_data)
    {
        if (data.id == id)
        {
            return &data;
        }
    }
    return nullptr;
}

int max_survivor_id()
{
    return k_survivor_data.empty() ? 0 : k_survivor_data.back().id;
}

int max_hunter_id()
{
    return k_hunter_data.empty() ? 0 : k_hunter_data.back().id;
}

int max_map_id()
{
    return k_map_data.empty() ? 0 : k_map_data.back().id;
}

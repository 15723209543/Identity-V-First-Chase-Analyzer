#pragma once

#include "auxiliary_trait.h"
#include "game_map.h"
#include "hunter.h"
#include "survivor.h"

const auxiliary_trait_static_data* find_auxiliary_trait_data(int id);
const survivor_static_data* find_survivor_data(int id);
const hunter_static_data* find_hunter_data(int id);
const map_static_data* find_map_data(int id);

int max_auxiliary_trait_id();
int max_survivor_id();
int max_hunter_id();
int max_map_id();

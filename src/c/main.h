#pragma once

#include "pebble.h"

// Minute hand
static const GPathInfo MINUTE_HAND_POINTS = {
 4,
  (GPoint []) {
    { 3,  0},
    { 3,-37},
    {-3,-37},
    {-3,  0},
  }
};

// Hour hand
static const GPathInfo HOUR_HAND_POINTS = {
 4,
  (GPoint []) {
    { 3,  0},
    { 3,-25},
    {-3,-25},
    {-3,  0},  
  }
};

static const GPathInfo SEC_POINTS = {
  4,
  (GPoint []) {
    { 2,  0},
    { 2,-37},
    {-2,-37},
    {-2,  0},
  }
};

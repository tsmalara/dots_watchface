#pragma once

#include "pebble.h"

// Minute hand
static const GPathInfo MINUTE_HAND_POINTS = {
 4,
  (GPoint []) {
    { 3,  0},
    { 3,-40},
    {-3,-40},
    {-3,  0},
  }
};

// Hour hand
static const GPathInfo HOUR_HAND_POINTS = {
 4,
  (GPoint []) {
    { 3,  0},
    { 3,-28},
    {-3,-28},
    {-3,  0},  
  }
};

static const GPathInfo SEC_POINTS = {
  4,
  (GPoint []) {
    { 2,  0},
    { 2,-40},
    {-2,-40},
    {-2,  0},
  }
};

#pragma once

#include "pebble.h"

// Minute hand
static const GPathInfo MINUTE_HAND_POINTS = {
 6,
  (GPoint []) {
    { 5,-39},
    { 2,-42},
    {-1,-42},
    {-5,-39},
    {-5,  0},
    { 5,  0},
  }
};

// Hour hand
static const GPathInfo HOUR_HAND_POINTS = {
 6,
  (GPoint []) {
    { 5,-27},
    { 2,-30},
    {-1,-30},
    {-5,-27},
    {-5,  0},
    { 5,  0}, 
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

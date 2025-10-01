#pragma once

#include <gtest/gtest.h>
#include <raylib.h>

namespace EpiGimp {

class GlobalTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        SetConfigFlags(FLAG_WINDOW_HIDDEN);
        InitWindow(800, 600, "EpiGimp Tests");
        SetTargetFPS(60);
    }

    void TearDown() override {
        CloseWindow();
    }
};

} // namespace EpiGimp
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include <algorithm>

// Mock structures for graphics operations (similar to raylib but without dependency)
struct TestColor {
    unsigned char r, g, b, a;
    
    TestColor() : r(0), g(0), b(0), a(255) {}
    TestColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
    
    bool operator==(const TestColor& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
};

struct TestVector2 {
    float x, y;
    
    TestVector2() : x(0.0f), y(0.0f) {}
    TestVector2(float x_val, float y_val) : x(x_val), y(y_val) {}
    
    TestVector2 operator+(const TestVector2& other) const {
        return TestVector2(x + other.x, y + other.y);
    }
    
    TestVector2 operator-(const TestVector2& other) const {
        return TestVector2(x - other.x, y - other.y);
    }
    
    TestVector2 operator*(float scalar) const {
        return TestVector2(x * scalar, y * scalar);
    }
    
    float length() const {
        return std::sqrt(x * x + y * y);
    }
    
    TestVector2 normalize() const {
        float len = length();
        return len > 0 ? TestVector2(x / len, y / len) : TestVector2();
    }
    
    float dot(const TestVector2& other) const {
        return x * other.x + y * other.y;
    }
};

struct TestRectangle {
    float x, y, width, height;
    
    TestRectangle() : x(0), y(0), width(0), height(0) {}
    TestRectangle(float x_val, float y_val, float w, float h)
        : x(x_val), y(y_val), width(w), height(h) {}
    
    bool contains(const TestVector2& point) const {
        return point.x >= x && point.x <= x + width &&
               point.y >= y && point.y <= y + height;
    }
    
    bool intersects(const TestRectangle& other) const {
        return !(x + width < other.x || other.x + other.width < x ||
                y + height < other.y || other.y + other.height < y);
    }
    
    float area() const {
        return width * height;
    }
};

// Canvas utility functions
class CanvasUtilsTest : public ::testing::Test {
protected:
    TestColor interpolateColor(const TestColor& color1, const TestColor& color2, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        
        return TestColor(
            static_cast<unsigned char>(color1.r * (1 - t) + color2.r * t),
            static_cast<unsigned char>(color1.g * (1 - t) + color2.g * t),
            static_cast<unsigned char>(color1.b * (1 - t) + color2.b * t),
            static_cast<unsigned char>(color1.a * (1 - t) + color2.a * t)
        );
    }
    
    TestColor blendColors(const TestColor& base, const TestColor& overlay) {
        float alpha = overlay.a / 255.0f;
        float invAlpha = 1.0f - alpha;
        
        return TestColor(
            static_cast<unsigned char>(base.r * invAlpha + overlay.r * alpha),
            static_cast<unsigned char>(base.g * invAlpha + overlay.g * alpha),
            static_cast<unsigned char>(base.b * invAlpha + overlay.b * alpha),
            255
        );
    }
    
    float distancePointToLine(const TestVector2& point, const TestVector2& lineStart, const TestVector2& lineEnd) {
        TestVector2 line = lineEnd - lineStart;
        TestVector2 pointToStart = point - lineStart;
        
        float lineLength = line.length();
        if (lineLength == 0) return pointToStart.length();
        
        float t = std::clamp(pointToStart.dot(line) / (lineLength * lineLength), 0.0f, 1.0f);
        TestVector2 projection = lineStart + line * t;
        
        return (point - projection).length();
    }
    
    std::vector<TestVector2> bresenhamLine(const TestVector2& start, const TestVector2& end) {
        std::vector<TestVector2> points;
        
        int x0 = static_cast<int>(start.x);
        int y0 = static_cast<int>(start.y);
        int x1 = static_cast<int>(end.x);
        int y1 = static_cast<int>(end.y);
        
        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        
        while (true) {
            points.emplace_back(static_cast<float>(x0), static_cast<float>(y0));
            
            if (x0 == x1 && y0 == y1) break;
            
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
        
        return points;
    }
};

// Color tests
TEST_F(CanvasUtilsTest, ColorCreation) {
    TestColor red(255, 0, 0);
    EXPECT_EQ(red.r, 255);
    EXPECT_EQ(red.g, 0);
    EXPECT_EQ(red.b, 0);
    EXPECT_EQ(red.a, 255);
    
    TestColor transparent(0, 0, 0, 0);
    EXPECT_EQ(transparent.a, 0);
}

TEST_F(CanvasUtilsTest, ColorInterpolation) {
    TestColor red(255, 0, 0);
    TestColor blue(0, 0, 255);
    
    TestColor middle = interpolateColor(red, blue, 0.5f);
    EXPECT_EQ(middle.r, 127);
    EXPECT_EQ(middle.g, 0);
    EXPECT_EQ(middle.b, 127);
    
    TestColor atStart = interpolateColor(red, blue, 0.0f);
    EXPECT_EQ(atStart, red);
    
    TestColor atEnd = interpolateColor(red, blue, 1.0f);
    EXPECT_EQ(atEnd, blue);
}

TEST_F(CanvasUtilsTest, ColorBlending) {
    TestColor white(255, 255, 255);
    TestColor semiTransparentRed(255, 0, 0, 128);
    
    TestColor blended = blendColors(white, semiTransparentRed);
    
    // With alpha = 128/255 ≈ 0.502, we expect:
    // Red: 255 * (1-0.502) + 255 * 0.502 ≈ 255
    // Green: 255 * (1-0.502) + 0 * 0.502 ≈ 127  
    // Blue: 255 * (1-0.502) + 0 * 0.502 ≈ 127
    EXPECT_GT(blended.r, 200);  // Should be close to 255
    EXPECT_GT(blended.g, 120);  // Should be around 127
    EXPECT_LT(blended.g, 135);  // Should be around 127
    EXPECT_GT(blended.b, 120);  // Should be around 127  
    EXPECT_LT(blended.b, 135);  // Should be around 127
}

// Vector tests
TEST_F(CanvasUtilsTest, VectorOperations) {
    TestVector2 v1(3.0f, 4.0f);
    TestVector2 v2(1.0f, 2.0f);
    
    // Length
    EXPECT_FLOAT_EQ(v1.length(), 5.0f);
    
    // Addition
    TestVector2 sum = v1 + v2;
    EXPECT_FLOAT_EQ(sum.x, 4.0f);
    EXPECT_FLOAT_EQ(sum.y, 6.0f);
    
    // Subtraction
    TestVector2 diff = v1 - v2;
    EXPECT_FLOAT_EQ(diff.x, 2.0f);
    EXPECT_FLOAT_EQ(diff.y, 2.0f);
    
    // Scalar multiplication
    TestVector2 scaled = v1 * 2.0f;
    EXPECT_FLOAT_EQ(scaled.x, 6.0f);
    EXPECT_FLOAT_EQ(scaled.y, 8.0f);
    
    // Dot product
    EXPECT_FLOAT_EQ(v1.dot(v2), 11.0f); // 3*1 + 4*2 = 11
}

TEST_F(CanvasUtilsTest, VectorNormalization) {
    TestVector2 v(3.0f, 4.0f);
    TestVector2 normalized = v.normalize();
    
    EXPECT_FLOAT_EQ(normalized.length(), 1.0f);
    EXPECT_FLOAT_EQ(normalized.x, 0.6f);
    EXPECT_FLOAT_EQ(normalized.y, 0.8f);
    
    // Test zero vector normalization
    TestVector2 zero(0.0f, 0.0f);
    TestVector2 normalizedZero = zero.normalize();
    EXPECT_FLOAT_EQ(normalizedZero.x, 0.0f);
    EXPECT_FLOAT_EQ(normalizedZero.y, 0.0f);
}

// Rectangle tests
TEST_F(CanvasUtilsTest, RectangleOperations) {
    TestRectangle rect(10, 20, 100, 50);
    
    EXPECT_FLOAT_EQ(rect.area(), 5000.0f);
    
    // Point containment
    EXPECT_TRUE(rect.contains(TestVector2(50, 40)));
    EXPECT_FALSE(rect.contains(TestVector2(5, 10)));
    EXPECT_FALSE(rect.contains(TestVector2(150, 100)));
    
    // Edge cases
    EXPECT_TRUE(rect.contains(TestVector2(10, 20))); // Top-left corner
    EXPECT_TRUE(rect.contains(TestVector2(110, 70))); // Bottom-right corner
}

TEST_F(CanvasUtilsTest, RectangleIntersection) {
    TestRectangle rect1(0, 0, 50, 50);
    TestRectangle rect2(25, 25, 50, 50);
    TestRectangle rect3(100, 100, 50, 50);
    
    // Overlapping rectangles
    EXPECT_TRUE(rect1.intersects(rect2));
    EXPECT_TRUE(rect2.intersects(rect1));
    
    // Non-overlapping rectangles
    EXPECT_FALSE(rect1.intersects(rect3));
    EXPECT_FALSE(rect3.intersects(rect1));
}

// Geometric operations
TEST_F(CanvasUtilsTest, DistancePointToLine) {
    TestVector2 point(2, 2);
    TestVector2 lineStart(0, 0);
    TestVector2 lineEnd(4, 0);
    
    float distance = distancePointToLine(point, lineStart, lineEnd);
    EXPECT_FLOAT_EQ(distance, 2.0f);
    
    // Point on line
    TestVector2 pointOnLine(2, 0);
    float distanceOnLine = distancePointToLine(pointOnLine, lineStart, lineEnd);
    EXPECT_NEAR(distanceOnLine, 0.0f, 0.001f);
}

TEST_F(CanvasUtilsTest, BresenhamLine) {
    TestVector2 start(0, 0);
    TestVector2 end(3, 3);
    
    std::vector<TestVector2> points = bresenhamLine(start, end);
    
    EXPECT_EQ(points.size(), 4);
    EXPECT_FLOAT_EQ(points[0].x, 0.0f);
    EXPECT_FLOAT_EQ(points[0].y, 0.0f);
    EXPECT_FLOAT_EQ(points[3].x, 3.0f);
    EXPECT_FLOAT_EQ(points[3].y, 3.0f);
    
    // Test horizontal line
    std::vector<TestVector2> horizontal = bresenhamLine(TestVector2(0, 0), TestVector2(3, 0));
    EXPECT_EQ(horizontal.size(), 4);
    for (const auto& point : horizontal) {
        EXPECT_FLOAT_EQ(point.y, 0.0f);
    }
}

// Canvas coordinate transformations
TEST_F(CanvasUtilsTest, CoordinateTransformations) {
    // Screen to world coordinates
    TestVector2 screenSize(800, 600);
    TestVector2 worldSize(100, 75);
    
    auto screenToWorld = [&](const TestVector2& screen) -> TestVector2 {
        return TestVector2(
            (screen.x / screenSize.x) * worldSize.x,
            (screen.y / screenSize.y) * worldSize.y
        );
    };
    
    auto worldToScreen = [&](const TestVector2& world) -> TestVector2 {
        return TestVector2(
            (world.x / worldSize.x) * screenSize.x,
            (world.y / worldSize.y) * screenSize.y
        );
    };
    
    TestVector2 screenPoint(400, 300); // Center of screen
    TestVector2 worldPoint = screenToWorld(screenPoint);
    TestVector2 backToScreen = worldToScreen(worldPoint);
    
    EXPECT_FLOAT_EQ(worldPoint.x, 50.0f);
    EXPECT_FLOAT_EQ(worldPoint.y, 37.5f);
    EXPECT_FLOAT_EQ(backToScreen.x, 400.0f);
    EXPECT_FLOAT_EQ(backToScreen.y, 300.0f);
}

// Brush operations
TEST_F(CanvasUtilsTest, CircularBrushMask) {
    auto generateCircleMask = [](int radius) -> std::vector<std::vector<float>> {
        int size = radius * 2 + 1;
        std::vector<std::vector<float>> mask(size, std::vector<float>(size, 0.0f));
        
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                float dx = x - radius;
                float dy = y - radius;
                float distance = std::sqrt(dx * dx + dy * dy);
                
                if (distance <= radius) {
                    mask[y][x] = 1.0f - (distance / radius);
                }
            }
        }
        
        return mask;
    };
    
    auto mask = generateCircleMask(2);
    
    EXPECT_EQ(mask.size(), 5);
    EXPECT_EQ(mask[0].size(), 5);
    
    // Center should have maximum intensity
    EXPECT_FLOAT_EQ(mask[2][2], 1.0f);
    
    // Corners should be zero (outside circle)
    EXPECT_FLOAT_EQ(mask[0][0], 0.0f);
    EXPECT_FLOAT_EQ(mask[0][4], 0.0f);
    EXPECT_FLOAT_EQ(mask[4][0], 0.0f);
    EXPECT_FLOAT_EQ(mask[4][4], 0.0f);
}
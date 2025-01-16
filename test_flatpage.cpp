#include <gtest/gtest.h>
#include "FlatPage.h"

// Define a fixture class for the FlatPage tests
class FlatPageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up the FlatPage with some initial data
        flatPage = new FlatPage<int, int>(4, true);
        flatPage->add(1, 1);
        flatPage->add(2, 2);
        flatPage->add(3, 3);
    }

    void TearDown() override {
        delete flatPage;
    }

    FlatPage<int, int>* flatPage;
};

// Test case for the add method
TEST_F(FlatPageTest, Add) {
    flatPage->add(4, 4);
    EXPECT_EQ(flatPage->count(), 4);
    EXPECT_EQ(*flatPage->getValue(4), 4);
}

// Test case for the getValue method
TEST_F(FlatPageTest, GetValue) {
    EXPECT_EQ(*flatPage->getValue(1), 1);
    EXPECT_EQ(*flatPage->getValue(2), 2);
    EXPECT_EQ(*flatPage->getValue(3), 3);
}

// Test case for the isFull method
TEST_F(FlatPageTest, IsFull) {
    EXPECT_FALSE(flatPage->isFull());
    flatPage->add(4, 4);
    flatPage->add(5, 5);
    EXPECT_TRUE(flatPage->isFull());
}

// Test case for the split method
TEST_F(FlatPageTest, Split) {
    flatPage->add(4, 4);
    flatPage->add(5, 5);
    FlatPage<int, int>* newPage = flatPage->split();
    EXPECT_EQ(flatPage->count(), 2);
    EXPECT_EQ(newPage->count(), 3);
    delete newPage;
}


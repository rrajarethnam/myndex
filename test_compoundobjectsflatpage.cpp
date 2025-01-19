#include <gtest/gtest.h>
#include "CompoundObjectsFlatPage.h"

// Define a fixture class for the CompoundObjectsFlatPage tests
class CompoundObjectsFlatPageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up the CompoundObjectsFlatPage with some initial data
        compoundPage = new CompoundObjectsFlatPage<std::string, std::string>(4, true);
        compoundPage->add("key1", "value1");
        compoundPage->add("key2", "value2");
        compoundPage->add("key3", "value3");
    }

    void TearDown() override {
        delete compoundPage;
    }

    CompoundObjectsFlatPage<std::string, std::string>* compoundPage;
};

// Test case for the add method
TEST_F(CompoundObjectsFlatPageTest, Add) {
    compoundPage->add("key4", "value4");
    EXPECT_EQ(compoundPage->count(), 4);
    EXPECT_EQ(*compoundPage->getValue("key4"), "value4");
}

// Test case for the getValue method
TEST_F(CompoundObjectsFlatPageTest, GetValue) {
    EXPECT_EQ(*compoundPage->getValue("key1"), "value1");
    EXPECT_EQ(*compoundPage->getValue("key2"), "value2");
    EXPECT_EQ(*compoundPage->getValue("key3"), "value3");
}

// Test case for the isFull method
TEST_F(CompoundObjectsFlatPageTest, IsFull) {
    EXPECT_FALSE(compoundPage->isFull());
    compoundPage->add("key4", "value4");
    compoundPage->add("key5", "value5");
    EXPECT_TRUE(compoundPage->isFull());
}

// Test case for the split method
TEST_F(CompoundObjectsFlatPageTest, Split) {
    compoundPage->add("key4", "value4");
    compoundPage->add("key5", "value5");
    CompoundObjectsFlatPage<std::string, std::string>* newPage = compoundPage->split();
    EXPECT_EQ(compoundPage->count(), 3);
    EXPECT_EQ(newPage->count(), 2);
    delete newPage;
}

// Test case for the save and open methods
TEST_F(CompoundObjectsFlatPageTest, SaveAndOpen) {
    compoundPage->add("key4", "value4");
    compoundPage->add("key5", "value5");
    CompoundObjectsFlatPage<std::string, std::string>* newPage = compoundPage->split();
    EXPECT_EQ(compoundPage->count(), 3);
    EXPECT_EQ(newPage->count(), 2);
    CompoundObjectsFlatPage<std::string, std::string>* root = new CompoundObjectsFlatPage<std::string, std::string>(4, false);
    root->add(compoundPage->firstKey(), compoundPage);
    root->add(newPage->firstKey(), newPage);
    root->save();

    CompoundObjectsFlatPage<std::string, std::string> loadedPage(root->getId(), 4);
    loadedPage.open();
    EXPECT_EQ(loadedPage.count(), 2);
    EXPECT_EQ(loadedPage.firstKey(), "key1");
    EXPECT_EQ(loadedPage.lastKey(), "key4");
    
}


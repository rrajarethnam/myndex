#include <gtest/gtest.h>
#include "btree.h"
#include "Iterator.h"
#include "CompoundObjectsFlatPage.h"

// Define a fixture class for the B-tree tests
class BtreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up the B-tree with some initial data
        btree = new Btree<std::string, std::string, CompoundObjectsFlatPage<std::string, std::string>>(4, "", "", true);
        btree->put("1", "one");
        btree->put("2", "two");
        btree->put("3", "three");
        btree->put("4", "four");
        btree->put("5", "five");
    }

    void TearDown() override {
        delete btree;
    }

    Btree<std::string, std::string, CompoundObjectsFlatPage<std::string, std::string>>* btree;
};

// Test case for the get method
TEST_F(BtreeTest, GetRange) {
    Iterator<std::string, std::string, Page<std::string, std::string>> it = btree->get("2", "4");

    std::vector<std::pair<std::string, std::string>> expected = {
        {"2", "two"},
        {"3", "three"},
        {"4", "four"}
    };
    //std::cout << **it << std::endl;

    for (const auto& pair : expected) {
        //ASSERT_FALSE(it.isEnd());
        EXPECT_EQ(**it, pair.second);
        ++it;
    }

    ASSERT_TRUE(it.isEnd());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#include <iostream>
#include <memory>
#include <gtest/gtest.h>
#include "hostname.h"

using namespace std;
using namespace jouyouyun::example::unittest;

TEST(GetHostname, Get) {
    Hostname h("testdata/hostname");
    EXPECT_STREQ("Jouyouyun", h.Get().c_str());
}

TEST(SetHostname, Set) {
    Hostname h("testdata/hostname_tmp");
    EXPECT_EQ(0, h.Set("Deepin"));
    EXPECT_STREQ("Deepin", h.Get().c_str());
}

// shared data, but the operations in each 'TEST_F' don't affect each other
class HostnameTest : public testing::Test {
    protected:
    // init
    virtual void SetUp() override {
        core = unique_ptr<Hostname>(new Hostname("testdata/hostname_tmp"));
    }
    // finalize
    virtual void TearDown() override {}

    unique_ptr<Hostname> core;
};
TEST_F(HostnameTest, SetAndGet) {
    EXPECT_EQ(0, core->Set("Wen"));
    EXPECT_STREQ("Wen", core->Get().c_str());
}
TEST_F(HostnameTest, Get) {
    EXPECT_STREQ("Wen", core->Get().c_str());
}

int
main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
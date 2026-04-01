#include <gtest/gtest.h>
#include "iso.hpp"
#include <fstream>

class IsoTest : public ::testing::Test {
protected:
    Iso iso;
};

TEST_F(IsoTest, IsWindowsValid) {
    std::string test_iso = "/tmp/test.iso";
    
    // Create dummy ISO with install.wim
    system("mkdir -p /tmp/iso_test/sources && touch /tmp/iso_test/sources/install.wim && mkisofs -o /tmp/test.iso /tmp/iso_test && rm -rf /tmp/iso_test");

    EXPECT_EQ(iso.addIsoInfo(test_iso, Iso::IsoType::WINDOWS), "OK");
    
    remove(test_iso.c_str());
}

TEST_F(IsoTest, IsWindowsInvalid) {
    // Create an ISO without the required files
    std::string invalid_iso = "/tmp/invalid_test.iso";
    system("mkdir -p /tmp/iso_invalid && mkisofs -o /tmp/invalid_test.iso /tmp/iso_invalid && rm -rf /tmp/iso_invalid");

    EXPECT_EQ(iso.addIsoInfo(invalid_iso, Iso::IsoType::WINDOWS), "ERROR");
    
    // Cleanup
    remove(invalid_iso.c_str());
}

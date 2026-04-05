#include <gtest/gtest.h>
#include "../src/extractor/iso.hpp"
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>

class IsoTest : public ::testing::Test {
protected:
    Iso iso;

    bool fileExists(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0;
    }

    bool commandExists(const std::string& cmd) {
        std::string check = "command -v " + cmd + " >/dev/null 2>&1";
        return std::system(check.c_str()) == 0;
    }

    std::string selectIsoCreator() {
        if (commandExists("mkisofs")) return "mkisofs";
        if (commandExists("genisoimage")) return "genisoimage";
        if (commandExists("xorriso")) return "xorriso -as mkisofs";
        return "";
    }

    bool has7zLibrary() {
        return fileExists("/usr/lib/7zip/7z.so") ||
               fileExists("/usr/lib/p7zip/7z.so") ||
               fileExists("/usr/lib64/7zip/7z.so") ||
               fileExists("/usr/lib64/p7zip/7z.so") ||
               fileExists("/usr/lib64/7z.so");
    }

    std::string depsMissingReason() {
        std::string creator = selectIsoCreator();
        if (creator.empty()) {
            return "No ISO creator found (mkisofs/genisoimage/xorriso)";
        }
        if (!commandExists("isoinfo")) {
            return "Missing isoinfo command";
        }
        if (!has7zLibrary()) {
            return "Missing 7z library (expected /usr/lib/7zip/7z.so or /usr/lib/p7zip/7z.so)";
        }
        return "";
    }

    void createUdfIsoWithInstallWim(const std::string& isoPath) {
        std::string creator = selectIsoCreator();
        std::string cmd = "mkdir -p /tmp/iso_test/sources && "
                          "touch /tmp/iso_test/sources/install.wim && " +
                          creator + " -udf -o " + isoPath + " /tmp/iso_test >/dev/null 2>&1 && "
                          "rm -rf /tmp/iso_test";
        if (std::system(cmd.c_str()) != 0) {
            throw std::runtime_error("Could not create UDF ISO with install.wim");
        }
    }

    void createUdfIsoEmpty(const std::string& isoPath) {
        std::string creator = selectIsoCreator();
        std::string cmd = "mkdir -p /tmp/iso_invalid && " +
                          creator + " -udf -o " + isoPath + " /tmp/iso_invalid >/dev/null 2>&1 && "
                          "rm -rf /tmp/iso_invalid";
        if (std::system(cmd.c_str()) != 0) {
            throw std::runtime_error("Could not create UDF ISO");
        }
    }
};

TEST_F(IsoTest, IsWindowsValid) {
    std::string reason = depsMissingReason();
    if (!reason.empty()) {
        GTEST_SKIP() << reason;
    }
    std::string test_iso = "/tmp/test.iso";
    
    // Create dummy ISO with install.wim
    createUdfIsoWithInstallWim(test_iso);

    EXPECT_EQ(iso.addIsoInfo(test_iso), "OK");
    ASSERT_FALSE(iso.getIsoInfo().empty());
    EXPECT_EQ(iso.getIsoInfo().back().type, "WINDOWS");
    EXPECT_EQ(iso.getIsoInfo().back().bootType, "Unknown / No Booteable");
    
    remove(test_iso.c_str());
}

TEST_F(IsoTest, IsWindowsInvalid) {
    std::string reason = depsMissingReason();
    if (!reason.empty()) {
        GTEST_SKIP() << reason;
    }
    // Create an ISO without the required files
    std::string invalid_iso = "/tmp/invalid_test.iso";
    createUdfIsoEmpty(invalid_iso);

    EXPECT_EQ(iso.addIsoInfo(invalid_iso), "ERROR");
    
    // Cleanup
    remove(invalid_iso.c_str());
}

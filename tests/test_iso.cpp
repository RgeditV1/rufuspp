#include <gtest/gtest.h>
#include "../src/extractor/iso.hpp"
#include <fstream>
#include <unistd.h>
#include <climits>
#include <filesystem>
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
        // Primero buscamos junto al binario de tests (copia de thirdparty via CMake)
        char exePath[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
        if (len > 0) {
            exePath[len] = '\0';
            std::filesystem::path local =
                std::filesystem::path(exePath).parent_path() / "7z.so";
            if (std::filesystem::exists(local))
                return true;
        }

        // Rutas estándar del sistema
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
            return "Missing 7z library (expected 7z.so next to test binary [copied from thirdparty/7zip/], "
                   "/usr/lib/7zip/7z.so, or /usr/lib/p7zip/7z.so)";
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

    try {
        createUdfIsoWithInstallWim(test_iso);
    } catch (const std::exception& e) {
        GTEST_SKIP() << "Could not create test ISO (env limitation): " << e.what();
    }

    EXPECT_EQ(iso.addIsoInfo(test_iso), true);
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
    std::string invalid_iso = "/tmp/invalid_test.iso";

    try {
        createUdfIsoEmpty(invalid_iso);
    } catch (const std::exception& e) {
        GTEST_SKIP() << "Could not create test ISO (env limitation): " << e.what();
    }

    EXPECT_EQ(iso.addIsoInfo(invalid_iso), false);

    remove(invalid_iso.c_str());
}

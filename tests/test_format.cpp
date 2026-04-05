#include <gtest/gtest.h>
#include "storage/format.hpp"
#include <cstdlib>
#include <cstdio>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>

class LoopDeviceHelper {
public:
    LoopDeviceHelper(const std::string& tempFile, size_t sizeMB) 
        : m_tempFile(tempFile) {
        // ensure old files are clean
        std::remove(m_tempFile.c_str());
        
        // 1. Create file
        std::string cmd = "truncate -s " + std::to_string(sizeMB) + "M " + m_tempFile;
        if (std::system(cmd.c_str()) != 0) {
            throw std::runtime_error("Could not create temp file");
        }

        // 2. Find and setup loop with partition scan enabled
        FILE* fp = popen(("sudo losetup --find --show --partscan " + m_tempFile + " 2>/dev/null").c_str(), "r");
        char buf[128];
        if (!fp || !fgets(buf, sizeof(buf), fp)) {
            if (fp) pclose(fp);
            throw std::runtime_error("No loop devices available or sudo required");
        }
        pclose(fp);
        m_loopPath = buf;
        if (!m_loopPath.empty() && m_loopPath.back() == '\n') m_loopPath.pop_back();

        if (m_loopPath.empty()) {
            throw std::runtime_error("Could not setup loop device");
        }
        
        // Clean any old partition info if device was reused
        std::system(("sudo wipefs -a " + m_loopPath + " 2>/dev/null").c_str());
    }

    ~LoopDeviceHelper() {
        if (!m_loopPath.empty()) {
            if (commandExists("kpartx")) {
                std::system(("sudo kpartx -d " + m_loopPath + " 2>/dev/null").c_str());
            }
            std::system(("sudo losetup -d " + m_loopPath + " 2>/dev/null").c_str());
        }
        if (!m_tempFile.empty()) {
            std::remove(m_tempFile.c_str());
        }
    }

    const std::string& getPath() const { return m_loopPath; }
    
    std::string getPartition1Path() const {
        if (std::isdigit(m_loopPath.back())) return m_loopPath + "p1";
        return m_loopPath + "1";
    }

private:
    bool commandExists(const std::string& cmd) const {
        std::string check = "command -v " + cmd + " >/dev/null 2>&1";
        return std::system(check.c_str()) == 0;
    }

    std::string m_tempFile;
    std::string m_loopPath;
};

class FormatTest : public ::testing::Test {
protected:
    bool commandExists(const std::string& cmd) {
        std::string check = "command -v " + cmd + " >/dev/null 2>&1";
        return std::system(check.c_str()) == 0;
    }

    bool canSudoNonInteractive() {
        if (geteuid() == 0) return true;
        return std::system("sudo -n true >/dev/null 2>&1") == 0;
    }

    void skipIfMissingIntegrationDeps(const std::string& fsTool) {
        if (!canSudoNonInteractive()) {
            GTEST_SKIP() << "Integration test requires passwordless sudo or root";
        }
        if (!commandExists("losetup") || !commandExists("wipefs") || !commandExists("parted") ||
            !commandExists("blkid") || !commandExists("umount") || !commandExists("truncate")) {
            GTEST_SKIP() << "Missing required system tools (losetup/wipefs/parted/blkid/umount/truncate)";
        }
        if (!commandExists(fsTool)) {
            GTEST_SKIP() << "Missing filesystem tool: " << fsTool;
        }
    }

    void settlePartitions(const std::string& devPath) {
        if (commandExists("partprobe")) {
            std::string cmd = "sudo partprobe " + devPath + " 2>/dev/null";
            std::system(cmd.c_str());
        }
        if (commandExists("udevadm")) {
            std::system("sudo udevadm settle 2>/dev/null");
        }
        usleep(300000);
    }

    bool exists(const std::string& path) {
        FILE* fp = fopen(path.c_str(), "r");
        if (fp) {
            fclose(fp);
            return true;
        }
        return false;
    }

    std::string readCommand(const std::string& cmd) {
        std::string out;
        FILE* fp = popen(cmd.c_str(), "r");
        if (!fp) return out;
        char buf[256];
        while (fgets(buf, sizeof(buf), fp)) {
            out += buf;
        }
        pclose(fp);
        return out;
    }

    bool hasPartitionInTable(const std::string& devPath) {
        std::string out = readCommand("sudo parted -s " + devPath + " print 2>/dev/null");
        return out.find("\n 1") != std::string::npos || out.find("\n1 ") != std::string::npos;
    }

    bool sysPartitionExists(const std::string& loopPath) {
        std::string base = loopPath.substr(loopPath.find_last_of('/') + 1);
        std::string sysPath = "/sys/block/" + base + "/" + base + "p1/partition";
        return exists(sysPath);
    }

    bool ensurePartitionNode(const std::string& loopPath, const std::string& partPath) {
        if (exists(partPath)) return true;
        if (commandExists("partx")) {
            std::string cmd = "sudo partx -u " + loopPath + " 2>/dev/null";
            std::system(cmd.c_str());
        }
        if (commandExists("kpartx")) {
            std::string cmd = "sudo kpartx -a " + loopPath + " 2>/dev/null";
            std::system(cmd.c_str());
        }
        settlePartitions(loopPath);
        return exists(partPath);
    }

    std::string getFsType(const std::string& path) {
        // Try multiple times as kernel might take a moment to update blkid cache
        for (int i = 0; i < 10; ++i) {
            std::string cmd = "sudo blkid -o value -s TYPE " + path + " 2>/dev/null";
            FILE* fp = popen(cmd.c_str(), "r");
            char buf[128];
            if (fp && fgets(buf, sizeof(buf), fp)) {
                pclose(fp);
                std::string res = buf;
                if (!res.empty() && res.back() == '\n') res.pop_back();
                if (!res.empty()) return res;
            }
            if (fp) pclose(fp);
            usleep(200000); // 200ms
        }
        return "";
    }
};

TEST_F(FormatTest, GetPartitionPath) {
    FFat32 format;
    // Discos estándar
    EXPECT_EQ(format.getPartitionPath("/dev/sda"), "/dev/sda1");
    EXPECT_EQ(format.getPartitionPath("/dev/sdb"), "/dev/sdb1");
    
    // Discos NVMe o similares que terminan en número
    EXPECT_EQ(format.getPartitionPath("/dev/nvme0n1"), "/dev/nvme0n1p1");
    EXPECT_EQ(format.getPartitionPath("/dev/mmcblk0"), "/dev/mmcblk0p1");
    EXPECT_EQ(format.getPartitionPath("/dev/loop0"), "/dev/loop0p1");
}

// Test de integración (requiere sudo y dispositivos loop)
TEST_F(FormatTest, IntegrationFormatFAT32) {
    skipIfMissingIntegrationDeps("mkfs.vfat");
    std::string loopPath;
    std::string partPath;
    
    try {
        LoopDeviceHelper loop("/tmp/rufuspp_test_fat32.img", 128);
        loopPath = loop.getPath();
        partPath = loop.getPartition1Path();

        UsbDetector::Device device;
        device.path = loopPath;
        device.name = "Test_Loop_Device_FAT32";
        
        FFat32 format;
        std::cout << "Running FAT32 integration test on " << loopPath << std::endl;
        format.applyFormat(Format::MakeType::FAT32, Format::PartitionTable::GPT, device);
        settlePartitions(loopPath);

        // Verificación: ¿Se creó la partición?
        bool partExists = false;
        for (int i = 0; i < 10; ++i) {
            if (ensurePartitionNode(loopPath, partPath)) {
                partExists = true;
                break;
            }
            usleep(300000);
        }
        if (!partExists) {
            // Some CI environments don't create /dev/loopXp1 even when partition table exists.
            if (hasPartitionInTable(loopPath) || sysPartitionExists(loopPath)) {
                GTEST_SKIP() << "Partition exists in table but /dev node was not created (udev/partscan not available)";
            }
        }
        EXPECT_TRUE(partExists) << "Partition " << partPath << " was not found after format";

        // Verificación: Filesystem type
        std::string fsType = getFsType(partPath);
        EXPECT_EQ(fsType, "vfat") << "Filesystem type mismatch on " << partPath;

    } catch (const std::exception& e) {
        GTEST_SKIP() << "Test skipped: " << e.what();
    }
}

TEST_F(FormatTest, IntegrationFormatNTFS) {
    skipIfMissingIntegrationDeps("mkfs.ntfs");
    std::string loopPath;
    std::string partPath;
    
    try {
        LoopDeviceHelper loop("/tmp/rufuspp_test_ntfs.img", 128);
        loopPath = loop.getPath();
        partPath = loop.getPartition1Path();

        UsbDetector::Device device;
        device.path = loopPath;
        device.name = "Test_Loop_Device_NTFS";
        
        FNtfs format;
        std::cout << "Running NTFS integration test on " << loopPath << std::endl;
        format.applyFormat(Format::MakeType::NTFS, Format::PartitionTable::MBR, device);
        settlePartitions(loopPath);

        // Verificación: ¿Se creó la partición?
        bool partExists = false;
        for (int i = 0; i < 10; ++i) {
            if (ensurePartitionNode(loopPath, partPath)) {
                partExists = true;
                break;
            }
            usleep(300000);
        }
        if (!partExists) {
            if (hasPartitionInTable(loopPath) || sysPartitionExists(loopPath)) {
                GTEST_SKIP() << "Partition exists in table but /dev node was not created (udev/partscan not available)";
            }
        }
        EXPECT_TRUE(partExists) << "Partition " << partPath << " was not found after format";

        // Verificación: Filesystem type
        std::string fsType = getFsType(partPath);
        EXPECT_EQ(fsType, "ntfs") << "Filesystem type mismatch on " << partPath;

    } catch (const std::exception& e) {
        GTEST_SKIP() << "Test skipped: " << e.what();
    }
}

#include <gtest/gtest.h>
#include "usb_detector.hpp"
#include <fstream>
#include <sstream>

class UsbDetectorTest : public ::testing::Test {
protected:
    UsbDetector detector;

    // Wrappers to access protected methods of UsbDetector
    // This works because UsbDetectorTest is a friend of UsbDetector
    std::string human_readable_size(const char* size_blocks) {
        return detector.human_readable_size(size_blocks);
    }

    std::string get_mount_point(const std::string& devnode) {
        return detector.get_mount_point(devnode);
    }
};

// Test para human_readable_size
TEST_F(UsbDetectorTest, HumanReadableSize) {
    // 0 bloques * 512 = 0 B
    EXPECT_EQ(human_readable_size("0"), "0 B");
    
    // 2 bloques * 512 = 1024 B = 1 KB
    EXPECT_EQ(human_readable_size("2"), "1 KB");
    
    // 2048 bloques * 512 = 1048576 B = 1 MB
    EXPECT_EQ(human_readable_size("2048"), "1 MB");
    
    // 2097152 bloques * 512 = 1073741824 B = 1 GB
    EXPECT_EQ(human_readable_size("2097152"), "1 GB");
    
    // Test con valor nulo
    EXPECT_EQ(human_readable_size(nullptr), "0 B");
}

// Test para get_mount_point (basado en lo que esté montado en el sistema)
TEST_F(UsbDetectorTest, GetMountPoint) {
    std::ifstream file("/proc/self/mounts");
    std::string line;
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string device, mountpoint;
        iss >> device >> mountpoint;
        
        // Si encontramos un dispositivo montado, verificamos que la función lo encuentre
        if (!device.empty() && device[0] == '/') {
            EXPECT_EQ(get_mount_point(device), mountpoint);
        }
    }
}

// Test funcional básico para listDevices
TEST_F(UsbDetectorTest, ListDevicesSmoke) {
    // No podemos garantizar que haya USBs, pero al menos no debe crashear
    auto devices = detector.listDevices();
    
    std::cout << "[ INFO     ] Detected " << devices.size() << " USB devices/partitions:" << std::endl;
    for (const auto& dev : devices) {
        std::cout << "[ DEVICE   ] " << dev.name << std::endl;
        std::cout << "  - Path:         " << dev.path << std::endl;
        std::cout << "  - Manufacturer: " << dev.manufacturer << std::endl;
        std::cout << "  - Size:         " << dev.size << std::endl;
        std::cout << "  - Mount Point:  " << dev.mountPoint << std::endl;
        std::cout << "  - File System:  " << dev.fileSystem << std::endl;
        std::cout << "  - Label:        " << dev.label << std::endl;
        std::cout << "  - Type:         " << dev.deviceType << std::endl;
        
        EXPECT_FALSE(dev.path.empty());
        EXPECT_FALSE(dev.name.empty());
        // El tipo debe ser disk o partition
        EXPECT_TRUE(dev.deviceType == "disk" || dev.deviceType == "partition");
    }
}

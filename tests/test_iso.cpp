#include <gtest/gtest.h>
#include "iso.hpp"
#include <fstream>

class IsoTest : public ::testing::Test {
protected:
    Iso iso;
};

TEST_F(IsoTest, ContainsFileValid) {
    std::string test_iso = "/tmp/test.iso";
    
    // El archivo debe haber sido creado por el script setup
    std::ifstream f(test_iso.c_str());
    if (!f.good()) {
        GTEST_SKIP() << "Test ISO file not found at " << test_iso;
    }

    EXPECT_TRUE(iso.containsFile(test_iso));
}

TEST_F(IsoTest, ContainsFileInvalid) {
    // Crear un ISO temporal sin el archivo buscado
    std::string invalid_iso = "/tmp/invalid_test.iso";
    system("mkdir -p /tmp/iso_invalid && mkisofs -o /tmp/invalid_test.iso /tmp/iso_invalid && rm -rf /tmp/iso_invalid");

    EXPECT_FALSE(iso.containsFile(invalid_iso));
    
    // Limpiar
    remove(invalid_iso.c_str());
}

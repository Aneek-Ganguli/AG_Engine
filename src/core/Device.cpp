#include "core/Device.hpp"

void AG_EngineV2::Core::Device::cleanUp() {
    device.destroy();
}

void AG_EngineV2::Core::Device::waitIdle() {
    device.waitIdle();
}

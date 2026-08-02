#include "Device.hpp"

void AG_EngineV2::Core::Device::cleanUp() {
    device.destroy();
}

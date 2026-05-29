#pragma once
#include "mc/ServerInstance.h"
#include "mc/Minecraft.h"

namespace ll {
namespace service {
    mc::ServerInstance* getServerInstance();
    mc::Minecraft* getMinecraft();
}
}

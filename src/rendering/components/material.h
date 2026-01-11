#pragma once

#include <asset_store.h>
#include <ecs/component.h>
#include <material_buffer.h>

class Material : ComponentBase {
  AssetHandle<MaterialBuffer> getHandle() const { return handle; }

 private:
  AssetHandle<MaterialBuffer> handle;
};
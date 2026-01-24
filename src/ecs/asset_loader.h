#pragma once

#include "render_context.h"
#include "uploader.h"
#include <ecs/asset_handle.h>
#include <ecs/asset_traits.h>

#include <string>

template <typename T> class AssetLoader {
public:
  typename AssetTraits<T>::CPUDataType loadCPU(const std::string& path) const;
  T loadGPU(const AssetTraits<T>::CPUDataType& CPUData,
            const RenderContext& context, const Uploader& uploader) const
    requires AssetTraits<T>::IsGpuAsset;
};
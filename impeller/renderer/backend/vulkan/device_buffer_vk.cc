// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/device_buffer_vk.h"

#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "vulkan/vulkan_handles.hpp"

namespace impeller {

DeviceBufferVK::DeviceBufferVK(DeviceBufferDescriptor desc,
                               std::weak_ptr<Context> context,
                               VmaAllocator allocator,
                               VmaAllocation allocation,
                               VmaAllocationInfo info,
                               vk::Buffer buffer)
    : DeviceBuffer(desc),
      context_(std::move(context)),
      resource_(ContextVK::Cast(*context_.lock().get()).GetResourceManager(),
                BufferResource{
                    allocator,   //
                    allocation,  //
                    info,        //
                    buffer       //
                }) {}

DeviceBufferVK::~DeviceBufferVK() {}

uint8_t* DeviceBufferVK::OnGetContents() const {
  return static_cast<uint8_t*>(resource_->info.pMappedData);
}

bool DeviceBufferVK::OnCopyHostBuffer(const uint8_t* source,
                                      Range source_range,
                                      size_t offset) {
  TRACE_EVENT0("impeller", "CopyToDeviceBuffer");
  uint8_t* dest = OnGetContents();

  if (!dest) {
    return false;
  }

  if (source) {
    ::memmove(dest + offset, source + source_range.offset, source_range.length);
  }

  return true;
}

bool DeviceBufferVK::SetLabel(const std::string& label) {
  auto context = context_.lock();
  if (!context || !resource_->buffer) {
    // The context could have died at this point.
    return false;
  }

  ::vmaSetAllocationName(resource_->allocator,   //
                         resource_->allocation,  //
                         label.c_str()           //
  );

  return ContextVK::Cast(*context).SetDebugName(resource_->buffer, label);
}

bool DeviceBufferVK::SetLabel(const std::string& label, Range range) {
  // We do not have the ability to name ranges. Just name the whole thing.
  return SetLabel(label);
}

vk::Buffer DeviceBufferVK::GetBuffer() const {
  return resource_->buffer;
}

}  // namespace impeller

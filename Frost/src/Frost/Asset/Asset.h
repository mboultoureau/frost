#pragma once

#include <string>
#include <atomic>

namespace Frost
{
    enum class AssetStatus
    {
        Unloaded,
        Loading,
        Loaded,
        Failed
    };

    class Asset
    {
    public:
        using Path = std::string;
        using DebugName = std::string;

        virtual ~Asset() = default;

        const AssetStatus GetStatus() const { return _status.load(std::memory_order_acquire); }
        bool IsLoaded() const { return _status == AssetStatus::Loaded; }
        void SetStatus(AssetStatus status) { _status.store(status, std::memory_order_release); }

    protected:
        std::atomic<AssetStatus> _status{ AssetStatus::Unloaded };

        friend class AssetManager;
    };
} // namespace Frost

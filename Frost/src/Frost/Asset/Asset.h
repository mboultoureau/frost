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
        Failed,
        Cancelled
    };

    class Asset
    {
    public:
        using Path = std::string;
        using DebugName = std::string;

        virtual ~Asset() = default;

        const AssetStatus GetStatus() const { return _status.load(std::memory_order_acquire); }
        bool IsLoaded() const { return _status == AssetStatus::Loaded; }
        bool IsCancelled() const { return _status.load(std::memory_order_acquire) == AssetStatus::Cancelled; }
        void SetStatus(AssetStatus status) { _status.store(status, std::memory_order_release); }

        virtual void UploadGPU() {}
        virtual bool UploadGPUChunk()
        {
            UploadGPU();
            return false;
        }
        virtual void Cancel() { SetStatus(AssetStatus::Cancelled); }

        void IncrementInternalRef() { _internalRefCount.fetch_add(1, std::memory_order_relaxed); }
        void DecrementInternalRef() { _internalRefCount.fetch_sub(1, std::memory_order_relaxed); }
        int GetInternalRefCount() const { return _internalRefCount.load(std::memory_order_relaxed); }

    protected:
        std::atomic<AssetStatus> _status{ AssetStatus::Unloaded };
        std::atomic<int> _internalRefCount{ 0 };

        friend class AssetManager;
    };

    class AsyncJobGuard
    {
    public:
        explicit AsyncJobGuard(Asset& asset) : _asset(asset) { _asset.IncrementInternalRef(); }
        ~AsyncJobGuard() { _asset.DecrementInternalRef(); }

    private:
        Asset& _asset;
    };
} // namespace Frost

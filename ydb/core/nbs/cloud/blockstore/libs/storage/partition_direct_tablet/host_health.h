#pragma once

#include <ydb/core/nbs/cloud/blockstore/libs/storage/partition_direct/model/host.h>
#include <ydb/core/nbs/cloud/blockstore/libs/storage/partition_direct/protos/direct_block_group_health.pb.h>

namespace NYdb::NBS::NBlockStore::NStorage::NPartitionDirect {

using EPersistentHostHealth = PartitionDirect::NProto::EPersistentHostHealth;

namespace {

[[maybe_unused]] EHostHealth HostHealthFromPersistent(
    const EPersistentHostHealth persistentHostHealth)
{
    switch (persistentHostHealth) {
        case EPersistentHostHealth::Online:
            return EHostHealth::Online;
        case EPersistentHostHealth::TemporaryOffline:
            return EHostHealth::TemporaryOffline;
        case EPersistentHostHealth::Offline:
            return EHostHealth::Offline;
        case EPersistentHostHealth::Broken:
            return EHostHealth::Broken;
        case EPersistentHostHealth::Removed:
            return EHostHealth::Removed;
        default:
            Y_ABORT(
                "Invalid persistent host health = %d",
                persistentHostHealth);
    }
}

[[maybe_unused]] EPersistentHostHealth HostHealthToPersistent(
    const EHostHealth hostHealth)
{
    switch (hostHealth) {
        case EHostHealth::Online:
            return EPersistentHostHealth::Online;
        case EHostHealth::Sufferer:
            return EPersistentHostHealth::Online;
        case EHostHealth::TemporaryOffline:
            return EPersistentHostHealth::TemporaryOffline;
        case EHostHealth::Offline:
            return EPersistentHostHealth::Offline;
        case EHostHealth::Broken:
            return EPersistentHostHealth::Broken;
        case EHostHealth::Removed:
            return EPersistentHostHealth::Removed;
    }
}

}   // namespace

}   // namespace NYdb::NBS::NBlockStore::NStorage::NPartitionDirect

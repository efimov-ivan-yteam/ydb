#include "partition_direct_actor.h"

#include <ydb/core/nbs/cloud/blockstore/libs/common/constants.h>
#include <ydb/core/nbs/cloud/blockstore/libs/storage/partition_direct/fast_path_service.h>
#include <ydb/core/nbs/cloud/blockstore/libs/storage/partition_direct_tablet/host_health.h>
#include <ydb/core/nbs/cloud/blockstore/libs/storage/partition_direct_tablet/part_database.h>

#include <ydb/core/nbs/cloud/storage/core/libs/common/error.h>

#include <ydb/library/actors/core/log.h>
#include <ydb/library/services/services.pb.h>

namespace NYdb::NBS::NBlockStore::NStorage::NPartitionDirect {

using namespace NActors;
using namespace NKikimr;
using namespace NKikimr::NTabletFlatExecutor;

////////////////////////////////////////////////////////////////////////////////

void TPartitionActor::HandlePersistHostHealth(
    const TEvPartitionDirectPrivate::TEvPersistHostHealth::TPtr& ev,
    const NActors::TActorContext& ctx)
{
    Y_ABORT_UNLESS(FastPathService);

    const auto* msg = ev->Get();

    const auto oldPersistentHealth = HostHealthToPersistent(msg->OldHealth);
    const auto newPersistentHealth = HostHealthToPersistent(msg->NewHealth);

    if (oldPersistentHealth == newPersistentHealth) {
        LOG_DEBUG(
            ctx,
            NKikimrServices::NBS_PARTITION,
            "%s PersistHostHealth ignoring health change for %s/%s from %s to "
            "%s: same persistent health value (%s)",
            LogTitle.GetWithTime().c_str(),
            PrintDbgId(msg->DirectBlockGroupId).c_str(),
            PrintHostIndex(msg->HostIndex).c_str(),
            ToString(msg->OldHealth).c_str(),
            ToString(msg->NewHealth).c_str(),
            ToString(newPersistentHealth).c_str());

        return;
    }

    LOG_INFO(
        ctx,
        NKikimrServices::NBS_PARTITION,
        "%s PersistHostHealth for %s/%s "
        "from %s (pers = %s) to %s (pers = %s)",
        LogTitle.GetWithTime().c_str(),
        PrintDbgId(msg->DirectBlockGroupId).c_str(),
        PrintHostIndex(msg->HostIndex).c_str(),
        ToString(msg->OldHealth).c_str(),
        ToString(oldPersistentHealth).c_str(),
        ToString(msg->NewHealth).c_str(),
        ToString(newPersistentHealth).c_str());

    ExecuteTx(
        ctx,
        CreateTx<TPersistHostHealth>(
            msg->DirectBlockGroupId,
            msg->HostIndex,
            newPersistentHealth));
}

////////////////////////////////////////////////////////////////////////////////

bool TPartitionActor::PreparePersistHostHealth(
    const NActors::TActorContext& ctx,
    NKikimr::NTabletFlatExecutor::TTransactionContext& tx,
    TTxPartition::TPersistHostHealth& args)
{
    Y_UNUSED(ctx);

    TPartitionDatabase db(tx.DB);

    return db.ReadHostHealthRevision(args.Revision) &&
           db.ReadDirectBlockGroupHealth(args.DirectBlockGroupId, args.Health);
}

void TPartitionActor::ExecutePersistHostHealth(
    const NActors::TActorContext& ctx,
    NKikimr::NTabletFlatExecutor::TTransactionContext& tx,
    TTxPartition::TPersistHostHealth& args)
{
    Y_UNUSED(ctx);

    TPartitionDatabase db(tx.DB);

    Y_ABORT_UNLESS(args.Revision.Defined());
    Y_ABORT_UNLESS(args.Health.Defined());

    *args.Revision += 1;

    args.Health->MutableHosts(args.HostId)->SetHealth(args.NewHealth);

    db.StoreHostHealthRevision(*args.Revision);
    db.StoreDirectBlockGroupHealth(args.DirectBlockGroupId, *args.Health);
    db.StoreNeedToNotifyDBSC(true);
}

void TPartitionActor::CompletePersistHostHealth(
    const NActors::TActorContext& ctx,
    TTxPartition::TPersistHostHealth& args)
{
    LOG_INFO(
        ctx,
        NKikimrServices::NBS_PARTITION,
        "%s PersistHostHealth persisted new health %s for %s/%s, new revision "
        "= %lu",
        LogTitle.GetWithTime().c_str(),
        ToString(args.NewHealth).c_str(),
        PrintDbgId(args.DirectBlockGroupId).c_str(),
        PrintHostIndex(args.HostId).c_str(),
        args.Revision);

    HostHealthRevision = *args.Revision;
    NeedToNotifyDBSC = true;
    DirectBlockGroupHealth[args.DirectBlockGroupId] = *args.Health;
}

}   // namespace NYdb::NBS::NBlockStore::NStorage::NPartitionDirect

#include "part_database.h"
#include "partition_direct_actor.h"

namespace NYdb::NBS::NBlockStore::NStorage::NPartitionDirect {

using namespace NActors;
using namespace NKikimr;
using namespace NKikimr::NTabletFlatExecutor;

////////////////////////////////////////////////////////////////////////////////

namespace {
constexpr ui64 InitialHostHealthRevision = 1;
}

////////////////////////////////////////////////////////////////////////////////

bool TPartitionActor::PrepareStorePartitionIds(
    const TActorContext& ctx,
    TTransactionContext& tx,
    TTxPartition::TStorePartitionIds& args)
{
    Y_UNUSED(ctx);
    Y_UNUSED(tx);
    Y_UNUSED(args);

    for (ui64 dbgId = 0;
         dbgId <
         args.DirectBlockGroupsConnections.DirectBlockGroupConnectionsSize();
         ++dbgId)
    {
        auto& health = args.DirectBlockGroupHealth[dbgId];
        for (ui64 hostId = 0;
             hostId < args.DirectBlockGroupsConnections
                          .GetDirectBlockGroupConnections(dbgId)
                          .ConnectionsSize();
             ++hostId)
        {
            health.AddHosts()->SetHealth(EPersistentHostHealth::Online);
        }
    }

    return true;
}

void TPartitionActor::ExecuteStorePartitionIds(
    const TActorContext& ctx,
    TTransactionContext& tx,
    TTxPartition::TStorePartitionIds& args)
{
    Y_UNUSED(ctx);

    TPartitionDatabase db(tx.DB);
    db.StoreDirectBlockGroupsConnections(args.DirectBlockGroupsConnections);

    for (const auto& [dbgId, health]: args.DirectBlockGroupHealth) {
        db.StoreDirectBlockGroupHealth(dbgId, health);
    }

    for (ui64 dbgId = 0;
         dbgId <
         args.DirectBlockGroupsConnections.DirectBlockGroupConnectionsSize();
         ++dbgId)
    {
        TDirectBlockGroupHealthProto health;
        for (ui64 hostId = 0;
             hostId < args.DirectBlockGroupsConnections
                          .GetDirectBlockGroupConnections(dbgId)
                          .ConnectionsSize();
             ++hostId)
        {
            health.AddHosts()->SetHealth(EPersistentHostHealth::Online);
        }
    }

    db.StoreHostHealthRevision(InitialHostHealthRevision);
    db.StoreNeedToNotifyDBSC(true);
}

void TPartitionActor::CompleteStorePartitionIds(
    const TActorContext& ctx,
    TTxPartition::TStorePartitionIds& args)
{
    // No persisted vchunk configs at first allocation: vchunks fall back to
    // TVChunkConfig::Make().

    HostHealthRevision = InitialHostHealthRevision;
    NeedToNotifyDBSC = true;

    Start(
        ctx,
        args.DirectBlockGroupsConnections,
        {},   // vChunkConfigs
        {},   // dirtyMapStates
        args.DirectBlockGroupHealth);
}

////////////////////////////////////////////////////////////////////////////////

}   // namespace NYdb::NBS::NBlockStore::NStorage::NPartitionDirect

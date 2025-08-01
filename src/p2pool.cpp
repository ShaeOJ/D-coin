
#include <chainparams.h>
#include <consensus/merkle.h>
#include <net.h>
#include <pow.h>
#include <validation.h>

// The share chain.
std::vector<CShare> share_chain;

// The P2P network.
std::vector<CNode*> p2p_nodes;

// The miner.
CMiner miner;

// Connect to the P2P network.
void CMiner::Connect()
{
    // For now, we don't need to do anything here.
}

// Disconnect from the P2P network.
void CMiner::Disconnect()
{
    // For now, we don't need to do anything here.
}

// Mine a new share.
CShare CMiner::Mine(const CP2PWork& work)
{
    CShare share;
    share.prev_share_hash = share_chain.empty() ? Params().GetConsensus().p2pool_genesis_hash : share_chain.back().GetHash();
    share.merkle_root = BlockMerkleRoot(work.block);
    share.timestamp = GetTime();
    share.bits = work.block.nBits;

    // Search for a valid nonce.
    arith_uint256 share_target_arith;
    share_target_arith.SetCompact(work.share_target.GetCompact());
    while (true) {
        share.nonce++;
        if (UintToArith256(share.GetHash()) <= share_target_arith) {
            return share;
        }
    }
}

// Check if a block is mined via the P2Pool protocol.
bool CheckP2Pool(const CBlock& block, BlockValidationState& state)
{
    // Check that the coinbase transaction contains a valid share.
    if (block.vtx.empty() || !block.vtx[0]->IsCoinBase()) {
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-cb-missing", "first tx is not coinbase");
    }
    const CTransaction& coinbase_tx = *block.vtx[0];
    if (coinbase_tx.vin.empty() || coinbase_tx.vin[0].scriptSig.size() < CShare::MIN_SERIALIZE_SIZE) {
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-cb-length", "coinbase scriptSig too short");
    }
    CDataStream ss(coinbase_tx.vin[0].scriptSig.begin(), coinbase_tx.vin[0].scriptSig.end(), SER_NETWORK, PROTOCOL_VERSION);
    CShare share;
    try {
        ss >> share;
    } catch (const std::exception& e) {
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-cb-share", "invalid share in coinbase");
    }

    // Check that the share is valid.
    if (share.GetHash() != block.hashMerkleRoot) {
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-cb-share-hash", "share hash does not match merkle root");
    }
    if (share.prev_share_hash != (share_chain.empty() ? Params().GetConsensus().p2pool_genesis_hash : share_chain.back().GetHash())) {
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-cb-prev-share-hash", "previous share hash does not match");
    }
    if (share.timestamp > GetTime() + 2 * 60 * 60) {
        return state.Invalid(BlockValidationResult::BLOCK_CONSENSUS, "bad-cb-share-time", "share timestamp too far in the future");
    }

    // Add the share to the share chain.
    share_chain.push_back(share);

    return true;
}

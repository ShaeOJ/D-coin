
#ifndef BITCOIN_P2POOL_H
#define BITCOIN_P2POOL_H

#include <primitives/block.h>
#include <primitives/transaction.h>
#include <uint256.h>

// The share chain is a linked list of shares.
class CShare
{
public:
    uint256 prev_share_hash;
    uint256 merkle_root;
    uint32_t timestamp;
    uint32_t bits;
    uint32_t nonce;

    CShare()
    {
        SetNull();
    }

    void SetNull()
    {
        prev_share_hash.SetNull();
        merkle_root.SetNull();
        timestamp = 0;
        bits = 0;
        nonce = 0;
    }

    uint256 GetHash() const;
};

// A P2P message for sharing work between nodes.
class CP2PWork
{
public:
    CBlock block;
    uint256 share_target;

    CP2PWork()
    {
        SetNull();
    }

    void SetNull()
    {
        block.SetNull();
        share_target.SetNull();
    }
};

// A miner connects to the P2P network and mines shares.
class CMiner
{
public:
    // Connect to the P2P network.
    void Connect();

    // Disconnect from the P2P network.
    void Disconnect();

    // Mine a new share.
    CShare Mine(const CP2PWork& work);
};

#endif // BITCOIN_P2POOL_H

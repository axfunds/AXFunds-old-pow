// Copyright (c) 2014-2014 The AXFunds developers
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of

        (  0,  uint256("0xd1d5329c28dfd4ee8907ca15584f8245badb7438c4004f5f977143a4fb1a61b9"))
		(  99, uint256("0xc5a49f1ebf31d73dc97d9eba9b5d0e82a25af5beb147220c1397c95b081342b7"))
		(  12345, uint256("0x981a7af4c51f1e6d7fca6b999f9dfc036a79a7fb3981caad39b05fbdb66b0dbd"))
		(  22345, uint256("0x7589086d1f848b6d6b8f8d49204cb9d6b32cceb49675d87e15b35f73a50f8969"))
		(  32345, uint256("0xb165d831f45e13dfe93ba1e7c74bdbbad88dd43eb1b52fcb29cc1fecf95c7057"))
		(  42345, uint256("0x34981cea78ad2a40e01f78181aaf802d751ff48daaa9b151b3c6570b906b2671"))
		(  52345, uint256("0x93f1c96f8e5f526a6e3b15a55e6f3f0ca054b32c79d38144d5334e6624012f5d"))
		(  62345, uint256("0xa13be3a6f2b6a810b4bcc7aa4ec7221f72713e30cc402ecd5123d1f66d70a95b"))
		(  72345, uint256("0xde0b6fdbe44ae3094d4b8a0f6e2353088286a5a9105f1784cd373998056d2a6a"))
		(  82345, uint256("0x62b501431dae787879b007105df825845ebf76c891255f2edf0518b8dbc8ed15"))
		(  97384, uint256("0xa2ec525dd1cb5bde9d1b6c4ab394f2ecba85a55098393f4973cbe8a72317e688"))
		(  97385, uint256("0x0ea11fc1a781b71906b64ccb9fea4954c0223626831334ed8e1af0d8306cbf33"))
		(  97765, uint256("0x3fc29748309707aff625f9195b57d5f6483ffa5113618c24bbc42d1c28a17b61"))
		 

        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1424789024 , // * UNIX timestamp of last checkpoint block
        98062,    // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        1500.0     // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of

        (   0, uint256("0xb44cc80bfa2a5629c618d5a3c07f2400462d781c1a289379576dbb4fc29618c5"))
       
	   ;

    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1365458829,
        547,
        576
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}

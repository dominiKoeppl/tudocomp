###############################################################################
#
# This script determines which algorithms will be available in the tdc binary.
#
# Please refer to the documentation ("Registering Algorithms" section) for
# more information.
#
###############################################################################

##### Coders #####

# Universal coders
universal_coders = [
    AlgorithmConfig(name="ASCIICoder", header="coders/ASCIICoder.hpp"),
    # AlgorithmConfig(name="BitCoder", header="coders/BitCoder.hpp"),
    # AlgorithmConfig(name="EliasGammaCoder", header="coders/EliasGammaCoder.hpp"),
    # AlgorithmConfig(name="EliasDeltaCoder", header="coders/EliasDeltaCoder.hpp"),
    # AlgorithmConfig(name="SLEIntCoder", header="coders/SLEIntCoder.hpp"),
]

# Entropy coders
entropy_coders = [
    # AlgorithmConfig(name="SigmaCoder", header="coders/SigmaCoder.hpp"),
    # AlgorithmConfig(name="HuffmanCoder", header="coders/HuffmanCoder.hpp"),
]

# Entropy coders that may consume characters without immediately generating an
# output (not suitable for all scenarios, see "Interleaved Coding" section in
# the documentation)
consuming_entropy_coders = [
    # AlgorithmConfig(name="ArithmeticCoder", header="coders/ArithmeticCoder.hpp"),
    # AlgorithmConfig(name="SLEKmerCoder", header="coders/SLEKmerCoder.hpp"),
]

# All non-consuming coders
non_consuming_coders = universal_coders + entropy_coders

# All coders
all_coders = universal_coders + entropy_coders + consuming_entropy_coders

##### LZSS Coding Strategies #####
lzss_streaming_coders = [
    # AlgorithmConfig(name="lzss::DidacticalCoder", header="compressors/lzss/DidacticalCoder.hpp"),
    AlgorithmConfig(name="lzss::StreamingCoder", header="compressors/lzss/StreamingCoder.hpp",
        sub=[universal_coders,universal_coders,universal_coders]),
]

lzss_coders = lzss_streaming_coders + [
    AlgorithmConfig(name="lzss::BufferedLeftCoder", header="compressors/lzss/BufferedLeftCoder.hpp",
        sub=[universal_coders,universal_coders,all_coders]),
    AlgorithmConfig(name="lzss::BufferedBidirectionalCoder", header="compressors/lzss/BufferedBidirectionalCoder.hpp",
        sub=[universal_coders,universal_coders,all_coders]),
]

##### Text data structures #####

# Suffix Array
sa = [
    AlgorithmConfig(name="SADivSufSort", header="ds/SADivSufSort.hpp"),
]

# Phi Array
phi = [
    AlgorithmConfig(name="PhiFromSA", header="ds/PhiFromSA.hpp"),
]

# PLCP Array
plcp = [
    AlgorithmConfig(name="PLCPFromPhi", header="ds/PLCPFromPhi.hpp"),
]

# Uncompressed LCP Array
lcp_uncompressed = [
    AlgorithmConfig(name="LCPFromPLCP", header="ds/LCPFromPLCP.hpp"),
]

# All LCP Arrays
lcp = lcp_uncompressed + [
    AlgorithmConfig(name="CompressedLCP", header="ds/CompressedLCP.hpp", sub=[sa]),
]

# Inverse Suffix Array
isa = [
    AlgorithmConfig(name="ISAFromSA", header="ds/ISAFromSA.hpp"),
    AlgorithmConfig(name="SparseISA", header="ds/SparseISA.hpp", sub=[sa]),
]

# TextDS
textds = [
    AlgorithmConfig(name="TextDS", header="ds/TextDS.hpp", sub=[sa, phi, plcp, lcp, isa]),
]

##### lz78 #####

# LZ78 trie hash map size managers
hash_manager = [
        AlgorithmConfig(name="SizeManagerPow2", header="util/Hash.hpp"),
        AlgorithmConfig(name="SizeManagerPrime", header="util/Hash.hpp"),
        AlgorithmConfig(name="SizeManagerDirect", header="util/Hash.hpp"),
        ]
# LZ78 trie hash probers
hash_prober = [
        AlgorithmConfig(name="LinearProber", header="util/Hash.hpp"),
        ]
# LZ78 trie rolling hash functions
hash_roll = [
        AlgorithmConfig(name="WordpackRollingHash", header="util/Hash.hpp"),
        # AlgorithmConfig(name="ZBackupRollingHash", header="util/Hash.hpp"),
        # AlgorithmConfig(name="CyclicHash", header="util/hash/cyclichash.h"),
        AlgorithmConfig(name="KarpRabinHash", header="util/hash/rabinkarphash.h"),
        # AlgorithmConfig(name="ThreeWiseHash", header="util/hash/threewisehash.h"),
        ]
# LZ78 trie hash functions
hash_function = [
        AlgorithmConfig(name="NoopHasher", header="util/Hash.hpp"),
        AlgorithmConfig(name="MixHasher", header="util/Hash.hpp"),
        AlgorithmConfig(name="VignaHasher", header="util/Hash.hpp"),
        AlgorithmConfig(name="KnuthHasher", header="util/Hash.hpp"),
        # AlgorithmConfig(name="Zobrist", header="util/hash/zobrist.h"),
        # AlgorithmConfig(name="CLHash", header="util/hash/clhash.h"),
        ]

# CompactHashTrie strategies
compact_hashmap_strategies = [
    AlgorithmConfig(name="lz78::Sparse", header="compressors/lz78/CompactHashTrie.hpp"),
    AlgorithmConfig(name="lz78::Plain", header="compressors/lz78/CompactHashTrie.hpp"),
    AlgorithmConfig(name="lz78::SparseDisplacement", header="compressors/lz78/CompactHashTrie.hpp"),
    AlgorithmConfig(name="lz78::PlainDisplacement", header="compressors/lz78/CompactHashTrie.hpp"),
    AlgorithmConfig(name="lz78::SparseEliasDisplacement", header="compressors/lz78/CompactHashTrie.hpp"),
    AlgorithmConfig(name="lz78::PlainEliasDisplacement", header="compressors/lz78/CompactHashTrie.hpp"),
]

# LZ78 tries ("lz78trie")
lz78_trie = [
    # AlgorithmConfig(name="lz78::BinarySortedTrie", header="compressors/lz78/BinarySortedTrie.hpp"),
    # AlgorithmConfig(name="lz78::BinaryTrie", header="compressors/lz78/BinaryTrie.hpp"),
    # AlgorithmConfig(name="lz78::CedarTrie", header="compressors/lz78/CedarTrie.hpp"),
    # AlgorithmConfig(name="lz78::ExtHashTrie", header="compressors/lz78/ExtHashTrie.hpp"),
    # AlgorithmConfig(name="lz78::HashTrie", header="compressors/lz78/HashTrie.hpp", sub=[hash_function,hash_prober,hash_manager]),
    # AlgorithmConfig(name="lz78::HashTriePlus", header="compressors/lz78/HashTriePlus.hpp", sub=[hash_function,hash_manager]),
    # AlgorithmConfig(name="lz78::RollingTrie", header="compressors/lz78/RollingTrie.hpp", sub=[hash_roll, hash_prober,hash_manager,hash_function]),
    # AlgorithmConfig(name="lz78::RollingTriePlus", header="compressors/lz78/RollingTriePlus.hpp", sub=[hash_roll, hash_manager,hash_function]),
    AlgorithmConfig(name="lz78::TernaryTrie", header="compressors/lz78/TernaryTrie.hpp"),
    # AlgorithmConfig(name="lz78::CompactHashTrie", header="compressors/lz78/CompactHashTrie.hpp", sub=[compact_hashmap_strategies]),
]

if config_match("^#define JUDY_H_AVAILABLE 1"): # if the Judy trie is available
    lz78_trie += [
        AlgorithmConfig(name="lz78::JudyTrie", header="compressors/lz78/JudyTrie.hpp"),
]

##### lz78u #####

# LZ78U factorization strategies ("comp")
lz78u_comp = [
    AlgorithmConfig(name="lz78u::StreamingStrategy", header="compressors/lz78u/StreamingStrategy.hpp", sub=[universal_coders]),
    AlgorithmConfig(name="lz78u::BufferingStrategy", header="compressors/lz78u/BufferingStrategy.hpp", sub=[non_consuming_coders]),
    ]

##### lcpcomp #####

# lcpcomp factorization strategies ("comp")
lcpcomp_comp = [
    AlgorithmConfig(name="lcpcomp::MaxHeapStrategy", header="compressors/lcpcomp/compress/MaxHeapStrategy.hpp"),
    AlgorithmConfig(name="lcpcomp::MaxHeapLeftOnlyStrategy", header="compressors/lcpcomp/compress/MaxHeapLeftOnlyStrategy.hpp"),
    AlgorithmConfig(name="lcpcomp::MaxLCPStrategy", header="compressors/lcpcomp/compress/MaxLCPStrategy.hpp"),
    AlgorithmConfig(name="lcpcomp::ArraysComp", header="compressors/lcpcomp/compress/ArraysComp.hpp"),
]

if config_match("^#define Boost_FOUND 1"): # if Boost is available
    lcpcomp_comp += [
        AlgorithmConfig(name="lcpcomp::BoostHeap", header="compressors/lcpcomp/compress/BoostHeap.hpp"),
        AlgorithmConfig(name="lcpcomp::PLCPStrategy", header="compressors/lcpcomp/compress/PLCPStrategy.hpp")
]

# lcpcomp factor decoding strategies ("dec")
lcpcomp_dec = [
    AlgorithmConfig(name="lcpcomp::ScanDec", header="compressors/lcpcomp/decompress/ScanDec.hpp"),
    AlgorithmConfig(name="lcpcomp::DecodeForwardQueueListBuffer", header="compressors/lcpcomp/decompress/DecodeQueueListBuffer.hpp"),
    AlgorithmConfig(name="lcpcomp::CompactDec", header="compressors/lcpcomp/decompress/CompactDec.hpp"),
    AlgorithmConfig(name="lcpcomp::MultimapBuffer", header="compressors/lcpcomp/decompress/MultiMapBuffer.hpp"),
    AlgorithmConfig(name="lcpcomp::LeftDec", header="compressors/lcpcomp/decompress/LeftDec.hpp"),
]

# Allowed TextDS instances for lcpcomp (LCP array must be writable!)
lcpcomp_textds = [
    AlgorithmConfig(name="TextDS", header="ds/TextDS.hpp", sub=[sa, phi, plcp, lcp_uncompressed, isa]),
]

##### ESP grammar compressor WIP #####
ipd = [
    AlgorithmConfig(name="esp::StdUnorderedMapIPD", header="compressors/esp/StdUnorderedMapIPD.hpp"),
    AlgorithmConfig(name="esp::HashMapIPD", header="compressors/esp/HashMapIPD.hpp"),
]

ipddyn = ipd + [
    AlgorithmConfig(name="esp::DynamicSizeIPD", header="compressors/esp/DynamicSizeIPD.hpp", sub=[ipd]),
]

slp_d_coder_2 = [
    AlgorithmConfig(name="esp::DPlain", header="compressors/esp/DRCoder.hpp"),
    AlgorithmConfig(name="esp::DHuffman", header="compressors/esp/DRCoder.hpp"),
    AlgorithmConfig(name="esp::DWaveletTree", header="compressors/esp/DRCoder.hpp"),
]

subseq = [
    AlgorithmConfig(name="esp::SubSeqOptimal", header="compressors/esp/SubseqStrategy.hpp"),
    AlgorithmConfig(name="esp::SubSeqGreedy", header="compressors/esp/SubseqStrategy.hpp"),
]

slp_d_coder = [
    AlgorithmConfig(name="esp::DPlain", header="compressors/esp/DRCoder.hpp"),
    AlgorithmConfig(name="esp::DHuffman", header="compressors/esp/DRCoder.hpp"),
    AlgorithmConfig(name="esp::DMonotonSubseq", header="compressors/esp/DRCoder.hpp", sub=[subseq, slp_d_coder_2]),
    AlgorithmConfig(name="esp::DDiff", header="compressors/esp/DRCoder.hpp"),
    AlgorithmConfig(name="esp::DRangeFit", header="compressors/esp/DRCoder.hpp"),
]

slp_coder = [
    AlgorithmConfig(name="esp::PlainSLPCoder", header="compressors/esp/PlainSLPCoder.hpp"),
    AlgorithmConfig(name="esp::SortedSLPCoder", header="compressors/esp/SortedSLPCoder.hpp", sub=[slp_d_coder]),
]

##### lfs WIP #####

lfs_strat = [
    AlgorithmConfig(name="lfs::ESAStrategy<>", header="compressors/lfs/ESAStrategy.hpp"),
    AlgorithmConfig(name="lfs::STStrategy", header="compressors/lfs/STStrategy.hpp"),
    AlgorithmConfig(name="lfs::BSTStrategy", header="compressors/lfs/BSTStrategy.hpp"),
    AlgorithmConfig(name="lfs::SimSTStrategy", header="compressors/lfs/SimSTStrategy.hpp"),
]

lit_coder = [
    AlgorithmConfig(name="BitCoder", header="coders/BitCoder.hpp"),
    AlgorithmConfig(name="ASCIICoder", header="coders/ASCIICoder.hpp"),
    AlgorithmConfig(name="HuffmanCoder", header="coders/HuffmanCoder.hpp"),
]

len_coder = universal_coders

coding_strat = [
	AlgorithmConfig(name="lfs::EncodeStrategy", header="compressors/lfs/EncodeStrategy.hpp", sub=[lit_coder, len_coder]),
]

dividing_strat = [
    AlgorithmConfig(name="DivisionDividingStrategy", header="compressors/DividingCompressor.hpp"),
    AlgorithmConfig(name="BlockedDividingStrategy", header="compressors/DividingCompressor.hpp"),
]

long_common_strat = [
    AlgorithmConfig(name="EscapingSparseFactorCoder", header="compressors/LongCommonStringCompressor.hpp"),
]

##### Export available compressors #####
tdc.compressors = [
    AlgorithmConfig(name="LZ78CicsCompressor", header="compressors/LZ78CicsCompressor.hpp", sub=[universal_coders]),
    AlgorithmConfig(name="LZSSCicsCompressor", header="compressors/LZSSCicsCompressor.hpp", sub=[lzss_streaming_coders]),
    # AlgorithmConfig(name="LCPCompressor", header="compressors/LCPCompressor.hpp", sub=[lzss_coders, lcpcomp_comp, lcpcomp_dec, lcpcomp_textds]),
    # AlgorithmConfig(name="RunLengthEncoder", header="compressors/RunLengthEncoder.hpp"),
    # AlgorithmConfig(name="LiteralEncoder", header="compressors/LiteralEncoder.hpp", sub=[all_coders]),
    AlgorithmConfig(name="LZ78Compressor", header="compressors/LZ78Compressor.hpp", sub=[universal_coders, lz78_trie]),
    # AlgorithmConfig(name="LZ78UCompressor", header="compressors/LZ78UCompressor.hpp", sub=[lz78u_comp, universal_coders]),
    # AlgorithmConfig(name="LZWCompressor", header="compressors/LZWCompressor.hpp", sub=[universal_coders, lz78_trie]),
    # AlgorithmConfig(name="RePairCompressor", header="compressors/RePairCompressor.hpp", sub=[non_consuming_coders]),
    # AlgorithmConfig(name="LZSSLCPCompressor", header="compressors/LZSSLCPCompressor.hpp", sub=[lzss_coders, textds]),
    # AlgorithmConfig(name="LZSSSlidingWindowCompressor", header="compressors/LZSSSlidingWindowCompressor.hpp", sub=[lzss_streaming_coders]),
    # AlgorithmConfig(name="MTFCompressor", header="compressors/MTFCompressor.hpp"),
    AlgorithmConfig(name="NoopCompressor", header="compressors/NoopCompressor.hpp"),
    # AlgorithmConfig(name="BWTCompressor", header="compressors/BWTCompressor.hpp", sub=[textds]),
    # AlgorithmConfig(name="ChainCompressor", header="compressors/ChainCompressor.hpp"),
    # AlgorithmConfig(name="DividingCompressor", header="compressors/DividingCompressor.hpp", sub=[dividing_strat]),
    # AlgorithmConfig(name="LongCommonStringCompressor", header="compressors/LongCommonStringCompressor.hpp", sub=[long_common_strat]),
    # AlgorithmConfig(name="EspCompressor", header="compressors/EspCompressor.hpp", sub=[slp_coder, ipddyn]),
    # AlgorithmConfig(name="lfs::LFSCompressor", header="compressors/lfs/LFSCompressor.hpp", sub=[lfs_strat, coding_strat]),
    # AlgorithmConfig(name="lfs::LFS2Compressor", header="compressors/lfs/LFS2Compressor.hpp", sub=[lit_coder, len_coder]),
    # AlgorithmConfig(name="lfs::LFS2BSTCompressor", header="compressors/lfs/LFS2BSTCompressor.hpp", sub=[lit_coder, len_coder]),
]

##### Export available decompressors #####
tdc.decompressors = [
    # AlgorithmConfig(name="BWTDecompressor", header="decompressors/BWTDecompressor.hpp",),
    # AlgorithmConfig(name="ChainDecompressor", header="decompressors/ChainDecompressor.hpp",),
    # AlgorithmConfig(name="DividingDecompressor", header="decompressors/DividingDecompressor.hpp"),
    # AlgorithmConfig(name="ESPDecompressor", header="decompressors/ESPDecompressor.hpp", sub=[slp_coder]),
    # AlgorithmConfig(name="LCPDecompressor", header="decompressors/LCPDecompressor.hpp", sub=[lzss_coders, lcpcomp_dec]),
    # AlgorithmConfig(name="lfs::LFSDecompressor", header="decompressors/LFSDecompressor.hpp", sub=[coding_strat]),
    # AlgorithmConfig(name="lfs::LFS2Decompressor", header="decompressors/LFS2Decompressor.hpp", sub=[lit_coder, len_coder]),
    AlgorithmConfig(name="LZ78Decompressor", header="decompressors/LZ78Decompressor.hpp", sub=[universal_coders]),
    # AlgorithmConfig(name="LZSSDecompressor", header="decompressors/LZSSDecompressor.hpp", sub=[lzss_coders]),
    # AlgorithmConfig(name="LZWDecompressor", header="decompressors/LZWDecompressor.hpp", sub=[universal_coders]),
    # AlgorithmConfig(name="WrapDecompressor", header="decompressors/WrapDecompressor.hpp"),
]


##### Export available string generators #####
tdc.generators = [
    AlgorithmConfig(name="FibonacciGenerator", header="generators/FibonacciGenerator.hpp"),
    AlgorithmConfig(name="ThueMorseGenerator", header="generators/ThueMorseGenerator.hpp"),
    AlgorithmConfig(name="RandomUniformGenerator", header="generators/RandomUniformGenerator.hpp"),
    AlgorithmConfig(name="RunRichGenerator", header="generators/RunRichGenerator.hpp"),
]

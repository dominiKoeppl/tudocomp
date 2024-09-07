#pragma once

#include <tudocomp/compressors/lzcics/cst_sada_light.hpp>
#include <tudocomp/compressors/lzcics/bp_support_sada.hpp>

#include <sdsl/suffix_trees.hpp>
// set the cache size used for caching psi and str_depth values. Undefine it to disable caching
#define CICS_CACHE_SIZE 100000
#include <tudocomp_stat/StatPhase.hpp>

namespace tdc {
namespace lzcics {

using namespace sdsl;

typedef cst_sada_light<csa_sada_light<>, bp_support_sadaM<>> cst_t;
// typedef cst_sada<sdsl::csa_sada<sdsl::enc_vector<sdsl::coder::elias_gamma,4>>> cst_t;

//typedef cst_sada<> cst_t;

/**
 * This is a wrapper class around the sdsl-lite library to get a easier translation between
 * the pseudocode in the LZCICS-paper and the C++ code
 */
struct ST {
    const cst_t& cst; //! sdsl suffix tree
    const cst_t::node_type root; //! the root node of the suffix tree
    const cst_t::size_type alpha; //! the \alpha-th leaf in the suffix tree has label 1
    const rank_support_v5<1> m_bp_rank1; //! rank the one bits in the BP
    const size_t internal_nodes; //! number of internal nodes

#ifdef CICS_CACHE_SIZE 
    constexpr static size_t cache_size = CICS_CACHE_SIZE;
    unsigned long *cache_value, *cache_index, *cache_op;
    IF_STATS(mutable size_t psi_cache_hit);
    IF_STATS(mutable size_t psi_cache_total);
    IF_STATS(mutable size_t str_depth_cache_hit);
    IF_STATS(mutable size_t str_depth_cache_total);


enum {
    NOP, PSI, STR_DEPTH, PARENT
};
  static constexpr size_t hash(int op, const cst_t::node_type& node)    {
        return (node * 7 + op) % cache_size;
    }
#endif

    void write_stats(tdc::StatPhase& phase) const {
#ifdef CICS_CACHE_SIZE
    IF_STATS(
                phase.log("psi_cache_hit"         , psi_cache_hit);
                phase.log("psi_cache_total"       , psi_cache_total);
                phase.log("str_depth_cache_hit"   , str_depth_cache_hit);
                phase.log("str_depth_cache_total" , str_depth_cache_total);
        )
                /*  */
                /* std::cout << "psi_cache_hit " << st.psi_cache_hit << " total " << st.psi_cache_total << std::endl; */
                /* std::cout << "str_depth_cache_hit " << st.str_depth_cache_hit << " total " << st.str_depth_cache_total << std::endl; */
#endif 
    }


    ST(const cst_t& _cst)
        : cst(_cst)
        , root(cst.root())
        , alpha(cst.csa.psi[0])
        , m_bp_rank1(&cst.bp)
        , internal_nodes(m_bp_rank1.rank(m_bp_rank1.size()) - cst.bp_rank_10.rank(m_bp_rank1.size()))
    {
        //traverse root children
        bit_vector root_children(cst.bp_support.size()); // temporary bit vector
        bit_vector root_children_lex(cst.bp_support.size()/2); // sada
        cst_t::size_type rc = 1;
    int c = 0; // sada
        while(rc < cst.bp_support.size() - 1) {
            root_children[rc] = 1;
            const cst_t::size_type rc_leafrank = leafrank(rc);
            root_children_lex[rc_leafrank] = 1; // sada
//            std::cout << c << " rc " << rc << " " << rc2 << std::endl;
            rc = cst.bp_support.find_close(rc) + 1; // take next sibling of rc
            c++;
        }

        m_rc_rrrv = rrr_vector<>(root_children);
        m_rc_rrrv_lex = rrr_vector<>(root_children_lex); // sada
        m_rc_rank = rrr_vector<>::rank_1_type(&m_rc_rrrv);
        m_rc_rank_lex = rrr_vector<>::rank_1_type(&m_rc_rrrv_lex); // sada

#ifdef CICS_CACHE_SIZE
        cache_value = new unsigned long[cache_size];
        cache_op    = new unsigned long[cache_size];
        cache_index = new unsigned long[cache_size];
        for (size_t i = 0; i < cache_size; i++) {
          cache_op[i]    = NOP;
          cache_index[i] = 0;
          cache_value[i] = 0;
        }
        IF_STATS(psi_cache_hit = psi_cache_total = 0);
        IF_STATS(str_depth_cache_hit = str_depth_cache_total = 0);
#endif


    }
    ~ST() {
#ifdef CICS_CACHE_SIZE
        delete [] cache_value;
        delete [] cache_op;
        delete [] cache_index;
#endif
    }

  // sada
  cst_t::node_type psi_get(const cst_t::node_type& node)  const {
//        std::cout << "psi_get(" << node << "): " <<  cache_op[node % cache_size] << " " << cache_index[node % cache_size] << std::endl;
    //int idx = node % cache_size;
#ifdef CICS_CACHE_SIZE
    const size_t idx = hash(PSI, node);
        if ((cache_op[idx] != PSI) || (cache_index[idx] != node)) {
            cache_value[idx] = cst.csa.psi[node];
            cache_op   [idx] = PSI;
            cache_index[idx] = node;
        } else {
            IF_STATS(++psi_cache_hit);
        }
        IF_STATS(++psi_cache_total);
//        if ((cache_op[idx] == PSI) && (cache_index[idx] == node) && (cst.csa.psi[node] != cache_value[idx])) {
//            printf("psi[%ld] = %ld, cache = %ld\n", node, cst.csa.psi[node], cache_value[idx]);
//        }
//        return cst.csa.psi[node];
        return cache_value[idx];
#else
    return cst.csa.psi[node];
#endif
    }

    cst_t::node_type str_depth(const cst_t::node_type& node)  const {
//        std::cout << "psi_get(" << node << "): " <<  psi_cache_index[node % cache_size] << std::endl;
//    int idx = node % cache_size;
#ifdef CICS_CACHE_SIZE
    int idx = hash(PSI, node);
    if ((cache_op[idx] != STR_DEPTH) || (cache_index[idx] != node)) {
            cache_value[idx] = str_depth_uncached(node);
            cache_op   [idx] = STR_DEPTH;
            cache_index[idx] = node;
        } else {
            IF_STATS(++str_depth_cache_hit);
        }
        IF_STATS(++str_depth_cache_total);
        return cache_value[idx];
#else
     return str_depth_uncached(node);
#endif
    }

    cst_t::node_type parent(const cst_t::node_type& node)const {
    return cst.parent(node);
    }
    /**
     * Returns the level ancestor of node.
     * In the sdsl-implementation, the 0-th ancestor is the node itself.
     * Here, we want that root is the 0-th ancestor.
     */
    cst_t::node_type level_anc(const cst_t::node_type& node, size_t depth)const {
        DVLOG(2) << "LevelAnc of node " << node << " on depth " << depth << " is " << cst.bp_support.level_anc(node, cst.node_depth(node)-depth);
        return cst.bp_support.level_anc(node, cst.node_depth(node)-depth);
    }

        rrr_vector<> m_rc_rrrv;
    rrr_vector<> m_rc_rrrv_lex; // sada
    rrr_vector<>::rank_1_type m_rc_rank;
    rrr_vector<>::rank_1_type m_rc_rank_lex; // sada

        inline static bool is_root(cst_t::size_type v)
        {
            return v==0;
        }

        //Degree of root
        /* cst_t::size_type root_degree() const // unused? */
        /* { */
        /*     return m_rc_rank(cst.bp_support.size() - 1); */
        /* } */
    /**
     * Returns how many preceding silbings node has.
     * node must be a direct child of the root.
     */
    cst_t::size_type root_child_rank(const cst_t::node_type& node) const {
            assert(!is_root(node));
//            assert(is_root(cst.bp_support.level_anc(node, 1))); //parent must be root! // sada

            return m_rc_rank(node);
        /* return cst.bp_support.root_child_rank(node); */
    }

    char head(const cst_t::node_type& node)const {
        DVLOG(2) << "child: " << root_child_rank(level_anc(node,1));
        DVLOG(2) << "Edge: " << (int) cst.csa.comp2char[root_child_rank(level_anc(node,1))];
        return cst.csa.comp2char[root_child_rank(node+1)-1];
    }


    char head2(const cst_t::node_type& lex)const { // sada
        return cst.csa.comp2char[m_rc_rank_lex(lex+1)-1];
    }

    /**
     * Return the leaf corresponding to the suffix starting at position 0, i.e., the leaf with the longest string-depth
     */
    cst_t::node_type smallest_leaf()const {
        return cst.select_leaf(alpha+1);
    }
    /*
     * Given a leaf node, we select the leaf whose label is the label of node plus one.
     */
    cst_t::node_type next_leaf(const cst_t::node_type& node)const {
//        DVLOG(2) << "Next Leaf " << node << "-> " << cst.select_leaf(cst.csa.psi[leafrank(node)]+1);
        DVLOG(2) << "Next Leaf " << node << "-> " << cst.select_leaf(psi_get(leafrank(node))+1);
        //sada
//        return cst.select_leaf(cst.csa.psi[leafrank(node)]+1);
        return cst.select_leaf(psi_get(leafrank(node))+1);
    }
    /*
     * apply m times next-leaf
     */
    cst_t::node_type next_mth_leaf(const cst_t::node_type& node, cst_t::size_type m)const {
        cst_t::size_type val = leafrank(node);
        while(m > 0) {
//            val = cst.csa.psi[val];
            val = psi_get(val);
            --m;
        }
        DVLOG(2) << "Next " << m << "-th Leaf " << node << "-> " << cst.select_leaf(m+1);
        return cst.select_leaf(val+1);
    }
    /*
     * returns the number of preceding leaves of node
     */
    cst_t::size_type leafrank(const cst_t::node_type& node)const{
        DVLOG(2) << "Leafrank " << node << "-> " << cst.bp_rank_10.rank(node);
        return cst.bp_rank_10.rank(node);
    }

    /*
     * Returns the length of the label read from the edges on the path from the root to node
     */
    cst_t::size_type str_depth_uncached(const cst_t::node_type& node)const{ // sada

        DCHECK(!cst.is_leaf(node)); // we do not support leaves
        auto lb = leafrank(cst.select_child(node, 2)); //! select the leftmost leaf of the second child
        auto la = lb - 1; //! the preceding leaf of lb is the rightmost leaf of the first child
        size_t m = 0; //! counts the number of matching characters
//        std::cout << "str_depth2(" << node << "): [" << head2(la) << "," << head2(lb) <<"] ";
        while(head2(la) == head2(lb)) {
            la = psi_get(la);
            lb = psi_get(lb);
            ++m;
        }
        //assert(m == cst.depth(node));
        DVLOG(2) << "Depth of " << node  << " is " << m;
        return m;
    }
    /*
     * Returns an unique ID for an internal node of the suffix tree
     */
    cst_t::size_type nid(const cst_t::node_type& node)const{
        assert(!cst.is_leaf(node));
        DVLOG(2) << "NID of node " << node << " is " << m_bp_rank1.rank(node) - cst.bp_rank_10.rank(node);
        return m_bp_rank1.rank(node)- cst.bp_rank_10.rank(node);
    }

    /**
     * Returns the number of leaves contained in the subtree rooted at 'node'
     */
    cst_t::size_type number_of_leaves(const cst_t::node_type& node)const{ // only for DEBUG
        return cst.size(node);
    }

};

inline void reset_bitvector(bit_vector& bv) { //! resets a bit-vector, clearing all ones
    for(auto it = bv.begin(); it != bv.end(); ++it) *it = 0; //TODO: is this the fastest approach?
}

//get suffix tree for text
inline ST suffix_tree(const uliteral_t* text, cst_t& cst) {
    construct_im(cst, (const char*)text, 1); //FIXME: SDSL s*cks
    return ST(cst);
}

}} //ns

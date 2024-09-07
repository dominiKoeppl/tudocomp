#pragma once

#include <tudocomp/Error.hpp>
#include <tudocomp/Tags.hpp>

#include <tudocomp/compressors/lz78/LZ78Coding.hpp>
#include <tudocomp/compressors/lzcics/st.hpp>

#include <tudocomp_stat/StatPhase.hpp>

#include <tudocomp/decompressors/LZ78Decompressor.hpp>

namespace tdc {

template<typename coder_t>
class LZ78CicsCompressor : public Compressor {
private:
    constexpr static size_t undef = -1;

    /**
     * For LZ78 only:
     * Stores for each ST edge the number of letters explored by the LZ78 factorization
     * Each factor adds a new letter to 'ne', exploring an ST edge further by adding a new LZ trie node on the edge.
     */
    struct ExplorationValues {
	    const size_t nodes; //! number of internal nodes which might get explored by the LZ78 factorization
	    size_t* ne; // storing the Exploration values
        /*
	     * TODO:
	     * Only internal nodes needed whose in-going edge has more than one letter.
	     * n_e wastes a lot of memory
	     *
	     * TODO:
	     * the factorization never explores nodes of ST whose string depth is larger than $\sqrt{2n}$.
	     *
	     * TODO:
	     * Store only ne for the big nodes, for the small nodes, use the implicit marking
	     *
	     */
	    ExplorationValues(const lzcics::ST& st)
		    : nodes(st.cst.bp.size()) //st.internal_nodes + st.cst.size()) // TODO: this value is too big
		    , ne(new size_t[nodes])
	    {
		    DVLOG(2) << "Nodes-Size: " << nodes;
		    reset();
	    }
	    ~ExplorationValues() {
		    delete [] ne;
	    }
	    size_t operator[](const lzcics::cst_t::node_type& node)const {
		    DCHECK_LT(node, nodes);
		    return ne[node];
	    }
	    void incr(const lzcics::cst_t::node_type& node) {
		    DCHECK_LT(node, nodes);
		    DVLOG(2) << "ne incr " << node;
		    ++ne[node];
	    }
	    void reset() {
		    for(size_t i = 0; i < nodes; ++i) ne[i] = 0;
	    }
    };

public:
    inline static Meta meta() {
        Meta m(Compressor::type_desc(), "lz78cics", "LZ78 compression in compressed space.");
        m.param("coder", "The output encoder.")
            .strategy<coder_t>(TypeDesc("coder"));
        m.add_tag(tags::require_sentinel);
        return m;
    }

    using Compressor::Compressor;

    /// \copydoc Compressor::compress
    inline virtual void compress(Input& input, Output& output) override {
        auto text = input.as_view();
        MissingSentinelError::check(text);

        // coder
        typename coder_t::Encoder coder(
            config().sub_config("coder"), output, ViewLiterals(text));

        // construct suffix tree
        lzcics::cst_t cst;
        auto st = StatPhase::wrap("Construct ST", [&]{
            return lzcics::suffix_tree(text.data(), cst);
        });

        // factorize
        StatPhase::wrap("Factorize", [&]{
	        const auto& cst = st.cst;
	        ExplorationValues ev(st);

	        sdsl::bit_vector bV(st.internal_nodes);
	        sdsl::bit_vector bC(st.internal_nodes);
	        sdsl::bit_vector bW(st.internal_nodes);
	        auto ell = st.smallest_leaf();
	        DVLOG(2) << "Selecting leaf " << ell;
	        do {
		        auto v = st.root;
		        size_t d = 0;
		        do {
			        v = st.level_anc(ell, ++d);
		        } while(v != ell && bV[st.nid(v)] != 0);
		        const auto u = st.parent(v);
		        const size_t s = st.str_depth(u); //ev[u];
		        if(v == ell) {
			        DVLOG(2) << "Leaf-Access: Omitting " << s << "+1 chars";
			        ell = st.next_mth_leaf(ell, s+1);
			        DVLOG(2) << "Selecting leaf " << ell;
			        continue;
		        }
		        const auto vid = st.nid(v);
		        bW[vid] = 1;

        //      I do not get why this code does not work!!!
        //
        // 		auto la = cst.leftmost_leaf(cst.select_child(v,st.child_rank(st.level_anc(ell,cst.node_depth(v)+1)) == 1 ? 0 : 1));
        // 		const size_t m = ev[v];
        // 		DVLOG(2) << "Omitting " << m << "+" << s << "+1 chars";
        // 		ell = st.next_mth_leaf(ell, m+s+1);
        // 		la = st.next_mth_leaf(la, m+s+1);
        // 		DVLOG(2) << "Selecting leaf " << ell;
        // 		if(st.head(ell) != st.head(la)) {
        // 			bV[vid] = 1;
        // 		}
        // 		assert( (st.head(ell) != st.head(la)) == ([&v,&cst,&st,&m,&s] () {
        // 			// TODO: exchange la with l, use child-rank and level-anc to determine lb correctly
        // 			auto la = cst.leftmost_leaf(cst.select_child(v, 1));
        // 			auto lb = cst.leftmost_leaf(cst.select_child(v, 2));
        //
        // 			DVLOG(2) << "Omitting " << m << "+" << s << "+1 chars";
        // //			ell = st.next_mth_leaf(ell, m+s+1);
        // 			la = st.next_mth_leaf(la, m+s+1);
        // 			lb = st.next_mth_leaf(lb, m+s+1);
        // //			DVLOG(2) << "Selecting leaf " << ell;
        // 			return st.head(la) != st.head(lb);
        // 		})());

		        // TODO: exchange la with l, use child-rank and level-anc to determine lb correctly
		        auto la = cst.leftmost_leaf(cst.select_child(v, 1));
		        auto lb = cst.leftmost_leaf(cst.select_child(v, 2));

		        const size_t m = ev[v];
		        DVLOG(2) << "Omitting " << m << "+" << s << "+1 chars";
		        ell = st.next_mth_leaf(ell, m+s+1);
		        la = st.next_mth_leaf(la, m+s+1);
		        lb = st.next_mth_leaf(lb, m+s+1);
		        DVLOG(2) << "Selecting leaf " << ell;
		        if(st.head(la) != st.head(lb)) {
			        bV[vid] = 1;
		        }
		        ev.incr(v);
	        } while(ell != st.smallest_leaf());
	        DVLOG(2) << bV;
	        DVLOG(2) << bW;
	        DVLOG(2) << "Nodes:";
	        for(size_t i = 0; i < ev.nodes; ++i) DVLOG(2) << ev[i];

	        sdsl::rank_support_v5<1> rankW(&bW);
	        const size_t zw = rankW.rank(bW.size());
	        size_t W[zw]; // TODO: lower this to int_vector<Math.ceil(log(zw)/log(2))>
	        for(size_t i = 0; i < zw; ++i) W[i] = undef;
	        ev.reset();
	        lzcics::reset_bitvector(bV);
	        size_t x = 0; // current factor == x-th factor

	        //std::vector<size_t> ref;
	        //std::vector<char> cha;

	        do {
		        auto v = st.root;
		        size_t d = 0;
		        do {
			        v = st.level_anc(ell, ++d);
		        } while(v != ell && bV[st.nid(v)] != 0);
		        DVLOG(2) << "v: " << v;
		        const auto u = st.parent(v);
		        const size_t s = st.str_depth(u); //ev[u];
		        if(v == ell) {
			        DVLOG(2) << "Leaf-Access: Omitting " << s << "+1 chars";
			        ell = st.next_mth_leaf(ell, s);
			        auto cha = st.head(ell);
			        DVLOG(2) << "cha: " << cha;
			        ell = st.next_leaf(ell);
			        DVLOG(2) << "Selecting leaf " << ell;

                    size_t ref;
			        if(cst.node_depth(v) == 1) {
				        DVLOG(2) << "fresh factor";
				        ref = undef;
			        } else {
				        ref = W[rankW.rank(st.nid(st.parent(v)))];
				        DVLOG(2) << x << "-th factor refers to " << ref << "-th factor";
			        }
                    lz78::encode_factor(coder, ref+1, cha, x);
			        ++x;
			        continue;
		        }
		        const auto vid = st.nid(v);
		        DVLOG(2) << "vid: " << vid << " witness: " << rankW.rank(vid);
		        const auto y = W[rankW.rank(vid)];
		        DVLOG(2) << "y= " << y;
                size_t ref;
		        if(y == undef) {
		            DVLOG(2) << "depth(v)= " << cst.node_depth(v);
			        if(cst.node_depth(v) == 1) {
				        DVLOG(2) << "fresh factor";
				        ref = undef;
			        } else {
				        ref = W[rankW.rank(st.nid(st.parent(v)))];
				        DVLOG(2) << x << "-th factor refers to " << ref << "-th factor";
			        }
		        }
		        else {
			        DVLOG(2) << x << "-th factor refers to " << y << "-th factor";
			        ref = y;
		        }

		        W[rankW.rank(vid)] = x;
		        DVLOG(2) << "W: ";
		        for(size_t i = 0; i < zw; ++i) DVLOG(2) << " " << W[i];
		        DVLOG(2);

		        DVLOG(2) << "V: ";
		        for(size_t i = 0; i < bV.size(); ++i) DVLOG(2) << " " << bV[i];
		        DVLOG(2);

		        DVLOG(2) << "N: ";
		        for(size_t i = 0; i < ev.nodes; ++i) DVLOG(2) << " " << ev[i];
		        DVLOG(2);

		        // TODO: exchange la with l, use child-rank and level-anc to determine lb correctly
		        auto la = cst.leftmost_leaf(cst.select_child(v, 1));
		        auto lb = cst.leftmost_leaf(cst.select_child(v, 2));

		        const size_t m = ev[v];
		        DVLOG(2) << "Omitting " << m << "+" << s << "+1 chars";
		        la = st.next_mth_leaf(la, m+s+1);
		        lb = st.next_mth_leaf(lb, m+s+1);
		        ell = st.next_mth_leaf(ell, m+s);
		        auto cha = st.head(ell);
		        DVLOG(2) << "cha: " << st.head(ell);
                lz78::encode_factor(coder, ref+1, cha, x);
		        ell = st.next_leaf(ell);
		        DVLOG(2) << "Selecting leaf " << ell;
		        if(st.head(la) != st.head(lb)) {
			        bV[vid] = 1;
		        }
		        ev.incr(v);
		        ++x;
	        } while(ell != st.smallest_leaf());
        IF_STATS(
		StatPhase::log("factor_count", x);
        )

        });
    }

    inline std::unique_ptr<Decompressor> decompressor() const override {
        return Algorithm::unique_instance<LZ78Decompressor<coder_t>>();
    }
};

}



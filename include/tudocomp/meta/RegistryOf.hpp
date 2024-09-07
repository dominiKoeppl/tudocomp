#pragma once

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <tudocomp/meta/Config.hpp>
#include <tudocomp/meta/Meta.hpp>

namespace tdc {
namespace meta {

/// \brief Error type for registry related errors.
class RegistryError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/// \cond INTERNAL
class RegistryOfAny {
};
/// \endcond INTERNAL

template<typename T>
class RegistryOf : public RegistryOfAny {
public:
    using register_callback_t = std::function<void(const Meta&)>;    

private:
    using ctor_t = std::function<std::unique_ptr<T>(Config&&)>;

    TypeDesc m_root_type;
    DeclLib m_lib;
    std::unordered_map<std::string, ctor_t> m_reg;

    std::vector<register_callback_t> m_callback;

public:
    inline RegistryOf() : m_root_type(T::type_desc()) {
    }

    inline RegistryOf(const TypeDesc& root_type) : m_root_type(root_type) {
    }

    template<typename Algo>
    inline void register_algorithm() {
        auto meta = Algo::meta();
        auto type = meta.decl()->type();
        if(!type.subtype_of(m_root_type)) {
            throw RegistryError(std::string(
                "trying to register algorithm of type ") +
                type.name() +
                std::string(", expected ") +
                m_root_type.name());
        }

        auto sig = meta.signature()->str();
        auto it = m_reg.find(sig);
        if(it == m_reg.end()) {
            add_to_lib(m_lib, meta);
            m_reg.emplace(sig, [](Config&& cfg) {
                return std::make_unique<Algo>(std::move(cfg));
            });
        } else {
            throw RegistryError(std::string("already registered: ") + sig);
        }

        for(auto& f : m_callback) {
            f(meta);
        }
    }

    inline void add_register_callback(register_callback_t f) {
        m_callback.push_back(f);
    }

    class Selection {
    private:
        friend class RegistryOf;

        std::shared_ptr<const Decl> m_decl;
        std::unique_ptr<T> m_instance;

        inline Selection(
            std::shared_ptr<const Decl> decl,
            std::unique_ptr<T>&& instance)
            : m_decl(decl), m_instance(std::move(instance)) {
        }

    public:
        inline Selection() {
        }

        inline Selection(Selection&& other)
            : m_decl(std::move(other.m_decl)),
              m_instance(std::move(other.m_instance)) {
        }

        inline Selection& operator=(Selection&& other) {
            m_decl = std::move(other.m_decl);
            m_instance = std::move(other.m_instance);
            return *this;
        }

        inline operator bool() const {
            return bool(m_instance);
        }

        inline std::shared_ptr<const Decl> decl() const {
            return m_decl;
        }

        inline T& instance() {
            return *m_instance;
        }

        inline T& operator*() {
            return instance();
        }

        inline T* operator->() {
            return m_instance.get();
        }

        inline std::unique_ptr<T>&& move_instance() {
            return std::move(m_instance);
        }
    };

    class Entry {
    private:
        friend class RegistryOf;

        std::shared_ptr<const Decl> m_decl;
        const ctor_t* m_ctor;
        Config m_cfg;

        inline Entry(
            std::shared_ptr<const Decl> decl,
            const ctor_t& ctor,
            Config&& cfg) : m_decl(decl), m_ctor(&ctor), m_cfg(cfg) {
        }

    public:
        inline std::shared_ptr<const Decl> decl() const {
            return m_decl;
        }

        inline const Config& config() const {
            return m_cfg;
        }

        inline Selection select() const {
            return Selection(m_decl, (*m_ctor)(Config(m_cfg)));
        }
    };

    inline Entry find(ast::NodePtr<ast::Object> obj) const {
        auto decl = m_lib.find(obj->name(), m_root_type);
        if(!decl) {
            throw RegistryError(
                std::string("unknown algorithm: ") + obj->name());
        }

        auto cfg = Config(decl, obj, m_lib);
        auto sig = cfg.signature();
        auto reg_entry = m_reg.find(sig->str());

        if(reg_entry == m_reg.end()) {
            throw RegistryError(
                std::string("unregistered instance: ") + sig->str());
        }

        return Entry(decl, reg_entry->second, std::move(cfg));
    }

    inline Selection select(ast::NodePtr<ast::Object> obj) const {
        return find(obj).select();
    }

    inline Selection select(const std::string& str) const {
        auto obj = ast::convert<ast::Object>(ast::Parser::parse(str));
        return select(obj);
    }

    template<typename C>
    inline Selection select(const std::string& options = "") const {
        auto meta = C::meta();
        auto decl = meta.decl();
        auto parsed = ast::convert<ast::Object>(
            ast::Parser::parse(decl->name() + paranthesize(options)));
        auto obj = parsed->inherit(meta.signature());
        auto cfg = Config(
            decl, obj, m_lib + meta.known());

        return Selection(decl, std::make_unique<C>(std::move(cfg)));
    }

    inline const TypeDesc& root_type() const {
        return m_root_type;
    }

    inline const DeclLib& library() const {
        return m_lib;
    }

    inline std::vector<std::shared_ptr<const Decl>> declarations() const {
        return m_lib.entries();
    }

    inline std::vector<std::string> signatures() const {
        std::vector<std::string> sigs;
        for(auto e : m_reg) sigs.emplace_back(e.first);
        return sigs;
    }
};

} //namespace meta

template<typename T>
using RegistryOf = meta::RegistryOf<T>;

} //namespace tdc

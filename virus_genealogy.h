#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <vector>
#include <exception>
#include <stdexcept>
#include <memory>
#include <map>
#include <unordered_map>

class VirusAlreadyCreated : std::runtime_error {
public:
    VirusAlreadyCreated()
        : std::runtime_error("VirusAlreadyCreated") {}
};

class VirusNotFound : std::runtime_error {
public:
    VirusNotFound()
        : std::runtime_error("VirusNotFound") {}
};

class TriedToRemoveStemVirus : std::runtime_error {
public:
    TriedToRemoveStemVirus()
        : std::runtime_error("TriedToRemoveStemVirus") {}
};

template<class Virus>
class VirusGenealogy {

public:

    struct Node;

    VirusGenealogy(const VirusGenealogy& v) = delete;

    VirusGenealogy(const typename Virus::id_type& stem_id)
        : stem_node(nullptr)
    {
        stem_node = std::make_shared<Node>(stem_id);
        std::weak_ptr<Node> stem_weak_ptr = stem_node;
        virus_map.insert(std::make_pair(stem_id, stem_weak_ptr));
    }

    typename Virus::id_type get_stem_id() const {
        return (*stem_node->virus_ptr).get_id();
    }

    std::vector<typename Virus::id_type>
    get_children(const typename Virus::id_type& id) const
    {
        auto spt = find_map(id).lock();
        std::vector<typename Virus::id_type> res;

        for (auto p : spt->children)
            res.push_back(p.first);

        return res;
    }

    std::vector<typename Virus::id_type>
    get_parents(const typename Virus::id_type& id) const
    {
        auto spt = find_map(id).lock();
        std::vector<typename Virus::id_type> res;

        for (auto p : spt->parents)
            res.push_back(p.first);

        return res;
    }

    bool exists(const typename Virus::id_type& id) const
    {
        return virus_map.find(id) != virus_map.end(); //TODO STRONG
    }

    Virus& operator[](const typename Virus::id_type& id) const
    {
        auto spt = find_map(id).lock(); //TODO STRONG?
        return *spt->virus_ptr;
    }

    void create(const typename Virus::id_type& id,
                const typename Virus::id_type& parent_id)
    {
        create(id, std::vector<const typename Virus::id_type>({parent_id}));
    }

    void create(const typename Virus::id_type& id,
                            const std::vector<typename Virus::id_type>& parent_ids)
    {
    if (exists(id))
        throw VirusAlreadyCreated();
    if (parent_ids.empty())
        throw VirusNotFound();
    for (typename Virus::id_type parent : parent_ids)
    {
        if (!exists(parent))
            throw VirusNotFound();
    }

    std::shared_ptr<Node> v_sptr = std::make_shared<Node>(id);
    std::weak_ptr<Node> v_ptr = v_sptr;

    for (typename Virus::id_type parent : parent_ids)
    {
        std::weak_ptr<Node> parent_ptr = find_map(parent);
        v_sptr->parents.insert(std::make_pair(parent, parent_ptr));

        auto parent_sptr = parent_ptr.lock();
        parent_sptr->children.insert(std::make_pair(id, v_ptr));
    }
        virus_map.insert(std::make_pair(id, v_ptr));
    }

    void connect(const typename Virus::id_type& child_id,
                             const typename Virus::id_type& parent_id)
    {
        auto parent_weak = find_map(parent_id);
        auto child = find_map(child_id).lock();

        auto ins_result = child->parents.insert(std::make_pair(parent_id, parent_weak));

        try {
            auto parent = parent_weak.lock();
            parent->children.insert(std::make_pair(child_id, child));
        } catch (std::exception& e) {
            child->parents.erase(ins_result.first); // nothrow na iteratorze
        }
    }

    void remove(const typename Virus::id_type& id)
    {
        if (id == stem_node->id)
            throw TriedToRemoveStemVirus();

        {
            auto spt = find_map(id).lock();

            for (auto parent : spt->parents) {
                auto parent_ptr = parent.second.lock();
                parent_ptr->children.erase(spt->id);
            }
        } // ~shared_ptr<Node>();

        for (auto it = virus_map.begin(); it != virus_map.end(); )
        {
            if ((*it).second.expired())
                it = virus_map.erase(it);
            else
                ++it;
        }
    }

    const VirusGenealogy& operator=(const VirusGenealogy& rhs) = delete;

private:

    class Node {
    public:
        typename Virus::id_type id;
        std::unique_ptr<Virus> virus_ptr;
        std::map<typename Virus::id_type, std::shared_ptr<Node>> children;
        std::map<typename Virus::id_type, std::weak_ptr<Node>> parents;

        Node(const typename Virus::id_type& vir_id)
            : virus_ptr(new Virus(id))
            , id(vir_id) {}
    };

    std::unordered_map<Virus::id_type, std::weak_ptr<Node>> virus_map;
    std::shared_ptr<Node> stem_node;

    std::weak_ptr<Node>& find_map(const typename Virus::id_type& id) const
    {
        try {
            return virus_map.at(id);
        }
        catch (const std::exception& e) {
            throw VirusNotFound();
        }
    }
};

#endif /* VIRUS_GENEALOGY_H */

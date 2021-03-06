#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <vector>
#include <exception>
#include <stdexcept>
#include <memory>
#include <map>
#include <unordered_map>
#include <stack>

class VirusNotFound : public std::exception
{
	const char* what() const noexcept
	{
		return "VirusNotFound";
	}
};

class VirusAlreadyCreated : public std::exception
{
	const char* what() const noexcept
	{
		return "VirusAlreadyCreated";
	}
};

class TriedToRemoveStemVirus : public std::exception
{
	const char* what() const noexcept
	{
		return "TriedToRemoveStemVirus";
	}
};

template<class Virus>
class VirusGenealogy {

private:

	typedef typename Virus::id_type id_type;

	class Node {
	public:
		id_type id;
		std::unique_ptr<Virus> virus_ptr;
		std::map<id_type, std::shared_ptr<Node>> children;
		std::map<id_type, std::weak_ptr<Node>> parents;

		Node(const id_type& vir_id)
				: id(vir_id)
				, virus_ptr(new Virus(vir_id)) {}
	};

	std::unordered_map<id_type, std::weak_ptr<Node>> virus_map;
	std::shared_ptr<Node> stem_node;

	std::weak_ptr<Node> find_map(const id_type& id) const
	{
		try {
			return virus_map.at(id);
		}
		catch (const std::exception& e) {
			throw VirusNotFound();
		}
	}

public:

    VirusGenealogy(const VirusGenealogy& v) = delete;

    VirusGenealogy(const id_type& stem_id)
        : stem_node(nullptr)
    {
        stem_node = std::make_shared<Node>(stem_id);
        std::weak_ptr<Node> stem_weak_ptr = stem_node;
        virus_map.insert(std::make_pair(stem_id, stem_weak_ptr));
    }

    id_type get_stem_id() const
    {
        return (*stem_node->virus_ptr).get_id();
    }

    std::vector<id_type>
    get_children(const id_type& id) const
    {
        auto spt = find_map(id).lock();
        std::vector<id_type> res;

        for (auto p : spt->children)
            res.push_back(p.first);

        return res;
    }

    std::vector<id_type>
    get_parents(const id_type& id) const
    {
        auto spt = find_map(id).lock();
        std::vector<id_type> res;

        for (auto p : spt->parents)
            res.push_back(p.first);

        return res;
    }

    bool exists(const id_type& id) const
    {
        return virus_map.find(id) != virus_map.end();
    }

    Virus& operator[](const id_type& id) const
    {
        auto spt = find_map(id).lock();
        return *spt->virus_ptr;
    }

    void create(const id_type& id,
                const id_type& parent_id)
    {
        create(id, std::vector<id_type>(1, parent_id));
    }

    void create(const id_type& id,
                const std::vector<id_type>& parent_ids)
    {
        if (exists(id))
            throw VirusAlreadyCreated();

        if (parent_ids.empty())
            throw VirusNotFound();

        for (id_type parent : parent_ids)
        {
            if (!exists(parent))
                throw VirusNotFound();
        }

        std::shared_ptr<Node> v_sptr = std::make_shared<Node>(id);
        std::weak_ptr<Node> v_ptr = v_sptr;

        std::stack<std::pair<std::map<id_type, std::shared_ptr<Node>>*,
                             typename std::map<id_type, std::shared_ptr<Node>>::iterator>>
        operation_stack;

        try {
            for (id_type parent : parent_ids)
            {
                auto parent_ptr = find_map(parent);
                v_sptr->parents.insert(std::make_pair(parent, parent_ptr));

                auto parent_sptr = parent_ptr.lock();
                auto ins_result = parent_sptr->children.insert(std::make_pair(id, v_sptr));

                operation_stack.push(std::make_pair(&parent_sptr->children, ins_result.first));
            }

            virus_map.insert(std::make_pair(id, v_ptr));
        } catch (...) {
            while (!operation_stack.empty())
            {
                auto op = operation_stack.top();
                (*op.first).erase(op.second);
                operation_stack.pop();
            }
        }
    }

    void connect(const id_type& child_id,
                             const id_type& parent_id)
    {
        auto parent_weak = find_map(parent_id);
        auto child = find_map(child_id).lock();

        auto ins_result = child->parents.insert(std::make_pair(parent_id, parent_weak));

        try {
            auto parent = parent_weak.lock();
            parent->children.insert(std::make_pair(child_id, child));
        } catch (...) {
            child->parents.erase(ins_result.first);
        }
    }

    void remove(const id_type& id)
    {
        if (id == stem_node->id)
            throw TriedToRemoveStemVirus();

        std::stack<std::map<id_type, std::shared_ptr<Node>>*>
        operation_stack_parents;

        std::stack<std::map<id_type, std::weak_ptr<Node>>*>
        operation_stack_children;

        {
            auto weak = find_map(id);
            auto spt = weak.lock();

            try {
                for (auto parent : spt->parents)
                {
                    auto parent_ptr = parent.second.lock();
                    parent_ptr->children.erase(spt->id);

                    operation_stack_parents.push(&parent_ptr->children);
                }

                for (auto child : spt->children)
                {
                    auto child_ptr = child.second;
                    child_ptr->parents.erase(spt->id);

                    operation_stack_children.push(&child_ptr->parents);
                }
            } catch (...) {
                while (!operation_stack_parents.empty())
                {
                    auto op = operation_stack_parents.top();
                    (*op).insert(std::make_pair(spt->id, spt));
                    operation_stack_parents.pop();
                }

                while (!operation_stack_children.empty())
                {
                    auto op = operation_stack_children.top();
                    (*op).insert(std::make_pair(spt->id, weak));
                    operation_stack_children.pop();
                }
            }
        }

        for (auto it = virus_map.begin(); it != virus_map.end(); )
        {
            if ((*it).second.expired())
                it = virus_map.erase(it);
            else
                ++it;
        }
    }

    const VirusGenealogy& operator=(const VirusGenealogy& rhs) = delete;
};

#endif /* VIRUS_GENEALOGY_H */
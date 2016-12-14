#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <vector>
#include <exception>
#include <stdexcept>

class VirusAlreadyCreated : std::runtime_error {
public:
	VirusAlreadyCreated()
		: std::runtime_error("VirusAlreadyCreated") {}
}

class VirusNotFound : std::runtime_error {
public:
	VirusNotFound()
		: std::runtime_error("VirusNotFound") {}
}

class TriedToRemoveStemVirus : std::runtime_error {
public:
	TriedToRemoveStemVirus()
		: std::runtime_error("TriedToRemoveStemVirus") {}
}

template<class Virus>
class VirusGenealogy {

public:

	VirusGenealogy(const VirusGenealogy& v) = delete;

	VirusGenealogy(const Virus::id_type& stem_id)
	{
		// TODO
	}

	Virus::id_type get_stem_id() const {
		// TODO
	}

	std::vector<Virus::id_type>
	get_children(const Virus::id_type& id) const {
		// TODO
	}

	std::vector<Virus::id_type>
	get_parents(const Virus::id_type& id) const {
		// TODO
	}

	bool exists(const Virus::id_type& id) const {
		// TODO
	}

	Virus& operator[](const Virus::id_type& id) const {
		// TODO
	}

	void create(const Virus::id_type& id,
				const Virus::id_type& parent_id) {
		// TODO
	}

	void create(const Virus::id_type& id,
				const std::vector<Virus::id_type>& parent_ids) {
		// TODO
	}

	void connect(const Virus::id_type& child_id,
				 const Virus::id_type& parent_id) {
		// TODO
	}

	void remove(const Virus::id_type& id) {
		// TODO
	}

	const VirusGenealogy& operator=(const VirusGenealogy& rhs) = delete;

private:

};

#endif /* VIRUS_GENEALOGY_H */

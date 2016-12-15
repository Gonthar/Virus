#include "virus_genealogy.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

class Virus {
public:
    typedef typename std::string id_type;
    Virus(id_type const &_id) : id(_id) {
    }
    id_type get_id() const {
        return id;
    }
private:
    id_type id;
};

int main() {
    VirusGenealogy<Virus> gen("A1H1");
    Virus::id_type const id1 = gen.get_stem_id();

    ///testing long chains of viruses
    gen.create("f", id1);
    gen.create("g", "f");
    gen.create("h", "g");
    gen.create("i", "g");
    gen.create("j", "g");
    std::vector<Virus::id_type> split;
    split.push_back("h");
    split.push_back("i");
    split.push_back("j");
    gen.create("k", split);
    gen.create("l", "k");

    gen.remove("h");
    assert(!gen.exists("h"));
    assert(gen.exists("i"));
    assert(gen.exists("k"));

    gen.remove("f");
    assert(!gen.exists("f"));
    assert(!gen.exists("g"));
    assert(!gen.exists("k"));

    ///testing parents/children size after removal
    gen.create("a", id1);
    gen.create("b", id1);
    std::vector<Virus::id_type> split2;
    split2.push_back("a");
    split2.push_back("b");
    gen.create("c", split2);
    gen.create("d", split2);
    gen.create("e", "b");
    assert(gen.get_children("a").size() == 2);
    assert(gen.get_children("b").size() == 3);
    assert(gen.get_parents("c").size() == 2);
    assert(gen.get_parents("d").size() == 2);
    assert(gen.get_parents("e").size() == 1);

    gen.remove("b");
    assert(gen.get_children("a").size() == 2);
    assert(gen.get_parents("c").size() == 1);
    assert(gen.get_parents("d").size() == 1);
    assert(!gen.exists("e"));

    gen.remove("c");
    assert(!gen.exists("c"));
    assert(gen.get_children("a").size() == 1);

    gen.remove("a");
    assert(!gen.exists("a"));
    assert(!gen.exists("c"));
    assert(!gen.exists("d"));

    ///testing connect
    gen.create("sebek", id1);
    gen.create("karyna", id1);
    gen.create("brajanek", "karyna");
    gen.create("dzesika", "karyna");
    gen.connect("brajanek", "sebek");
    gen.connect("dzesika", "sebek");
    gen.connect("brajanek", id1);
    assert(gen.get_children("sebek").size() == 2);
    assert(gen.get_children(id1).size() == 3);
    assert(gen.get_parents("dzesika").size() == 2);
    assert(gen.get_parents("brajanek").size() == 3);
    gen.remove("sebek");
    gen.remove("karyna");
    assert(gen.exists("brajanek"));
    assert(!gen.exists("dzesika"));

    try {
        gen["X"];
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    try {
        gen.create("X", "A1H1");
        gen.create("X", "A1H1");
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    try {
        gen.remove("A1H1");
    }
    catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}

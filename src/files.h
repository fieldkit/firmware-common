#ifndef FK_FILES_H_INCLUDED
#define FK_FILES_H_INCLUDED

#include <phylum/phylum.h>

namespace fk {

class Files {
private:
    phylum::FileOpener *files;

public:
    Files(phylum::FileOpener &files);

public:
    phylum::SimpleFile &data();
    phylum::SimpleFile &log();

};

}

#endif

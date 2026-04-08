#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlib.h>
#include <rpm/rpmlog.h>
#include <rpm/rpmts.h>

int main(void) {
	FILE *devnull = fopen("/dev/null", "w");
	rpmlogSetFile(devnull);

	rpmReadConfigFiles(NULL, NULL);

	rpmts ts = NULL;
	ts = rpmtsCreate();
	rpmtsSetFlags(ts, rpmtsFlags(ts) | RPMTRANS_FLAG_NOPLUGINS);
	rpmdbMatchIterator mi = rpmtsInitIterator(ts, RPMDBI_PACKAGES, NULL, 0);
	
	Header header = NULL;
	while((header = rpmdbNextIterator(mi)) != NULL) {
		const char *name, *version, *release;
		name = headerGetString(header, RPMTAG_NAME);
		version = headerGetString(header, RPMTAG_VERSION);
		release = headerGetString(header, RPMTAG_RELEASE);
		printf("%s-%s-%s\n", name, version, release);
	}

	rpmdbFreeIterator(mi);
	rpmtsFree(ts);
	return 0;
}

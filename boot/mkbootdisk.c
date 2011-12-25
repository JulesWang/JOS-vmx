#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#if defined(_MSDOS) || defined(_WIN32)
# include <fcntl.h>
# include <io.h>
#endif

/* This program makes a boot image.
 * It takes at least one argument, the boot sector.
 * Any succeeding arguments are written verbatim to the output file.
 *
 * Before jumping to the boot sector, the BIOS checks that the last
 * two bytes in the sector equal 0x55 and 0xAA.
 * This code makes sure the code intended for the boot sector is at most
 * 512 - 2 = 510 bytes long, then appends the 0x55-0xAA signature.
 *
 * This program used to be written in Perl, but Solaris SEASnet machines
 * have an ancient crappy Perl.
 */

void
usage(void)
{
	fprintf(stderr, "Usage: mkbootdisk BOOTSECTOR [FILE | @SECNUM]...\n");
	exit(1);
}

FILE *
fopencheck(const char *name)
{
	FILE *f = fopen(name, "rb");
	if (!f) {
		fprintf(stderr, "%s: %s\n", name, strerror(errno));
		usage();
	}
	return f;
}

int
main(int argc, char *argv[])
{
	char buf[1000];
	char zerobuf[512];
	FILE *f;
	size_t n;
	size_t nsectors;
	int i;
	
#if defined(_MSDOS) || defined(_WIN32)
	// As our output file is binary, we must set its file mode to binary.
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	
	// Read boot sector
	if (argc < 2)
		usage();
	f = fopencheck(argv[1]);
	n = fread(buf, 1, 1000, f);
	if (n > 510) {
		fprintf(stderr, "%s: boot block too large: %s%u bytes (max 510)\n", argv[1], (n == 1000 ? ">= " : ""), (unsigned) n);
		usage();
	}
	fclose(f);

	// Append signature and write modified boot sector
	memset(buf + n, 0, 510 - n);
	buf[510] = 0x55;
	buf[511] = 0xAA;
	fwrite(buf, 1, 512, stdout);
	nsectors = 1;
	
	// Read any succeeding files, then write them out
	memset(zerobuf, 0, 512);
	for (i = 2; i < argc; i++) {
		size_t pos;
		char *str;
		unsigned long skipto_sector;
		
		// An argument like "@X" means "skip to sector X".
		if (argv[i][0] == '@' && isdigit(argv[i][1])
		    && ((skipto_sector = strtoul(argv[i] + 1, &str, 0)), *str == 0)) {
			if (nsectors > skipto_sector) {
				fprintf(stderr, "mkbootdisk: can't skip to sector %u, already at sector %u\n", (unsigned) skipto_sector, (unsigned) nsectors);
				usage();
			}
			while (nsectors < skipto_sector) {
				fwrite(zerobuf, 1, 512, stdout);
				nsectors++;
			}
			continue;
		}

		// Otherwise, read the file.
		f = fopencheck(argv[i]);
		pos = 0;
		while ((n = fread(buf, 1, 1000, f)) > 0) {
			fwrite(buf, 1, n, stdout);
			pos += n;
		}
		if (pos % 512 != 0) {
			fwrite(zerobuf, 1, 512 - (pos % 512), stdout);
			pos += 512 - (pos % 512);
		}
		nsectors += pos / 512;
		fclose(f);
	}

	// Fill out to 1024 sectors with 0 blocks
	while (nsectors < 1024) {
		fwrite(zerobuf, 1, 512, stdout);
		nsectors++;
	}
	
	return 0;
}

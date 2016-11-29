#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#define PROG_NAME "jsinc"
#define PROG_VER  "1.0"

#define PROG_USAGE                                    \
"Convert any file into a JavaScript ArrayBuffer.\n\n" \
"Usage: " PROG_NAME " <options> <file>\n\n"           \
"Options:\n"                                          \
"  -v           Print version number and exit.\n"     \
"  -p <prefix>  Default:  window['<file>'] =\n"       \
"  -P <suffix>  Default:  ;\n"

static void require_unset(const void* val, const char *name)
{
	if (val) {
		fprintf(stderr, PROG_NAME ": %s is already set\n", name);
		exit(EXIT_FAILURE);
	}
}

static void require_arg(int i, int argc, const char *name)
{
	if (i + 1 >= argc) {
		fprintf(stderr, PROG_NAME
			": %s requires an argument\n", name);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, const char *argv[])
{
	const char *filename = NULL;
	const char *prefix = NULL;
	const char *suffix = NULL;
	int i, c, n;
	long len;
	FILE *in;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
			printf(PROG_VER "\n");
			return 0;
		} else if (!strcmp(argv[i], "-p")) {
			require_unset(prefix, "prefix (-p)");
			require_arg(i, argc, "-p");
			prefix = argv[++i];
		} else if (!strcmp(argv[i], "-P")) {
			require_unset(suffix, "suffix (-P)");
			require_arg(i, argc, "-P");
			suffix = argv[++i];
		} else if (argv[i][0] == '-') {
			fprintf(stderr, PROG_NAME ": unknown argument: %s\n",
				argv[i]);
			exit(EXIT_FAILURE);
		} else {
			require_unset(filename, "filename");
			filename = argv[i];
		}
	}

	if (!filename) {
		if (argc > 1) {
			fprintf(stderr, PROG_NAME ": no filename given\n");
			exit(EXIT_FAILURE);
		} else {
			fprintf(stderr, "%s", PROG_USAGE);
			return 0;
		}
	}

	in = fopen(filename, "r");
	if (!in) {
		perror(PROG_NAME);
		exit(EXIT_FAILURE);
	}

	if (prefix) printf("%s new Uint32Array([", prefix);
	else        printf("; window['%s'] = new Uint32Array([", filename);

	i = 0;
	while ((c = fgetc(in)) != EOF) {
		n = c << 24;
		if ((c = fgetc(in)) != EOF) n |= c << 16;
		if ((c = fgetc(in)) != EOF) n |= c << 8;
		if ((c = fgetc(in)) != EOF) n |= c;

		     if (i%6) printf(   ", 0x%08X", htonl(n));
		else if (i)   printf(",\n\t0x%08X", htonl(n));
		else          printf( "\n\t0x%08X", htonl(n));

		i++;
	}

	if (ferror(in)) {
		perror("file-js");
		fclose(in);
		exit(EXIT_FAILURE);
	}

	len = ftell(in);
	if (len % 4) {
		printf("\n]).buffer.slice(0, %ld)%s\n", len,
			suffix ? suffix : ";");
	} else {
		printf("\n]).buffer%s\n", suffix ? suffix : ";");
	}

	fclose(in);
	return 0;
}

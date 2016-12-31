#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#ifdef _WIN32
# include <winsock2.h>
#else
# include <arpa/inet.h>
#endif

#define PROG_NAME "jsinc"

#ifdef PROG_VER
# define STR_(x) #x
# define STR(x)  STR_(x)
# define PROG_VER_STR STR(PROG_VER)
#else
# define PROG_VER_STR "dev"
#endif

#define PROG_USAGE                                               \
"Convert any file into a JavaScript ArrayBuffer.\n\n"            \
"Usage: " PROG_NAME " <options> <file>\n\n"                      \
"Options:\n"                                                     \
"  -v           Print version number and exit.\n"                \
"  -m <style>   Use module style, see below. Default: global\n"  \
"  -p <prefix>  Text to prepend to the output\n"                 \
"  -s <suffix>  Text to append to the output\n"                  \
"  -c           Do not output a generator comment\n\n"           \
"Supported module styles:\n"                                     \
"  global    ; window['<file>'] = ...;\n"                        \
"  amd       ; define('<file>', function() { return  ...; });\n" \
"  commonjs  module.exports = ...;\n"                            \
"  es6       export default ...;\n"                              \
"  none      ...\n"

struct module_style {
	const char *name;
	const char *prefix_fmt;
	const char *tail;
};

static const struct module_style module_styles[] = {
	{ "none",     "",                                    ""      },
	{ "global",   "; window['%s'] = ",                   ";"     },
	{ "amd",      "; define('%s', function() { return ", "; });" },
	{ "commonjs", "module.exports = ",                   ";"     },
	{ "es6",      "export default ",                     ";"     }
};

static const struct module_style *find_module_style(const char *name)
{
	int i;
	size_t num_styles = sizeof(module_styles) / sizeof(module_styles[0]);

	for (i = 0; i < num_styles; i++) {
		if (!strcmp(module_styles[i].name, name)) {
			return &module_styles[i];
		}
	}

	return NULL;
}

int main(int argc, char **argv)
{
	const char *filename = NULL;
	const char *prefix = NULL;
	const char *suffix = NULL;
	const char *stylearg = NULL;
	const struct module_style *style;
	int nocomment = 0;
	int opt;
	int i, c, n;
	long len;
	FILE *in;

	if (argc < 2) {
		printf(PROG_USAGE);
		exit(EXIT_SUCCESS);
	}

	if (argc > 1 && !strcmp(argv[1], "--version")) {
		printf(PROG_VER_STR "\n");
		exit(EXIT_SUCCESS);
	}

	while ((opt = getopt(argc, argv, "vm:p:s:c")) >= 0) {
		switch (opt) {
		case 'v':
			printf(PROG_VER_STR "\n");
			exit(EXIT_SUCCESS);

		case 'm': stylearg = optarg; break;
		case 'p': prefix = optarg; break;
		case 's': suffix = optarg; break;
		case 'c': nocomment = 1; break;

		default:
			exit(EXIT_FAILURE);
		}
	}

	if (optind < argc-1) {
		fprintf(stderr, PROG_NAME ": no filename given\n");
		exit(EXIT_FAILURE);
	} else if (optind > argc-1) {
		fprintf(stderr, PROG_NAME ": too many arguments given\n");
		exit(EXIT_FAILURE);
	}

	filename = argv[optind];

	if (!stylearg) stylearg = "global";
	style = find_module_style(stylearg);
	if (!style) {
		fprintf(stderr, PROG_NAME ": unknown module style: %s\n",
			stylearg);
		exit(EXIT_FAILURE);
	}

	in = fopen(filename, "r");
	if (!in) {
		perror(PROG_NAME);
		exit(EXIT_FAILURE);
	}

	if (prefix) printf("%s", prefix);
	else        printf(style->prefix_fmt, filename);

	printf("new Uint32Array([");
	if (!nocomment) {
		printf("\n\t/* " PROG_NAME "-" PROG_VER_STR " */");
	}

	i = 0;
	while ((c = fgetc(in)) != EOF) {
		n = c << 24;
		if ((c = fgetc(in)) != EOF) n |= c << 16;
		if ((c = fgetc(in)) != EOF) n |= c << 8;
		if ((c = fgetc(in)) != EOF) n |= c;

		     if (i%6) printf(   ", 0x%08X", (int)htonl(n));
		else if (i)   printf(",\n\t0x%08X", (int)htonl(n));
		else          printf( "\n\t0x%08X", (int)htonl(n));

		i++;
	}

	if (ferror(in)) {
		perror("file-js");
		fclose(in);
		exit(EXIT_FAILURE);
	}

	len = ftell(in);
	if (len % 4) printf("\n]).buffer.slice(0, %ld)", len);
	else         printf("\n]).buffer");

	printf("%s\n", suffix ? suffix : style->tail);
	fclose(in);
	return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
"  -s <suffix>  Text to append to the output\n\n"                \
"Supported module styles:\n"                                     \
"  global    ; window['<file>'] = ...;\n"                        \
"  amd       ; define('<file>', function() { return  ...; });\n" \
"  commonjs  module.exports = ...;\n"                            \
"  es6       export default ...;\n"                              \
"  none      ...\n"

static void require_unset(const void *val, const char *name)
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

int main(int argc, const char *argv[])
{
	const char *filename = NULL;
	const char *prefix = NULL;
	const char *suffix = NULL;
	const char *stylearg = NULL;
	const struct module_style *style;
	int i, c, n;
	long len;
	FILE *in;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
			printf(PROG_VER_STR "\n");
			return 0;
		} else if (!strcmp(argv[i], "-m")) {
			require_unset(stylearg, "module style (-m)");
			require_arg(i, argc, "-m");
			stylearg = argv[++i];
		} else if (!strcmp(argv[i], "-p")) {
			require_unset(prefix, "prefix (-p)");
			require_arg(i, argc, "-p");
			prefix = argv[++i];
		} else if (!strcmp(argv[i], "-s")) {
			require_unset(suffix, "suffix (-s)");
			require_arg(i, argc, "-s");
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

	if (!stylearg) stylearg = "global";
	style = find_module_style(stylearg);
	if (!style) {
		fprintf(stderr, PROG_NAME ": unknown module style: %s\n",
			stylearg);
		exit(EXIT_FAILURE);
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

	if (prefix) printf("%s", prefix);
	else        printf(style->prefix_fmt, filename);

	printf("new Uint32Array([");

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

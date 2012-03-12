#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH    255

int main(char *argc, char **argv) {
    char filename[MAX_PATH];
    FILE* f;
    char svn_rev[16];
    char out_str[4096];
    errno_t err;

    char out_header[1024] = 
        "// GENERATED - Do not edit!\n"
        "#ifndef VERSION_H_\n"
        "#define VERSION_H_\n";
    char out_footer[1024] = 
        "#define VERSION __BUILD__\n"
        "#endif // VERSION_H_\n";

    // Check for args
    if (argv[1] == NULL) {
        fprintf(stderr, "ERROR an input project root directory must be specified\n");
        exit(1);
    }
    // An output
    if (argv[2] == NULL) {
        fprintf(stderr, "ERROR an output version file must be specified\n");
        exit(1);
    }
    sprintf_s(filename, sizeof(filename), "%s\\.svn\\entries", argv[1]);

    // Get SVN rev
    if ((err = fopen_s(&f, filename, "r")) == 0) {
        fseek(f, 0, SEEK_SET);
        fscanf_s(f, "%s\n", svn_rev, sizeof(svn_rev));
        fscanf_s(f, "%s\n", svn_rev, sizeof(svn_rev));
        fscanf_s(f, "%s\n", svn_rev, sizeof(svn_rev));
        fclose(f);
    } else {
        sprintf_s(svn_rev, sizeof(svn_rev), "unk");
    }
    // Build header
    sprintf_s(out_str, sizeof(out_str), "%s#define __BUILD__ \"%s\"\n%s", out_header, 
        svn_rev, out_footer);

    // Write header out
    if ((err = fopen_s(&f, argv[2], "w")) == 0) {
        fwrite(out_str, strlen(out_str), 1, f);
        fclose(f);
    } else {
        fprintf(stderr, "ERROR unable to write version.h\n");
        exit(1);
    }
    return 0;
}
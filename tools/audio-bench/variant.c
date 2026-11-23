/* SPDX-License-Identifier: GPL-3.0-only */
#include "felucca_web.c"

static int read_project(const char *path, project_t *project)
{
    project_store_t packed;
    FILE *file = fopen(path, "rb");
    if (!file) return 0;
    int ok = fread(&packed, 1, sizeof packed, file) == sizeof packed
             && fgetc(file) == EOF && proj_import(project, &packed, sizeof packed);
    fclose(file);
    return ok;
}

static int number(const char *input, int min, int max)
{
    char *end;
    long value = strtol(input, &end, 10);
    return end != input && *end == '\0' && value >= min && value <= max ? (int)value : -1;
}

int main(int argc, char **argv)
{
    static project_t subject, control, decoded;
    project_store_t packed;

    if (argc < 5 || !read_project(argv[2], &subject))
        goto usage;

    if (!strcmp(argv[1], "vocoder") && (argc == 5 || argc == 6)) {
        int track = number(argv[3], 1, NTRK);
        int carrier = number(argv[4], 0, NTRK);
        int bands = argc == 6 ? number(argv[5], 0, 2) : -1;
        if (track < 0 || carrier < 0 || carrier == track || (argc == 6 && bands < 0))
            goto usage;
        subject.t[track - 1].p[P_VCARR] = carrier;
        if (argc == 6)
            subject.t[track - 1].p[P_VBANDS] = bands;
    } else if (!strcmp(argv[1], "split") && argc == 5 && read_project(argv[3], &control)) {
        memcpy(subject.g, control.g, sizeof subject.g);
        /* The mask selects tracks to replace with the control; bit 0 is track 1. */
        int mask = number(argv[4], 0, (1 << NTRK) - 1);
        if (mask < 0)
            goto usage;
        for (unsigned i = 0; i < NTRK; ++i) {
            if (!(mask & (1 << i))) continue;
            subject.t[i] = control.t[i];
            memcpy(subject.fm6[i], control.fm6[i], sizeof subject.fm6[i]);
            memcpy(subject.vocal_id[i], control.vocal_id[i], sizeof subject.vocal_id[i]);
            subject.mch[i] = control.mch[i];
        }
    } else {
        goto usage;
    }

    subject.sum = proj_sum(&subject);
    if (!proj_pack(&packed, &subject) || !proj_import(&decoded, &packed, sizeof packed)
        || memcmp(&decoded, &subject, sizeof subject)) {
        fputs("B6S1 output did not round-trip\n", stderr);
        return 1;
    }
    return fwrite(&packed, 1, sizeof packed, stdout) != sizeof packed || fflush(stdout) != 0;

usage:
    fputs("Usage: variant vocoder INPUT OWNER CARRIER [BAND_INDEX]\n"
          "       variant split INPUT CONTROL REPLACEMENT_MASK\n", stderr);
    return 2;
}

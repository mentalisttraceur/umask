/* SPDX-License-Identifier: 0BSD */
/* Copyright 2019 Alexander Kozhevnikov <mentalisttraceur@gmail.com> */

/* Standard C library headers */
#include <errno.h> /* errno */
#include <stdio.h> /* EOF, fputc, fputs, perror, stderr, stdout */
#include <stdlib.h> /* EXIT_FAILURE, EXIT_SUCCESS */
#include <string.h> /* strcmp */

/* Standard UNIX/Linux (POSIX/SUS base) headers */
#include <sys/stat.h> /* umask */
#include <sys/types.h> /* mode_t */
#include <unistd.h> /* execvp */


char const version_text[] = "umaskexec 1.0.0\n";

char const help_text_prefix[] = "Usage: ";
char const help_text[] =
    " [OPTION] MASK COMMAND [ARGUMENT]...\n"
    "\n"
    "Execute a command with the given umask, which can be specified in\n"
    "in octal or symbolically. If no umask is given, print the current\n"
    "umask. If no command is given, print what the new umask would be.\n"
    "\n"
    "  -h, --help     Print this help text and exit.\n"
    "  -V, --version  Print version information and exit.\n"
    "  -S, --symbolic Print the umask symbolically instead of in octal.\n"
;


static
int error_bad_option(char * option, char * arg0)
{
    if(fputs(arg0, stderr) != EOF
    && fputs(": bad option: ", stderr) != EOF
    && fputs(option, stderr) != EOF)
    {
        fputc('\n', stderr);
    }
    return EXIT_FAILURE;
}


static
int error_writing_output(char * arg0)
{
    int errno_ = errno;
    if(fputs(arg0, stderr) != EOF)
    {
        errno = errno_;
        perror(": error writing output");
    }
    return EXIT_FAILURE;
}


static
int error_bad_umask(char * umask_string, char * arg0)
{
    if(fputs(arg0, stderr) != EOF
    && fputs(": bad umask: ", stderr) != EOF
    && fputs(umask_string, stderr) != EOF)
    {
        fputc('\n', stderr);
    }
    return EXIT_FAILURE;
}


static
int error_executing_command(char * command, char * arg0)
{
    int errno_ = errno;
    if(fputs(arg0, stderr) != EOF
    && fputs(": error executing command: ", stderr) != EOF)
    {
        errno = errno_;
        perror(command);
    }
    return EXIT_FAILURE;
}


static
int print_help(char * arg0)
{
    if(fputs(help_text_prefix, stdout) != EOF
    && fputs(arg0, stdout) != EOF
    && fputs(help_text, stdout) != EOF
    && fflush(stdout) != EOF)
    {
        return EXIT_SUCCESS;
    }
    return error_writing_output(arg0);
}


static
int print_version(char * arg0)
{
    if(fputs(version_text, stdout) != EOF
    && fflush(stdout) != EOF)
    {
        return EXIT_SUCCESS;
    }
    return error_writing_output(arg0);
}


static
int print_umask_octal(char * arg0)
{
    char mask_string[] = "0000\n";
    mode_t mask = umask(0);
    mask_string[1] += 7 & (mask >> 6);
    mask_string[2] += 7 & (mask >> 3);
    mask_string[3] += 7 & (mask >> 0);

    if(fputs(mask_string, stdout) != EOF
    && fflush(stdout) != EOF)
    {
        return EXIT_SUCCESS;
    }
    return error_writing_output(arg0);
}


static
int print_umask_symbolic(char * arg0)
{
    char mask_string[sizeof("u=rwx,g=rwx,o=rwx\n")];
    char * ptr = mask_string;
    mode_t mask = umask(0);

    *ptr++ = 'u';
    *ptr++ = '=';
    if(!(mask & S_IRUSR)) *ptr++ = 'r';
    if(!(mask & S_IWUSR)) *ptr++ = 'w';
    if(!(mask & S_IXUSR)) *ptr++ = 'x';
    *ptr++ = ',';
    *ptr++ = 'g';
    *ptr++ = '=';
    if(!(mask & S_IRGRP)) *ptr++ = 'r';
    if(!(mask & S_IWGRP)) *ptr++ = 'w';
    if(!(mask & S_IXGRP)) *ptr++ = 'x';
    *ptr++ = ',';
    *ptr++ = 'o';
    *ptr++ = '=';
    if(!(mask & S_IROTH)) *ptr++ = 'r';
    if(!(mask & S_IWOTH)) *ptr++ = 'w';
    if(!(mask & S_IXOTH)) *ptr++ = 'x';
    *ptr++ = '\n';
    *ptr = '\0';

    if(fputs(mask_string, stdout) != EOF
    && fflush(stdout) != EOF)
    {
        return EXIT_SUCCESS;
    }
    return error_writing_output(arg0);
}


static
int parse_and_set_octal_umask(char * mask_string)
{
    mode_t new_mask = 0;
    char c = *mask_string;
    do
    {
        if(c < '0' || c > '7')
        {
            return 0;
        }
        new_mask <<= 3;
        new_mask += c - '0';
        if(new_mask > 0777)
        {
            return 0;
        }
        mask_string += 1;
        c = *mask_string;
    }
    while(c != '\0');
    umask(new_mask);
    return 1;
}


static
int parse_and_set_symbolic_umask(char * mask_string)
{
    /* umask bits combined by permission type: */
    mode_t static const r_bits = S_IRUSR | S_IRGRP | S_IROTH;
    mode_t static const w_bits = S_IWUSR | S_IWGRP | S_IWOTH;
    mode_t static const x_bits = S_IXUSR | S_IXGRP | S_IXOTH;

    /* umask bits combined by who they apply to: */
    mode_t static const u_bits = S_IRUSR | S_IWUSR | S_IXUSR;
    mode_t static const g_bits = S_IRGRP | S_IWGRP | S_IXGRP;
    mode_t static const o_bits = S_IROTH | S_IWOTH | S_IXOTH;
    mode_t static const a_bits = u_bits | g_bits | o_bits;

    /* new umask starts as the old umask, then is updated: */
    mode_t new_mask = umask(0);

    for(;;)
    {
        mode_t u_g_o_a_target = 0;
        char c;

        for(;;)
        {
            c = *mask_string++;
            if(c == 'u') u_g_o_a_target |= u_bits;
            else
            if(c == 'g') u_g_o_a_target |= g_bits;
            else
            if(c == 'o') u_g_o_a_target |= o_bits;
            else
            if(c == 'a') u_g_o_a_target |= a_bits;
            else
            break;
        }

        if(!u_g_o_a_target)
        {
            u_g_o_a_target = a_bits;
        }

        do
        {
            int inverted = 0;

            switch(c)
            {
                case '=':
                    new_mask |= u_g_o_a_target;
                    /* '=' is the same as '+' after setting the bits */
                case '+':
                    new_mask = ~new_mask;
                    inverted = 1;
                    /* '+' is the same as '-' once the mask is inverted */
                case '-':
                    break;
                default:
                    return 0;
            }

            for(;;)
            {
                c = *mask_string++;

                /* Symbolic umask '-' sets bits in binary umask. '+' and '=' */
                /* clear bits, which is setting bits on the inverted mask. */

                if(c == 'r') new_mask |= r_bits & u_g_o_a_target;
                else
                if(c == 'w') new_mask |= w_bits & u_g_o_a_target;
                else
                if(c == 'x') new_mask |= x_bits & u_g_o_a_target;
                else
                break;
            }

            if(inverted)
            {
                new_mask = ~new_mask;
            }

            if(c == '\0')
            {
                umask(new_mask);
                return 1;
            }
        }
        while(c != ',');
    }
}


static
int parse_and_set_umask(char * mask_string)
{
    if(parse_and_set_octal_umask(mask_string)
    || parse_and_set_symbolic_umask(mask_string))
    {
        return 1;
    }
    return 0;
}


int main(int argc, char * * argv)
{
    char * arg;
    char * arg0 = *argv;

    /* Function pointer holds octal or symbolic umask printing choice: */
    int (* print_umask)(char * arg0) = print_umask_octal;

    /* Without any arguments (two, counting argv[0]), just print the umask: */
    if(argc < 2)
    {
        /* Many systems allow execution without even the zeroth argument: */
        if(!arg0)
        {
            arg0 = "";
        }
        return print_umask(arg0);
    }

    /* The goal is to shift argv until it points to the command to execute: */
    argv += 1;

    /* First argument is either an option (starts with '-') or the umask: */
    arg = *argv;

    if(*arg == '-')
    {
        arg += 1;
        if(!strcmp(arg, "-help") || !strcmp(arg, "h"))
        {
            return print_help(arg0);
        }
        if(!strcmp(arg, "-version") || !strcmp(arg, "V"))
        {
            return print_version(arg0);
        }

        if(!strcmp(arg, "-symbolic") || !strcmp(arg, "S"))
        {
            print_umask = print_umask_symbolic;
        }
        else
        /* If it is *not* the "end of options" ("--") "option": */
        if(strcmp(arg, "-"))
        {
            return error_bad_option(arg - 1, arg0);
        }

        /* Shift argv past the consumed option, leaving the umask: */
        argv += 1;
        arg = *argv;

        /* No more arguments after parsing options? Print the umask: */
        if(!arg)
        {
            return print_umask(arg0);
        }
    }

    /* Now arg should be the umask. */

    if(!parse_and_set_umask(arg))
    {
        return error_bad_umask(arg, arg0);
    }

    /* Shift argv past the umask, leaving just the command in argv: */
    argv += 1;

    arg = *argv;
    /* Now arg should be the command to execute. */

    if(!arg)
    {
        /* If no command was given, just print the new umask: */
        return print_umask(arg0);
    }

    execvp(arg, argv);
    /* If we're here, execvp failed to execute the command. */

    return error_executing_command(arg, arg0);
}

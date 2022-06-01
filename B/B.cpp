#include <cstdio>
#include <cstdarg>
#include <memory>
#include <vector>
#include <set>

//#define N_CHAR_IN_WORD (10000)

#define NAME_PROBLEM "epsilon"

constexpr char name_input_file[] = NAME_PROBLEM ".in";
constexpr char name_output_file[] = NAME_PROBLEM ".out";

using namespace std;

enum class Errors: int
{
    SUCCESS,
    INPUT,
    OUTPUT,
    ALLOCATE,
    REALLOCATE,
    NON_COR_SYMB,
    STR_FULL
};

int error_handler(enum Errors e, ...)
{
    va_list args;
    va_start(args, e);
    switch (e) {
        case Errors::SUCCESS: break;
        case Errors::INPUT:         vprintf("Error, unable to open input file \"%s\"\n",    args); break;
        case Errors::OUTPUT:        vprintf("Error, unable to create output file \"%s\"\n", args); break;
        case Errors::ALLOCATE:      vprintf("Error, failed to allocate memory\n",           args); break;
        case Errors::REALLOCATE:    vprintf("Error, failed to reallocate memory\n",         args); break;
        case Errors::NON_COR_SYMB:  vprintf("Error, char in word not correct \"%c\"\n",     args); break;
        case Errors::STR_FULL:      vprintf("Error, word is too long\n",                    args); break;
    }
    va_end(args);

    return (int)e;
}

class FSG //finite state grammar
{
private:
//    static constexpr char first_symbol = 'a';
//    static constexpr char last_symbol = 'z';
//    static constexpr char Alphabet_size = (last_symbol + 1) - first_symbol;

    static constexpr char first_non_terminal = 'A';
    static constexpr char last_non_terminal = 'Z';
    static constexpr char Non_terminal_size = (last_non_terminal + 1) - first_non_terminal;
//    static constexpr char EPS = '!';
//    static constexpr char NON_TRANSITION = 0;
    struct State {
        vector<set<char>> transitions_next;
    };
    State states[Non_terminal_size];
    char start_non_terminal;
    set <char> eps_states;


public:
    explicit FSG(char start_non_terminal) : start_non_terminal(start_non_terminal) {}

    void add_transition(char A, set<char> C) {
        states[A - first_non_terminal].transitions_next.push_back(C);
    }

    void add_transition(char A) {
        eps_states.insert(A);
    }

    void print_eps(FILE *out)
    {
//        for(char i = 0; i < Non_terminal_size; i++)
//            for(auto &tr : states[i].transitions_next)
//            {
//                printf("rule - \"%c -> ", i + first_non_terminal);
//                for(auto C : tr)
//                    printf("%c", C);
//                printf("\"\n");
//            }

        for(char i = 0; i < Non_terminal_size; i++)
        {
            if(eps_states.find((char)(i + first_non_terminal)) == eps_states.end())
            {
                for(auto &tr : states[i].transitions_next)
                {
                    bool res_tr = true;
                    for(auto next : tr)
                    {
                        if(eps_states.find(next) == eps_states.end())
                            res_tr = false;
                    }
                    if(res_tr) {eps_states.insert((char)(i + first_non_terminal)); i = 0 - 1; break;}
                }
            }
        }

        for(auto cur : eps_states)
            fprintf(out, "%c ", cur);
    }
};

FSG* scanFSG(FILE *in, int *error)
{
    size_t n;
    char S;
    fscanf(in, "%zu %c\n", &n, &S);
    //printf("n = %zu, S = \'%c\'\n", n, S);

    FSG *fsg = new (nothrow) FSG(S);
    if(!fsg) {*error = error_handler(Errors::ALLOCATE); return nullptr;}

    while(n--)
    {
        char A;
        fscanf(in, "%c ->", &A);

        char N;
        int i = 0;
        int j = 0;
        set<char> tr;
        while(fscanf(in, "%c", &N) != EOF)
        {
            if (N == '\n')
                break;
            else if (N >= 'a' && N <= 'z')
            {
                j++;
                continue;
            }
            else if(N == ' ' || N == '\t')
                continue;
            else if(N >= 'A' && N <= 'Z')
            {
                tr.insert(N);
                i++;
            }
        }

        if(j == 0)
        {
            if (i != 0)
                fsg->add_transition(A, tr);
            else
                fsg->add_transition(A);
        }
//
//        printf("s rule - \"%c -> ", A);
//        for(auto C : tr)
//        {
//            printf("%c", C);
//        }
//        printf("\"\n");
    }

    return fsg;
}

//int scan_word(FILE *in, char *data)
//{
//    char buffer;
//    size_t n;
//    for(n = 0; fscanf(in, "%c", &buffer) != EOF && n < N_CHAR_IN_WORD; n++)
//    {
//        if(buffer == '\n' || buffer == ' ' || buffer == '\r' || buffer == '\t')
//            break;
//        if(buffer < 'a' || buffer > 'z')
//            return error_handler(Errors::NON_COR_SYMB, buffer);
//
//        data[n] = buffer;
//    }
//    if(n >= N_CHAR_IN_WORD && !(buffer == '\n' || buffer == ' ' || buffer == '\r' || buffer == '\t'))
//        return error_handler(Errors::STR_FULL, buffer);
//
//    data[n] = '\0';
//
//    //printf("word - \"%s\" - ", data);
//    return error_handler(Errors::SUCCESS);
//}

int main() {

    unique_ptr<FILE, int (*)(FILE *)> in(fopen(name_input_file, "r"), fclose);
    if (!in) return error_handler(Errors::INPUT, name_input_file);

    int result = 0;
    unique_ptr<FSG> fsg(scanFSG(in.get(), &result));

    if (!fsg)
        return result;

    unique_ptr<FILE, int (*)(FILE *)> out(fopen(name_output_file, "w"), fclose);
    if (!out) return error_handler(Errors::OUTPUT, name_output_file);

    fsg->print_eps(out.get());

    return error_handler(Errors::SUCCESS);
}
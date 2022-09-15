#include <cstdio>
#include <cstdarg>
#include <memory>
#include <vector>
#include <set>

//#define N_CHAR_IN_WORD (10000)

#define NAME_PROBLEM "useless"

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
        bool isSpecified = false;
        bool isGenerating = false;
        bool isReachable = false;
        set<set<char>> transitions_next;
    };
    State states[Non_terminal_size];
    char start_non_terminal;
    //set <char> eps_states;
    set <char> generating_states;
    set <char> reachable_states;

public:
    explicit FSG(char start_non_terminal) : start_non_terminal(start_non_terminal) {}

    void add_transition(char A, set<char> C) {
        states[A - first_non_terminal].transitions_next.insert(C);
    }

    void add_transition(char A, bool isEps) {
//        if(isEps)
//            generating_states.insert(A);
//        else
//            generating_states.insert(A);

        generating_states.insert(A);
    }

    void addSpecified(char A)
    {
        states[A - first_non_terminal].isSpecified = true;
    }

//    void dfs(char cur)
//    {
//        if(states[cur - first_non_terminal].isReachable)
//            return;
//
//        states[cur - first_non_terminal].isReachable = true;
//
//        for(set<char> &p : states[cur - first_non_terminal].transitions_next)
//        {
//            for(char next : p)
//            {
//                dfs(next);
//            }
//        }
//    }

    void print_useless(FILE *out)
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
            if(generating_states.find((char)(i + first_non_terminal)) == generating_states.end())
            {
                for(auto &tr : states[i].transitions_next)
                {
                    bool res_tr = true;
                    for(auto next : tr)
                    {
                        if(generating_states.find(next) == generating_states.end())
                            res_tr = false;
                    }
                    if(res_tr) {generating_states.insert((char)(i + first_non_terminal)); i = 0 - 1; break;}
                }
            }
        }

        for(auto cur : generating_states)
        {
           states[cur - first_non_terminal].isGenerating = true;
        }

        for(char i = 0; i < Non_terminal_size; i++)
        {
            for(auto &tr : states[i].transitions_next)
            {
                bool res_tr = true;
                for(auto next : tr)
                {
                    if(generating_states.find(next) == generating_states.end())
                        res_tr = false;
                }
                if(!res_tr) {states[i].transitions_next.erase(tr); i--; break;}
            }

        }

        reachable_states.insert(start_non_terminal);
        for(char i = 0; i < Non_terminal_size; i++)
        {
            if(reachable_states.find((char)(i + first_non_terminal)) != reachable_states.end())
            {
                for(auto &tr : states[i].transitions_next)
                {
                    bool res_tr = false;
                    for(auto next : tr)
                    {
                        if(reachable_states.find(next) == reachable_states.end())
                        {
                            reachable_states.insert((char)(next));
                            res_tr = true;
                        }
                    }
                    if(res_tr) {i = 0 - 1; break;}
                }
            }
        }

        for(auto cur : reachable_states)
        {
            states[cur - first_non_terminal].isReachable = true;
        }

        for(char i = first_non_terminal; i <= last_non_terminal; i++)
        {
            //printf("%c - %d %d %d\n", i, states[i - first_non_terminal].isSpecified, states[i - first_non_terminal].isGenerating, states[i - first_non_terminal].isReachable);
            if(states[i - first_non_terminal].isSpecified)
            {
                if(!states[i - first_non_terminal].isGenerating || !states[i - first_non_terminal].isReachable)
                    fprintf(out, "%c ", i);
            }
        }
    }

    void print()
    {
        for(char i = first_non_terminal; i <= last_non_terminal; i++)
        {
            printf("%c - %d %d %d\n", i, states[i - first_non_terminal].isSpecified, states[i - first_non_terminal].isGenerating, states[i - first_non_terminal].isReachable);
        }
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

    fsg->addSpecified(S);

    while(n--)
    {
        char A;
        fscanf(in, "%c ->", &A);

        fsg->addSpecified(A);

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
                fsg->addSpecified(N);
                tr.insert(N);
                i++;
            }
        }

        if(i != 0)
        {
            fsg->add_transition(A, tr);
        }
        else
        {
            if(j == 0)
            {
                fsg->add_transition(A, true);
            }
            else
            {
                fsg->add_transition(A, false);
            }
        }
    }

    return fsg;
}

int main() {

    unique_ptr<FILE, int (*)(FILE *)> in(fopen(name_input_file, "r"), fclose);
    if (!in) return error_handler(Errors::INPUT, name_input_file);

    int result = 0;
    unique_ptr<FSG> fsg(scanFSG(in.get(), &result));

    if (!fsg)
        return result;

    unique_ptr<FILE, int (*)(FILE *)> out(fopen(name_output_file, "w"), fclose);
    if (!out) return error_handler(Errors::OUTPUT, name_output_file);

    fsg->print_useless(out.get());

    return error_handler(Errors::SUCCESS);
}
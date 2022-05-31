#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <memory>
#include <vector>
#include <set>

#define N_CHAR_IN_WORD (10000)

#define NAME_PROBLEM "automaton"

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
    static constexpr char first_symbol = 'a';
    static constexpr char last_symbol = 'z';
    static constexpr char Alphabet_size = (last_symbol + 1) - first_symbol;

    static constexpr char first_non_terminal = 'A';
    static constexpr char last_non_terminal = 'Z';
    static constexpr char Non_terminal_size = (last_non_terminal + 1) - first_non_terminal;
    static constexpr char EPS = '!';
    static constexpr char NON_TRANSITION = 0;
    struct State {
        vector<char> transitions_next[Alphabet_size];
    };
    State states[Non_terminal_size];
    char start_non_terminal;


public:
    explicit FSG(char start_non_terminal) : start_non_terminal(start_non_terminal) {}

    void add_transition(char A, char C, char s) {
        states[A - first_non_terminal].transitions_next[s - first_symbol].push_back(C);
    }
    void add_transition(char A, char s) {
        states[A - first_non_terminal].transitions_next[s - first_symbol].push_back((char)FSG::EPS);
    }

    bool check_word(const char *word)
    {
        auto *cur_states = new set<unsigned char>;
        cur_states->insert(start_non_terminal);

        for(size_t i = 0; word[i] != '\0'; i++)
        {
            if(cur_states->empty()) {delete cur_states; return false;}
            auto *new_cur_states = new set<unsigned char>;
            for(size_t cur_state : *cur_states)
            {
                if(cur_state == FSG::EPS)
                    continue;
                for(auto & k : states[cur_state - first_non_terminal].transitions_next[word[i] - first_symbol])
                {
                    new_cur_states->insert(k);
                }
            }
            delete cur_states;
            cur_states = new_cur_states;
        }
        for(size_t cur_state : *cur_states)
            if(cur_state == FSG::EPS) {delete cur_states; return true;}

        delete cur_states;
        return false;
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
        char A, b, C;
        fscanf(in, "%c -> %c%c", &A, &b, &C);
        //printf("rule - \"%c -> %c%c\"\n", A, b, C);
        if(C == '\n' || C == ' ' || C == '\t')
            fsg->add_transition(A, b);
        else
        {
            fsg->add_transition(A, C, b);
            fscanf(in, "\n");
        }
    }

    return fsg;
}

int scan_word(FILE *in, char *data)
{
    char buffer;
    size_t n;
    for(n = 0; fscanf(in, "%c", &buffer) != EOF && n < N_CHAR_IN_WORD; n++)
    {
        if(buffer == '\n' || buffer == ' ' || buffer == '\r' || buffer == '\t')
            break;
        if(buffer < 'a' || buffer > 'z')
            return error_handler(Errors::NON_COR_SYMB, buffer);

        data[n] = buffer;
    }
    if(n >= N_CHAR_IN_WORD && !(buffer == '\n' || buffer == ' ' || buffer == '\r' || buffer == '\t'))
        return error_handler(Errors::STR_FULL, buffer);

    data[n] = '\0';

    //printf("word - \"%s\" - ", data);
    return error_handler(Errors::SUCCESS);
}

int main() {

    unique_ptr<FILE, int (*)(FILE *)> in(fopen(name_input_file, "r"), fclose);
    if (!in) return error_handler(Errors::INPUT, name_input_file);

    int result = 0;
    unique_ptr<FSG> fsg(scanFSG(in.get(), &result));

    if (!fsg)
        return result;


    size_t n_words;
    fscanf(in.get(), "%zu\n", &n_words);
    //printf("n_words = %zu\n", n_words);


    unique_ptr<FILE, int (*)(FILE *)> out(fopen(name_output_file, "w"), fclose);
    if (!out) return error_handler(Errors::OUTPUT, name_output_file);

    char *data = (char*)malloc((N_CHAR_IN_WORD + 1) * sizeof(char));
    if(!data) return error_handler(Errors::ALLOCATE);
    while(n_words--)
    {
        result = scan_word(in.get(), data);
        if(result) {free(data); return result;}
        if(fsg->check_word(data))
        {
            fprintf(out.get(), "yes\n");
            //printf("yes\n");
        }
        else
        {
            fprintf(out.get(), "no\n");
            //printf("no\n");
        }
    }
    free(data);

    return error_handler(Errors::SUCCESS);
}
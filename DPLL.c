#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include <direct.h>
#define getcwd _getcwd  


#define MAX_VARS 1000
#define MAX_CLAUSES 10000

typedef struct {
    int num_vars;
    int num_clauses;
    int clauses[MAX_CLAUSES][MAX_VARS];
} CNF;

typedef struct {
    int assignment[MAX_VARS + 1]; 
    int trail[MAX_VARS];
    int trail_size;
} SolverState;

void initialize_solver(SolverState* state);
CNF* create_cnf();
void free_cnf(CNF* cnf);
void parse_cnf(const char* filename, CNF* cnf);
bool unit_propagate(CNF* cnf, SolverState* state);
bool dpll(CNF* cnf, SolverState* state);
void print_solution(CNF* cnf, SolverState* state);

int main() {
    const char* filename = "test_dpll.cnf";

    FILE* file_test = fopen(filename, "r");
    if (!file_test) {
        fprintf(stderr, "Eroare: Fisierul '%s' nu exista in directorul:\n", filename);

        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            fprintf(stderr, "Director curent: %s\n", cwd);
        }
        else 

            fprintf(stderr, "Continut director:\n");

        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile(".\\*", &findFileData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                fprintf(stderr, "%s\n", findFileData.cFileName);
            } while (FindNextFile(hFind, &findFileData) != 0);
            FindClose(hFind);
        }
        else {
            fprintf(stderr, "Eroare la listarea fisierelor din director.\n");
        }

        return EXIT_FAILURE;
    }
    fclose(file_test);

    CNF* cnf = create_cnf();
    SolverState state;
    initialize_solver(&state);

    parse_cnf(filename, cnf);
    printf("Problema contine %d variabile si %d clauze\n", cnf->num_vars, cnf->num_clauses);

    bool result = dpll(cnf, &state);

    if (result) {
        printf("\nRezultat: SATISFIABILA\n");
        print_solution(cnf, &state);
    }
    else {
        printf("\nRezultat: NESATISFIABILA\n");
    }

    free_cnf(cnf);
    return EXIT_SUCCESS;
}

void initialize_solver(SolverState* state) {
    memset(state, 0, sizeof(SolverState));
}

CNF* create_cnf() {
    CNF* cnf = (CNF*)calloc(1, sizeof(CNF));
    if (!cnf) {
        fprintf(stderr, "Eroare alocare memorie CNF\n");
        exit(EXIT_FAILURE);
    }
    return cnf;
}

void free_cnf(CNF* cnf) {
    free(cnf);
}

void parse_cnf(const char* filename, CNF* cnf) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Eroare deschidere fisier CNF");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    bool header_found = false;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'c') continue;
        if (line[0] == 'p') {
            if (sscanf(line, "p cnf %d %d", &cnf->num_vars, &cnf->num_clauses) != 2) {
                fprintf(stderr, "Eroare format: linia antet trebuie sa fie 'p cnf nr_variabile nr_clauze'\n");
                fprintf(stderr, "Linia problematica: %s", line);
                exit(EXIT_FAILURE);
            }
            header_found = true;
            continue;
        }

        if (!header_found) {
            fprintf(stderr, "Eroare: Antetul 'p cnf' lipseate sau este dupa clauze\n");
            exit(EXIT_FAILURE);
        }
        int lit, idx = 0;
        char* token = strtok(line, " \t\n");

        while (token != NULL) {
            lit = atoi(token);
            if (lit == 0) break; 

            if (abs(lit) > cnf->num_vars) {
                fprintf(stderr, "Eroare: Variabila %d depaseste numarul declarat de variabile (%d)\n",
                    abs(lit), cnf->num_vars);
                exit(EXIT_FAILURE);
            }

            if (idx >= MAX_VARS - 1) {
                fprintf(stderr, "Eroare: Clauza depaseste lungimea maxima (%d literale)\n", MAX_VARS);
                exit(EXIT_FAILURE);
            }

            cnf->clauses[cnf->num_clauses][idx++] = lit;
            token = strtok(NULL, " \t\n");
        }

        if (idx == 0) continue;

        cnf->clauses[cnf->num_clauses][idx] = 0;
        cnf->num_clauses++;

        if (cnf->num_clauses >= MAX_CLAUSES) {
            fprintf(stderr, "Eroare: Depasire numar maxim de clauze (%d)\n", MAX_CLAUSES);
            exit(EXIT_FAILURE);
        }
    }

    fclose(file);
}

bool is_clause_satisfied(CNF* cnf, int clause_idx, SolverState* state) {
    for (int i = 0; cnf->clauses[clause_idx][i] != 0; i++) {
        int lit = cnf->clauses[clause_idx][i];
        if (state->assignment[abs(lit)] * lit > 0) {
            return true;
        }
    }
    return false;
}

bool unit_propagate(CNF* cnf, SolverState* state) {
    bool changed;
    do {
        changed = false;
        for (int i = 0; i < cnf->num_clauses; i++) {
            if (is_clause_satisfied(cnf, i, state)) continue;

            int unassigned_lit = 0;
            int unassigned_count = 0;

            for (int j = 0; cnf->clauses[i][j] != 0; j++) {
                int lit = cnf->clauses[i][j];
                int var = abs(lit);

                if (state->assignment[var] == 0) {
                    unassigned_lit = lit;
                    unassigned_count++;
                }
                else if (state->assignment[var] * lit > 0) {
                    unassigned_count = 0;
                    break;
                }
            }

            if (unassigned_count == 1) { 
                int var = abs(unassigned_lit);
                state->assignment[var] = (unassigned_lit > 0) ? 1 : -1;
                state->trail[state->trail_size++] = var;
                changed = true;

                printf("Propagare unitara: x%d = %s\n",
                    var, (unassigned_lit > 0) ? "Adevarat" : "Fals");
            }
        }
    } while (changed);

    for (int i = 0; i < cnf->num_clauses; i++) {
        bool has_unassigned = false;
        bool is_conflict = true;

        for (int j = 0; cnf->clauses[i][j] != 0; j++) {
            int lit = cnf->clauses[i][j];
            int var = abs(lit);

            if (state->assignment[var] == 0) {
                has_unassigned = true;
                break;
            }
            else if (state->assignment[var] * lit > 0) {
                is_conflict = false;
                break;
            }
        }

        if (!has_unassigned && is_conflict) {
            printf("Conflict in clauza %d: ", i + 1);
            for (int j = 0; cnf->clauses[i][j] != 0; j++) {
                printf("%d ", cnf->clauses[i][j]);
            }
            printf("\n");
            return false;
        }
    }

    return true;
}

bool dpll(CNF* cnf, SolverState* state) {
    if (!unit_propagate(cnf, state)) {
        return false;
    }

    bool all_satisfied = true;
    for (int i = 0; i < cnf->num_clauses; i++) {
        if (!is_clause_satisfied(cnf, i, state)) {
            all_satisfied = false;
            break;
        }
    }
    if (all_satisfied) return true;
 
    int next_var = 0;
    for (int i = 1; i <= cnf->num_vars; i++) {
        if (state->assignment[i] == 0) {
            next_var = i;
            break;
        }
    }
    if (next_var == 0) return false;

    printf("Decizie: incerc x%d = Adevarat\n", next_var);

  
    SolverState true_state = *state;
    true_state.assignment[next_var] = 1;
    true_state.trail[true_state.trail_size++] = next_var;

    if (dpll(cnf, &true_state)) {
        *state = true_state;
        return true;
    }

    printf("Backtrack: x%d = Adevarat duce la conflict, incerc Fals\n", next_var);

    SolverState false_state = *state;
    false_state.assignment[next_var] = -1;
    false_state.trail[false_state.trail_size++] = next_var;

    if (dpll(cnf, &false_state)) {
        *state = false_state;
        return true;
    }

    return false;
}

void print_solution(CNF* cnf, SolverState* state) {
    printf("\nSolutie gasita:\n");
    for (int i = 1; i <= cnf->num_vars; i++) {
        printf("x%d = %s\n", i, state->assignment[i] > 0 ? "Adevarat" : "Fals");
    }

    printf("\nVerificare clauze:\n");
    for (int i = 0; i < cnf->num_clauses; i++) {
        printf("Clauza %d: [", i + 1);
        for (int j = 0; cnf->clauses[i][j] != 0; j++) {
            printf("%d ", cnf->clauses[i][j]);
        }
        printf("] - %s\n",
            is_clause_satisfied(cnf, i, state) ? "SATISFACUTA" : "NESATISFACUTA");
    }
}

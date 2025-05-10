#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>  

#define MAX_VARS 100
#define MAX_CLAUSES 1000

typedef struct {
    int num_vars;
    int num_clauses;
    int clauses[MAX_CLAUSES][MAX_VARS];
} CNF;

CNF* create_cnf() {
    CNF* cnf = (CNF*)malloc(sizeof(CNF));
    if (!cnf) {
        fprintf(stderr, "Alocarea memoriei a esuat!\n");
        exit(1);
    }
    memset(cnf, 0, sizeof(CNF));
    return cnf;
}

void free_cnf(CNF* cnf) {
    free(cnf);
}

void parse_cnf(const char* filename, CNF* cnf) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Fisierul nu poate fi deschis!");
        exit(1);
    }

    char line[1024];
    bool header_found = false;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'c') continue;
        if (line[0] == 'p') {
            if (sscanf(line, "p cnf %d %d", &cnf->num_vars, &cnf->num_clauses) != 2) {
                fprintf(stderr, "Antet CNF invalid!\n");
                exit(1);
            }
            header_found = true;
            continue;
        }
        if (!header_found) continue;

        int lit, idx = 0;
        char* token = strtok(line, " \t\n");
        while (token) {
            lit = atoi(token);
            if (lit == 0) break;
            if (idx >= MAX_VARS - 1) {
                fprintf(stderr, "Clauza este prea lunga!\n");
                exit(1);
            }
            cnf->clauses[cnf->num_clauses][idx++] = lit;
            token = strtok(NULL, " \t\n");
        }
        cnf->clauses[cnf->num_clauses][idx] = 0;
        cnf->num_clauses++;

        if (cnf->num_clauses >= MAX_CLAUSES) {
            fprintf(stderr, "Prea multe clauze!\n");
            exit(1);
        }
    }
    fclose(file);
}

bool is_unit_clause(int* clause) {
    return clause[0] != 0 && clause[1] == 0;
}

bool contains_literal(int* clause, int lit) {
    for (int i = 0; clause[i] != 0; i++) {
        if (clause[i] == lit) return true;
    }
    return false;
}

void remove_clause(CNF* cnf, int index) {
    if (index < 0 || index >= cnf->num_clauses) return;

    for (int i = index; i < cnf->num_clauses - 1; i++) {
        memcpy(cnf->clauses[i], cnf->clauses[i + 1], MAX_VARS * sizeof(int));
    }
    cnf->num_clauses--;
}

void remove_literal(int* clause, int lit) {
    for (int i = 0; clause[i] != 0; i++) {
        if (clause[i] == lit) {
            for (int j = i; clause[j] != 0; j++) {
                clause[j] = clause[j + 1];
            }
            i--;
        }
    }
}

void apply_resolution(CNF* cnf, int var) {
    CNF new_cnf = *cnf;
    new_cnf.num_clauses = 0;

    for (int i = 0; i < cnf->num_clauses; i++) {
        if (contains_literal(cnf->clauses[i], var)) {
            for (int j = 0; j < cnf->num_clauses; j++) {
                if (contains_literal(cnf->clauses[j], -var)) {
                    int resolvent[MAX_VARS] = { 0 };
                    int k = 0;

                    for (int m = 0; cnf->clauses[i][m] != 0 && k < MAX_VARS - 1; m++) {
                        if (cnf->clauses[i][m] != var) resolvent[k++] = cnf->clauses[i][m];
                    }

                    for (int m = 0; cnf->clauses[j][m] != 0 && k < MAX_VARS - 1; m++) {
                        if (cnf->clauses[j][m] != -var) {
                            bool duplicate = false;
                            for (int n = 0; n < k; n++) {
                                if (resolvent[n] == cnf->clauses[j][m]) {
                                    duplicate = true;
                                    break;
                                }
                            }
                            if (!duplicate) resolvent[k++] = cnf->clauses[j][m];
                        }
                    }
                    resolvent[k] = 0;

                    
                    if (k > 0 && new_cnf.num_clauses < MAX_CLAUSES) {
                        memcpy(new_cnf.clauses[new_cnf.num_clauses], resolvent, (k + 1) * sizeof(int));
                        new_cnf.num_clauses++;
                    }
                }
            }
        }
    }

    for (int i = 0; i < cnf->num_clauses && new_cnf.num_clauses < MAX_CLAUSES; i++) {
        if (!contains_literal(cnf->clauses[i], var) &&
            !contains_literal(cnf->clauses[i], -var)) {
            memcpy(new_cnf.clauses[new_cnf.num_clauses], cnf->clauses[i], MAX_VARS * sizeof(int));
            new_cnf.num_clauses++;
        }
    }

    *cnf = new_cnf;
}


bool davis_putnam(CNF* cnf) {
    while (1) {
      
        bool changed;
        do {
            changed = false;
            for (int i = 0; i < cnf->num_clauses; i++) {
                if (is_unit_clause(cnf->clauses[i])) {
                    int lit = cnf->clauses[i][0];
                    changed = true;

                    for (int j = 0; j < cnf->num_clauses; ) {
                        if (contains_literal(cnf->clauses[j], lit)) {
                            remove_clause(cnf, j);
                        }
                        else {
                            remove_literal(cnf->clauses[j], -lit);
                            j++;
                        }
                    }
                    break;
                }
            }
        } while (changed);

       
        for (int i = 0; i < cnf->num_clauses; i++) {
            if (cnf->clauses[i][0] == 0) return false; 
        }
        if (cnf->num_clauses == 0) return true; 

        int var = abs(cnf->clauses[0][0]);
        apply_resolution(cnf, var);
    }
}


#pragma comment(linker, "/STACK:8388608,4096")

int main() {
    ULONG_PTR stack_size = 8 * 1024 * 1024;
    SetThreadStackGuarantee(&stack_size);

    CNF* cnf = create_cnf();
    parse_cnf("test_dp.cnf", cnf);

    printf("Rezolvare cu %d variabile si %d clauze...\n", cnf->num_vars, cnf->num_clauses);

    bool result = davis_putnam(cnf);
    printf(result ? "SATISFIABIL\n" : "NESATISFIABIL\n");

    free_cnf(cnf);
    return 0;
}

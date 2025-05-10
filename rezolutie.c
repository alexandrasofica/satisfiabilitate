#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_CLAUSES 1000
#define MAX_LITS 100

typedef struct {
    int num_clauses;
    int num_vars;
    int clauses[MAX_CLAUSES][MAX_LITS];
} CNFFormula;
void parse_dimacs(const char* filename, CNFFormula* formula) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Nu pot deschide fisierul CNF");
        exit(1);
    }

    char line[1024];
    formula->num_clauses = 0;
    bool header_found = false;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'c') continue;
        if (line[0] == 'p') {
            sscanf(line, "p cnf %d %d", &formula->num_vars, &formula->num_clauses);
            header_found = true;
            continue;
        }
        if (!header_found) continue;

        int lit, i = 0;
        char* token = strtok(line, " \t\n");
        while (token != NULL) {
            lit = atoi(token);
            if (lit == 0) break;
            formula->clauses[formula->num_clauses][i++] = lit;
            token = strtok(NULL, " \t\n");
        }
        formula->clauses[formula->num_clauses][i] = 0; 
        formula->num_clauses++;
    }

    fclose(file);
}

bool contains_complement(int* cl1, int* cl2, int* out_lit) {
    for (int i = 0; cl1[i] != 0; i++) {
        for (int j = 0; cl2[j] != 0; j++) {
            if (cl1[i] == -cl2[j]) {
                *out_lit = cl1[i];
                return true;
            }
        }
    }
    return false;
}

void resolve(int* cl1, int* cl2, int pivot, int* result) {
    int k = 0;
    for (int i = 0; cl1[i] != 0; i++) {
        if (cl1[i] != pivot) result[k++] = cl1[i];
    }
    for (int j = 0; cl2[j] != 0; j++) {
        if (cl2[j] != -pivot) {
            bool duplicate = false;
            for (int m = 0; m < k; m++) {
                if (result[m] == cl2[j]) {
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate) result[k++] = cl2[j];
        }
    }
    result[k] = 0;
}

bool is_empty_clause(int* clause) {
    return clause[0] == 0;
}

int compare_lits(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

bool same_clause(int* c1, int* c2) {
    int temp1[MAX_LITS], temp2[MAX_LITS];
    int len1 = 0, len2 = 0;

    while (c1[len1] != 0) temp1[len1] = c1[len1], len1++;
    while (c2[len2] != 0) temp2[len2] = c2[len2], len2++;

    if (len1 != len2) return false;

    qsort(temp1, len1, sizeof(int), compare_lits);
    qsort(temp2, len2, sizeof(int), compare_lits);

    for (int i = 0; i < len1; i++) {
        if (temp1[i] != temp2[i]) return false;
    }
    return true;
}

bool clause_exists(CNFFormula* f, int* new_clause) {
    for (int i = 0; i < f->num_clauses; i++) {
        if (same_clause(f->clauses[i], new_clause)) return true;
    }
    return false;
}

bool resolution(CNFFormula* f) {
    int initial_clauses = f->num_clauses;
    int new_clauses = 1;

    while (new_clauses) {
        new_clauses = 0;
        int current_clauses = f->num_clauses;

        for (int i = 0; i < current_clauses; i++) {
            for (int j = i + 1; j < current_clauses; j++) {
                int pivot;
                if (contains_complement(f->clauses[i], f->clauses[j], &pivot)) {
                    int res_clause[MAX_LITS];
                    resolve(f->clauses[i], f->clauses[j], pivot, res_clause);

                    if (is_empty_clause(res_clause)) {
                        printf("S-a obtinut clauza vida. Formula este UNSAT.\n");
                        return false;
                    }

                    if (!clause_exists(f, res_clause)) {
                        if (f->num_clauses >= MAX_CLAUSES) {
                            printf("Numarul maxim de clauze a fost depasit.\n");
                            return true;
                        }
                        memcpy(f->clauses[f->num_clauses], res_clause, sizeof(int) * MAX_LITS);
                        f->num_clauses++;
                        new_clauses++;
                    }
                }
            }
        }
    }

    printf("Nu s-a obtinut clauza vida. Formula este SAT.\n");
    return true;
}

int main() {
    CNFFormula formula = { 0 };

    const char* filename = "test.cnf";
    parse_dimacs(filename, &formula);

    printf("Formula cu %d clauze si %d variabile.\n", formula.num_clauses, formula.num_vars);

    bool result = resolution(&formula);
    if (result)
        printf("SAT\n");
    else
        printf("UNSAT\n");

    return 0;
}

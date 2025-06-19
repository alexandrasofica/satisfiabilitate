README - Soluționare SAT în C (Rezoluție, Davis-Putnam și DPLL)

Descriere
Acest proiect conține două implementări în limbajul C pentru rezolvarea problemelor SAT, reprezentate în format CNF (Conjunctive Normal Form):
1.	Algoritmul de Rezoluție – bazat pe metoda clasică de rezoluție logică.
2.	Algoritmul Davis-Putnam (DP) – algoritmul original DP pentru SAT.
3.	Algoritmul DPLL (Davis–Putnam–Logemann–Loveland) – extensie a algoritmului DP cu propagare unitară și backtracking.
Fiecare algoritm preia formule SAT în format DIMACS .cnf și determină dacă problema este satisfiabilă.

Structura proiectului 
1. rezolutie.c # Implementare algoritm rezoluție
2. davis_putnam.c # Implementare algoritm Davis-Putnam (DP)
3. dpll_solver.c # Implementare algoritm DPLL
4. test50.cnf # Test cu 50 clauze
5. test200.cnf # Test cu 200 clauze
6. test400.cnf # Test cu 400 clauze
 README.md # Acest fișier

Compilare
•	Algoritmul de Rezoluție:
gcc rezolutie.c -o rezolutie.exe 
•	Algoritmul Davis-Putnam (DP):
gcc davis_putnam.c -o dp.exe 
•	Algoritmul DPLL:
gcc dpll_solver.c -o dpll.exe 
Rulare
După compilare, rulează executabilele astfel:
•	Rezoluție:
./rezolutie.exe 
•	Davis-Putnam (DP):
./dp.exe 
•	DPLL:
./dpll.exe 

Fișiere test
•	test50.cnf – 50 clauze
•	test200.cnf – 200 clauze
•	test400.cnf – 400 clauze

Algoritmi
Algoritmul de Rezoluție
•	Folosește rezoluția clasică pe perechi de clauze
•	Detectează clauza vidă (contradicție) pentru a decide NESAT
•	Dacă nu găsește contradicție, concluzionează SAT
Algoritmul Davis-Putnam (DP)
•	Elimină variabile prin rezoluție succesivă
•	Redu formulele la clauze mai simple
•	Determină satisfiabilitatea în mod recursiv

Algoritmul DPLL
•	Include propagare unitară pentru deducții rapide
•	Folosește backtracking și alegere heuristica a variabilelor
•	Determină satisfiabilitatea eficient
Output
Exemplu DPLL:
Problema contine 10 variabile si 50 clauze Propagare unitara: x3 = Adevarat ...Rezultat: SATISFIABILA 

Exemplu Rezoluție sau DP:
SATISFIABILsau NESATISFIABIL 



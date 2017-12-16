# git-refactoring-mip-2017-michalkostal
"Refaktorovanie projektu z PKS; najma optimalizovanie a prehladnost kodu." 

V ramci projektu som sa zameral na zoptimalizovanie a prehladnost kodu z Pocitacovych a Komunikacnych sieti. Refaktoroval som najma vnutorne struktury. 
Moj kod predtym pozostaval z dvoch hlavnych funkcii Klient a Server a kazdy z nich fungoval samostatne. Teraz som sa snazil nahradit najma vacsinu IF-ov 
a while-ov nahradit podfunkciami, ktorymi hlavne sprehladnime kod. Taktiez som kod este trosku sfunkcnil, pri jednom z commitov som opravil par chyb, 
ktore som este predtym nemal otestovane. V povodnej vetve Master som si vytvoril README subor, do ktoreho som vlozil, co budem robit a este povodny kod. 
Nasledne som si vytvoril Refaktoring vetvu, kde som postupne commitoval a pushoval priebezne zadania. Taktiež som do nej pridal gitignore súbor, do 
ktorého som vložil okrem už prítoných súborov všetky, ktoré som nepridal (teda tie, ktoré mi GIT STATUS vyhadzoval ako cervene.) Refaktorovany kod som 
mergol do vetvy master.

// Zistenie: Pri commitovaní do vetvy Refaktorovanie, sa mi vytvorila v Insights->Network ďaľšia vetva. Pri mergnutí s masterom sa táto cesta kompletne 
vymazala, respektíve sa možno tie commity tvária ako keby boli prevedené na vetve master. Použitý kód bol "git push origin Refaktorovanie:master"

// Pre krajší workflow chart, kde by bolo vidno, kedy som do ktorej vetvy commitoval/pushoval a kedy som ich mergoval som mal použiť príkaz "git rebase"
rozdiel od mergu je v tom, že rebase zachováva históriu projektu v pôvodnom stave. Mergeovania a pushovanie vetiev na seba má za následok, že všetko 
robené s pushovanou vetvou sa prejaví v tej, do ktorej pushujeme.
#include <stdio.h>
#include <time.h>

#include "kissat.h"

__attribute__((always_inline)) inline int literal(int row, int col, int val) {
  return 100 * row + 10 * col + val;
}

kissat* init_sudoku_solver() {
  kissat* solver = kissat_init();
  kissat_reserve(solver, 1000);

  // each cell must contain one of 9 digits
  for (int r = 1; r <= 9; ++r) {
    for (int c = 1; c <= 9; ++c) {
      kissat_add(solver, literal(r, c, 1));
      kissat_add(solver, literal(r, c, 2));
      kissat_add(solver, literal(r, c, 3));
      kissat_add(solver, literal(r, c, 4));
      kissat_add(solver, literal(r, c, 5));
      kissat_add(solver, literal(r, c, 6));
      kissat_add(solver, literal(r, c, 7));
      kissat_add(solver, literal(r, c, 8));
      kissat_add(solver, literal(r, c, 9));
      kissat_add(solver, 0);
    }
  }

  // each digit can appear only once in a row
  for (int r = 1; r <= 9; ++r) {
    for (int d = 1; d <= 9; ++d) {
      for (int c1 = 1; c1 <= 9; ++c1) {
        for (int c2 = c1 + 1; c2 <= 9; ++c2) {
          kissat_add(solver, -literal(r, c1, d));
          kissat_add(solver, -literal(r, c2, d));
          kissat_add(solver, 0);
        }
      }
    }
  }

  // each digit can appear only once in a col
  for (int c = 1; c <= 9; ++c) {
    for (int d = 1; d <= 9; ++d) {
      for (int r1 = 1; r1 <= 9; ++r1) {
        for (int r2 = r1 + 1; r2 <= 9; ++r2) {
          kissat_add(solver, -literal(r1, c, d));
          kissat_add(solver, -literal(r2, c, d));
          kissat_add(solver, 0);
        }
      }
    }
  }

  // each digit can appear only once in a block
  int rr[9], cc[9];
  for (int block = 0; block < 9; ++block) {
    int r0 = 3 * (block / 3) + 1;
    int c0 = 3 * (block % 3) + 1;
    for (int i = 0; i < 3; ++i) {
      rr[3 * i + 0] = r0 + i;
      rr[3 * i + 1] = r0 + i;
      rr[3 * i + 2] = r0 + i;
      cc[3 * i + 0] = c0 + 0;
      cc[3 * i + 1] = c0 + 1;
      cc[3 * i + 2] = c0 + 2;
    }
    for (int d = 1; d <= 9; ++d) {
      for (int i = 0; i < 9; ++i) {
        for (int j = i + 1; j < 9; ++j) {
          kissat_add(solver, -literal(rr[i], cc[i], d));
          kissat_add(solver, -literal(rr[j], cc[j], d));
          kissat_add(solver, 0);
        }
      }
    }
  }

  return solver;
}

void print_solution(kissat* solver) {
  const char* row_marker = "+---------+---------+---------+\n";
  printf("%s", row_marker);
  for (int r = 1; r <= 9; ++r) {
    putchar('|');
    for (int c = 1; c <= 9; ++c) {
      for (int d = 1; d <= 9; ++d) {
        if (kissat_value(solver, literal(r, c, d)) > 0) {
          printf(" %d ", d);
          if (c % 3 == 0) {
            putchar('|');
          }
          break;
        }
      }
    }
    printf("\n%s", r % 3 == 0 ? row_marker : "");
  }
}

void set_digit(kissat* solver, int row, int col, int digit) {
  kissat_add(solver, literal(row, col, digit));
  kissat_add(solver, 0);
}

int main(int argc, char* argv[]) {
  kissat* solver = init_sudoku_solver();
  kissat_set_option(solver, "quiet", 1);

  FILE* file = NULL;
  if (argc == 2) { file = fopen(argv[1], "r"); }
  char sudoku[81];
  fscanf(file ? file : stdin, "%81s", sudoku);
  if (file) { fclose(file); }
  for (int r = 1; r <= 9; ++r) {
    for (int c = 1; c <= 9; ++c) {
      int index = 9 * (r - 1) + (c -  1);
      if (sudoku[index] < '1' || sudoku[index] > '9') { continue; }
      set_digit(solver, r, c, sudoku[index] - '0');
    }
  }

  clock_t start = clock();
  int res = kissat_solve(solver);
  clock_t end = clock();

  if (res != 10) {
    printf("No solution found\n");
    return 1;
  }

  print_solution(solver);
  double time_taken = (double)(end - start) / (double)CLOCKS_PER_SEC;
  printf("Time taken: %fms\n", 1000.0 * time_taken);
  // kissat_print_statistics(solver);

  kissat_release(solver);
  return 0;
}

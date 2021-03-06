#include <cassert>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

#if defined(USE_MINISAT)
#include "minisat/core/Solver.h"
using namespace Minisat;
#endif

#if defined(USE_SLIME)
#include "Solver.h"
using namespace SLIME;
#endif

using namespace std;
using namespace std::chrono;

class Sudoku {
 public:
  Sudoku() {
    for (int i = 0; i < 9; ++i) {
      for (int j = 0; j < 9; ++j) {
        s[i][j] = 0;
      }
    }
  }

  Sudoku(const Sudoku& p) {
    for (int i = 0; i < 9; ++i) {
      for (int j = 0; j < 9; ++j) {
        s[i][j] = p(i, j);
      }
    }
  }

  int operator()(int row, int col) const {
    return s[row][col];
  }
  int& operator()(int row, int col) {
    return s[row][col];
  }

  bool valid() {
    // checks if the cells contain a valid entry
    for (int i = 0; i < 9; ++i) {
      for (int j = 0; j < 9; ++j) {
        if (s[i][j] < 0 or s[i][j] > 9) {
          return false;
        }
      }
    }

    for (int i = 0; i < 9; ++i) {
      if (not row_valid(i))
        return false;
      if (not col_valid(i))
        return false;
      if (not box_valid(i))
        return false;
    }
    return true;
  }

  bool solved() {
    for (int i = 0; i < 9; ++i) {
      for (int j = 0; j < 9; ++j) {
        if (s[i][j] == 0) {
          return false;
        }
      }
    }
    return valid();
  }

  void print() {
#if defined(USE_SLIME)
    printf("\n");
#endif
    for (int i = 0; i < 9; ++i) {
      if (i % 3 == 0)
        printf("+---------+---------+---------+\n");
      for (int j = 0; j < 9; ++j) {
        if (j % 3 == 0)
          printf("|");
        printf(" %d ", s[i][j]);
      }
      printf("|\n");
    }
    printf("+---------+---------+---------+\n");
  }

 private:
  int s[9][9]; ///< Sudoku data

  bool row_valid(int idx) {
    vector<int> count(10, 0);
    for (int i = 0; i < 9; ++i) {
      int d = s[idx][i];
      if (d != 0) { // check only for non-empty cells
        if (count[d] > 0) {
          return false;
        }
        count[d]++;
      }
    }
    return true;
  }

  bool col_valid(int idx) {
    vector<int> count(10, 0);
    for (int i = 0; i < 9; ++i) {
      int d = s[i][idx];
      if (d != 0) { // check only for non-empty cells
        if (count[d] > 0) {
          return false;
        }
        count[d]++;
      }
    }
    return true;
  }

  bool box_valid(int idx) {
    vector<int> count(10, 0);
    for (int i = 3 * (idx / 3); i < 3 * (idx / 3) + 3; ++i) {
      for (int j = 3 * (idx % 3); j < 3 * (idx % 3) + 3; ++j) {
        int d = s[i][j];
        if (d != 0) { // check only for non-empty cells
          if (count[d] > 0) {
            return false;
          }
          count[d]++;
        }
      }
    }
    return true;
  }
};

int main(int argc, char** argv) {
  char buf[81];
  FILE* file = NULL;
  if (argc == 2) { file = fopen(argv[1], "r"); }
  fscanf(file ? file : stdin, "%81s", buf);
  if (file) { fclose(file); }
  Sudoku sudoku;
  for (int k = 0; k < 81; ++k) {
    int val = (buf[k] < '1' || buf[k] > '9') ? 0 : (buf[k] - '0');
    sudoku(k / 9, k % 9) = val;
  }

  if (not sudoku.valid()) {
    cerr << "The provided sudoku is invalid" << endl;
    exit(-1);
  }

  Solver s;

  Var v[9][9][10]; // i, j, d
  Lit x[9][9][10];
  for (int k = 0; k < 81; ++k) {
    int i(k / 9), j(k % 9);
    for (int d = 1; d < 10; ++d) {
      v[i][j][d] = s.newVar();
      x[i][j][d] = mkLit(v[i][j][d]);
    }
  }

  // add clauses to the solver
  // each cell must contain only one of the 9 digits
  for (int k = 0; k < 81; ++k) {
    int i(k / 9), j(k % 9);
    vec<Lit> c;
    for (int d = 1; d <= 9; ++d) {
      c.push(x[i][j][d]);
    }
    s.addClause(c);
  }

  // only one digit can be true. others must be false
  for (int k = 0; k < 81; ++k) {
    int i(k / 9), j(k % 9);
    for (int d = 1; d <= 9; ++d) {
      for (int dp = d + 1; dp <= 9; ++dp) {
        s.addClause(~x[i][j][d], ~x[i][j][dp]);
      }
    }
  }

  // each digit can appear only once in a row
  for (int d = 1; d <= 9; ++d) {
    for (int i = 0; i < 9; ++i) {
      // only one of x[i][:][d] can be true
      for (int j = 0; j < 9; ++j) {
        for (int jp = j + 1; jp < 9; ++jp) {
          s.addClause(~x[i][j][d], ~x[i][jp][d]);
        }
      }
    }
  }

  // each digit can appear only once in a col
  for (int d = 1; d <= 9; ++d) {
    for (int j = 0; j < 9; ++j) {
      // only one of x[:][j][d] can be true
      for (int i = 0; i < 9; ++i) {
        for (int ip = i + 1; ip < 9; ++ip) {
          s.addClause(~x[i][j][d], ~x[ip][j][d]);
        }
      }
    }
  }

  // each digit can appear only once in a 3x3 block
  for (int d = 1; d <= 9; ++d) {
    for (int g = 0; g < 9; ++g) {
      int r(g / 3), c(g % 3);
      for (int i = 3 * r; i < 3 * r + 3; ++i) {
        for (int j = 3 * c; j < 3 * c + 3; ++j) {
          for (int ip = 3 * r; ip < 3 * r + 3; ++ip) {
            for (int jp = 3 * c; jp < 3 * c + 3; ++jp) {
              if (i != ip or j != jp) {
                s.addClause(~x[i][j][d], ~x[ip][jp][d]);
              }
            }
          }
        }
      }
    }
  }

  // add clauses for numbers which are already filled
  for (int k = 0; k < 81; ++k) {
    int i(k / 9), j(k % 9), d(sudoku(i, j));
    if (d != 0) {
      s.addClause(x[i][j][d]);
    }
  }

  auto t1 = high_resolution_clock::now();
  s.solve();
  auto t2 = high_resolution_clock::now();
  auto duration = duration_cast<microseconds>(t2 - t1).count();

  if (s.okay()) {
    // update the sudoku
    for (int k = 0; k < 81; ++k) {
      int i(k / 9), j(k % 9);
      if (sudoku(i, j) == 0) {
        for (int d = 1; d <= 9; ++d)
          if (s.model[v[i][j][d]] == l_True)
            sudoku(i, j) = d;
      }
    }
    sudoku.print();
  }

  cout << "time: " << duration * 1e-3 << " ms\n";
  return 0;
}

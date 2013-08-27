/*
 * name : Kuo Liu
 * Andrew ID : kuol
 *
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"
#include "contracts.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. The REQUIRES and ENSURES from 15-122 are included
 *     for your convenience. They can be removed if you like.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int start_row, start_column, end_row, end_column;
    int i,j;
    int a0,a1,a2,a3,a4,a5,a6,a7;
    int temp;
    int row;
    int flag;
    int step;
    REQUIRES(M > 0);
    REQUIRES(N > 0);
    if(M == N && M == 32){
        if(M == 32)
            step = 8;
        for(start_row = 0; start_row < N; start_row += step){
            end_row = start_row + step;
            for(start_column = 0; start_column < M; start_column += step){
                end_column = start_column + step;
                for(i = start_row; i < end_row; ++ i){
                    for(j = start_column; j < end_column; ++ j){
                        if(i != j)
                            B[j][i] = A[i][j];
                        else{
                            flag = 1;
                            temp = A[i][i];
                            row = i;
                        }
                    }
                    if(flag){
                        flag = 0;
                        B[row][row] = temp;
                    }
                }
            }
        }
    }else if(M == N && M ==64){
        for (start_column = 0; start_column < M; start_column += 8)
            for (start_row = 0; start_row < N; start_row += 8){
                for(i = start_row; i < start_row + 8; i += 2){
                    a0 = A[i][start_column];
                    a1 = A[i][start_column + 1];
                    a2 = A[i][start_column + 2];
                    a3 = A[i][start_column + 3];
                    a4 = A[i + 1][start_column];
                    a5 = A[i + 1][start_column + 1];
                    a6 = A[i + 1][start_column + 2];
                    a7 = A[i + 1][start_column + 3];
                    B[start_column][i] = a0;
                    B[start_column + 1][i] = a1;
                    B[start_column + 2][i] = a2;
                    B[start_column + 3][i] = a3;
                    B[start_column][i + 1] = a4;
                    B[start_column + 1][i + 1] = a5;
                    B[start_column + 2][i + 1] = a6;
                    B[start_column + 3][i + 1] = a7;
                }
                for(i = start_row + 7; i >= start_row ; i -= 2){
                    a0 = A[i][start_column + 4];
                    a1 = A[i][start_column + 5];
                    a2 = A[i][start_column + 6];
                    a3 = A[i][start_column + 7];
                    a4 = A[i - 1][start_column + 4];
                    a5 = A[i - 1][start_column + 5];
                    a6 = A[i - 1][start_column + 6];
                    a7 = A[i - 1][start_column + 7];
                    B[start_column + 4][i] = a0;
                    B[start_column + 5][i] = a1;
                    B[start_column + 6][i] = a2;
                    B[start_column + 7][i] = a3;
                    B[start_column + 4][i - 1] = a4;
                    B[start_column + 5][i - 1] = a5;
                    B[start_column + 6][i - 1] = a6;
                    B[start_column + 7][i - 1] = a7;
                }
            }
    }else if(M == 61 && N == 67){
        step = 21;
        for(start_column = 0; start_column < M; start_column += step){
            end_column = start_column + step;
            for(start_row = 0; start_row < N; start_row += step){
                end_row = start_row + step;
                for(i = start_row; i < end_row; ++ i){
                    for(j = start_column; j < end_column; ++ j){
                        if(i < 67 && j < 61)
                            B[j][i] = A[i][j];
                    }
                }
            }
        }
    }else{
        for(i = 0; i < M; ++ i)
            for(j = 0; j < M; ++ j)
                B[j][i] = A[i][j];
    }

    ENSURES(is_transpose(M, N, A, B));
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    REQUIRES(M > 0);
    REQUIRES(N > 0);

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

    ENSURES(is_transpose(M, N, A, B));
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}


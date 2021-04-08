#include <bits/stdc++.h>
#include <thread>

#define MAX 30000 // Número de 0 a 29999
#define BLOCKS_NUMBER 6 // Número de blocos

using namespace std;

int **_matrix; // Matriz global
int _primeCount; // Contador de números primos
pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex para controlar concorrência de thread ao atualizar a variável _primeCount

/*
 Estrutura com as dimensões da matriz

 Members:
  int lines - Número de linhas
  int cols - Número de colunas
*/
typedef struct _dimensions{
    int lines;
    int cols;
}Dimensions;

/*
 Estrutura com os argumentos a serem passados para a função getMatrixPrimeCount

 Members:
  int** matrix - Matriz a ser calculada
  Dimensions* dims - Dimensão da matriz a ser calculada
  int pos - Posição de submatriz a ser calculada
*/
typedef struct _args{
    int **matrix;
    int pos;
    Dimensions *dims;
}Args;

/*
 Função que aloca e preenche uma Dimensions e retorna um ponteiro para ela

 Arguments:
  int lines - Número de linhas
  int cols - Número de colunas
*/
Dimensions *make_dimensions(int lines, int cols){
    Dimensions *dims = (Dimensions*)malloc(sizeof(Dimensions));
    if(dims!=NULL){
        dims->lines = lines;
        dims->cols = cols;
    }
    return dims;
}

/*
 Função que aloca e preenche uma Args e retorna um ponteiro para esta

 Arguments:
  int** matrix - Matriz a ser passada
  int pos - Posição inicial do bloco
  Dimensions *dims - Dimensões da matriz
*/
Args *make_args(int **matrix, int pos, Dimensions *dims){
    Args *args = (Args*)malloc(sizeof(Args));
    args->matrix = matrix;
    args->pos = pos;
    args->dims = dims;
    return args;
}

/*
 Função que mostra o conteúdo de uma struct do tipo Dimensions na tela

 Arguments:
  Dimensions* dims - Struct que se deseja imprimir
*/
void print(Dimensions *dims){
    printf("(%d,%d)\n", dims->lines, dims->cols);
}

/*
 Função que aloca uma matriz dada as suas dimensões

 Arguments:
  Dimensios* dims - Dimensões da matriz a ser gerada
*/
int **generateMatrix(Dimensions *dims){
    int **matrix = (int**)malloc(sizeof(int*)*dims->lines);
    for(int i=0; i<dims->lines; i++)
        matrix[i] = (int*)malloc(sizeof(int)*dims->cols);
    return matrix;
}

/*
 Função que preenche uma matrix com números aleatórios

 Arguments:
  int** matrix - Matriz a ser preenchida
  Dimensions* dims - Dimensão da matriz a ser preenchida
*/
void fillMatrixWithRandomNumbers(int **matrix, Dimensions *dims){
    for(int i=0; i<dims->lines; i++)
        for(int j=0; j<dims->cols; j++)
            matrix[i][j] = rand() % MAX;
}

/*
 Função que imprime uma matriz na tela

 Arguments:
  int** matrix - Matriz a ser impressa
  Dimensions* dims - Dimensões da matriz a ser impressa
*/
void printMatrix(int **matrix, Dimensions *dims){
    for(int i=0; i<dims->lines; i++){
        for(int j=0; j<dims->cols; j++)
            printf("%d\t", matrix[i][j]);
        cout << endl;
    }
}

/*
 Função que retorna se um número é ou não primo

 Arguments:
  int number - Número a ser verificado.
*/
bool isPrime(int number){
    if(number==2) return true;
    if (number%2==0) return false;

    int limit = (int)sqrt(number);

    for(int i=3; i<=limit; i+=2)
        if(number%i==0)  return false;

    return true;
}

/*
 Função que muda a variável global _primeCount de acordo com o número de números primos na matriz

 Arguments:
   void *args - Argumentos para a função, deve ser um ponteiro para uma estrutura do tipo Args.
*/
void *getMatrixPrimeCount(void *args){
    Args *type_args = (Args*)args;
    int **matrix = type_args->matrix;
    Dimensions *dims = type_args->dims;
    int initial_block_pos = type_args->pos;
    int block_size = (dims->lines * dims->cols) / BLOCKS_NUMBER;

    int count = 0;
    while(count < block_size && count + initial_block_pos < dims->lines * dims->cols){
        int line = (initial_block_pos + count) / dims->cols;
        int col = (initial_block_pos + count) % dims->cols;
        pthread_mutex_lock(&_mutex);
        _primeCount += (isPrime(matrix[line][col]) ? 1 : 0);
        pthread_mutex_unlock(&_mutex);
        count++;
    }
    free(args);
    return NULL;
}

/*
 Função que retorna o número de primos em uma tabela, sem utilizar thread(Apenas um sanity check)

 Arguments:
  int** matrix - Matriz a ser verificada
  Dimensions *dims - Dimensões da matriz
*/
int getMatrixPrimeCountLinear(int **matrix, Dimensions *dims){
    int count=0;
    for(int i=0; i<dims->lines; i++)
        for(int j=0; j<dims->cols; j++)
            count += (isPrime(matrix[i][j]) ? 1 : 0);

    return count;
}


int main(int argc, char **argv){
    srand(42);

    Dimensions *matrix_dims = {make_dimensions(3600, 3600)};
    pthread_t threads[BLOCKS_NUMBER];

    _matrix = generateMatrix(matrix_dims);
    fillMatrixWithRandomNumbers(_matrix, matrix_dims); 

    for(int i=0; i<BLOCKS_NUMBER; i++){
        int block_size = (matrix_dims->lines * matrix_dims->cols) / BLOCKS_NUMBER;
        Args *args = make_args(_matrix, i*block_size, matrix_dims);
        pthread_create(&threads[i], NULL, getMatrixPrimeCount, (void*)args);
    }

    for(int i=0; i<BLOCKS_NUMBER; i++)
        pthread_join(threads[i], NULL);

    cout << "Numero de primos(thread): " << _primeCount << endl;

    for(int i=0; i<matrix_dims->lines; i++)
	    free(_matrix[i]);
    free(_matrix);
    free(matrix_dims);

    return 0;
}
